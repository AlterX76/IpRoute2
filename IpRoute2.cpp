#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>

#include "utils.h"

#include "IpRoute2.h"
//
// internal message for the kernel iproute2:
struct IpRoute2::Request {
	struct nlmsghdr nh;
	struct rtmsg 	rt;
	char   			attributes[1024];
};			

bool IpRoute2::AddRoute(const char *interface, const char *destination, unsigned int table, const char *gateway) 
{
	return this->manage_route(RTM_NEWROUTE, interface, destination, table, gateway);
}			

bool IpRoute2::RemoveRoute(const char *interface, const char *destination, unsigned int table, const char *gateway) 
{
	return this->manage_route(RTM_DELROUTE, interface, destination, table, gateway);
}

bool IpRoute2::AddRule(const char *from, unsigned int table)
{
	return this->manage_rule(RTM_NEWRULE, from, table);
}

bool IpRoute2::RemoveRule(const char *from, unsigned int table)
{
	return this->manage_rule(RTM_DELRULE, from, table);
}

int IpRoute2::GetIdFromTableName(const char *table)
{
	FILE	*rt_table = fopen("/etc/iproute2/rt_tables", "r");	
	int 	id_table = -1;
	
	if (rt_table == NULL) {
		return id_table;
	}
	//
	// loops to find table in the file to grab the id:
	char buffer[1024] = { 0 };
	
	while (fgets(buffer, 1023, rt_table)) {
		if (strstr(buffer, table)) { // found it!			
			char *tmp = buffer;
			
			while (*tmp != ' ')
				tmp++;
			*tmp = 0;			
			id_table = atoi(buffer);
			break;
		}
	}
	fclose(rt_table);
	return id_table;
}

bool IpRoute2::DeleteRuleTable(unsigned int table)
{
	auto	fd = this->open_socket();
	Request	req;

	if (fd < 0)
	    return false;
	// initialises the header:
	memset(&req, 0, sizeof(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(req.rt));
    req.nh.nlmsg_flags = NLM_F_REQUEST;
	req.nh.nlmsg_type = RTM_DELRULE;
	//
	// initialises the request:
	req.rt.rtm_family = AF_INET;
	req.rt.rtm_protocol = RTPROT_BOOT;
	req.rt.rtm_scope = RT_SCOPE_UNIVERSE;

	auto id_table = table;	
	if (id_table < 256)
	    req.rt.rtm_table = id_table;
	//
	// sends the packet to the kernel:    
	auto read = this->send_request(&req, fd);
	close_socket(fd);
	return read > 0;
}

bool IpRoute2::manage_rule(int action, const char *from, unsigned int table)
{	
	auto 	fd = this->open_socket();
	Request	req;

	if(fd < 0)
	    return false;
	// initialises the header:
	memset(&req, 0, sizeof(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(req.rt));
    req.nh.nlmsg_flags = NLM_F_REQUEST;
	req.nh.nlmsg_type = action;
	if (action == RTM_NEWRULE) {
		req.nh.nlmsg_flags |= NLM_F_CREATE|NLM_F_EXCL;
		req.rt.rtm_type = RTN_UNICAST;
	}	
	//
	// initialises the request:
	req.rt.rtm_family = AF_INET;
	req.rt.rtm_protocol = RTPROT_BOOT;
	req.rt.rtm_scope = RT_SCOPE_UNIVERSE;
	//
	// settings for the rule:
	auto id_table = table;
	
	addattr_l(&req, RTA_TABLE, &id_table, sizeof(id_table));		
	this->add_source(&req, from);
	auto read = this->send_request(&req, fd);
	this->close_socket(fd);
	return EXIT_SUCCESS;
}

bool IpRoute2::manage_route(int action, const char *interface, const char *destination, unsigned int table, const char *gateway) 
{
	auto 	fd = IpRoute2::open_socket();
	Request	req;
		
	if (fd < 0)
		return false;
	// initialises the header:
	memset(&req, 0, sizeof(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(req.rt));
	req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
	req.nh.nlmsg_type = action;
	//
	// initialises the request:
	req.rt.rtm_family = AF_INET;
	req.rt.rtm_protocol = RTPROT_BOOT;
	req.rt.rtm_scope = RT_SCOPE_LINK; // used when it is not a default gw (instead use RT_SCOPE_UNIVERSE)
	req.rt.rtm_type = RTN_UNICAST;
	req.rt.rtm_table = RT_TABLE_MAIN; // main table if not set RTA_TABLE attribute
	//
	// custom table or main one:
	if (table > 0) {
		auto id_table = table;
		addattr_l(&req, RTA_TABLE, &id_table, sizeof(id_table));
	}
	if (destination != nullptr) {
		this->add_destination(&req, destination);
	}
	if (gateway) {
		req.rt.rtm_scope = RT_SCOPE_UNIVERSE;
		this->add_gateway(&req, gateway);
	}
	if (interface) {
		this->add_oif(&req, interface);
	}
	//
	// sends the packet to the kernel:
	auto read = this->send_request(&req, fd);
	close_socket(fd);
	return read > 0;
}			
		
int IpRoute2::addattr_l(struct Request *request, int type, void *data, int alen)
{
	if (request == nullptr || data == nullptr)
		return -1;	
	/* alen is the length of the data. Add sizeof(struct rtattr) to it to accomodate
	 *     type, length, value format for rtattr */
	int len = RTA_LENGTH(alen); // (RTA_ALIGN(sizeof(struct rtattr)) + (len))
	int maxlen = sizeof(*request);
	struct nlmsghdr *n = &(request->nh);
	struct rtattr *rta;

	/* size of request should not be violated*/
	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
		return -1;
	/* go to end of buffer in request data structure*/
	rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
	/* specify attribute using TLV format*/
	rta->rta_type = type;
	rta->rta_len = len;
	/* increase the nlmsg_len to accomodate the added new attribute*/
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;
	memcpy(RTA_DATA(rta), data, alen);
	return 0;
}

int IpRoute2::open_socket()
{	
	return socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
}

void IpRoute2::close_socket(int descriptor)
{
	close(descriptor);
}

int IpRoute2::send_request(const Request *request, int descriptor)
{
	if (request == nullptr)
		return -1;	
	return send(descriptor, request, request->nh.nlmsg_len, 0);
}

void IpRoute2::add_oif(Request *request, const char *interface)
{
	if (request == nullptr || interface == nullptr)
		return;
		
	auto if_index = if_nametoindex(interface);	
	
	addattr_l(request, RTA_OIF, &if_index, sizeof(if_index));	
}

void IpRoute2::add_gateway(Request *request, const char *gateway)
{
	inet_prefix gw;
	char 		gw_ip[512];
	
	if (request == nullptr || gateway == nullptr)
		return;	
	strncpy(gw_ip, gateway, 511);
	get_prefix(&gw, gw_ip, request->rt.rtm_family);
	request->rt.rtm_src_len = gw.bitlen;    
	addattr_l(request, RTA_GATEWAY, &gw.data, gw.bytelen);    	
}

void IpRoute2::add_destination(Request *request, const char *destination)
{
	inet_prefix dst;
	char 		dst_ip[512];
	
	if (request == nullptr || destination == nullptr)
		return;	
	strncpy(dst_ip, destination, 511);		
	get_prefix(&dst, dst_ip, request->rt.rtm_family);
	request->rt.rtm_dst_len = dst.bitlen; 
	addattr_l(request, RTA_DST, &dst.data, dst.bytelen);	
}

void IpRoute2::add_source(Request *request, const char *source)
{
	inet_prefix src;
	char 		src_ip[512];
	
	if (request == nullptr || source == nullptr)
		return;
	strncpy(src_ip, source, 511);		
	get_prefix(&src, src_ip, request->rt.rtm_family);
	request->rt.rtm_src_len = src.bitlen; 
	addattr_l(request, RTA_SRC, &src.data, src.bytelen);	
}
