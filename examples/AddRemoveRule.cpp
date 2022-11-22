#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "utils.h"
 
struct Request {
	struct nlmsghdr  nh;
        struct rtmsg interface;
	char   attributes[512];
} req;

int addattr_l(struct Request *request, int type, void *data, int alen)
{
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

int main()
{
	int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	if(fd < 0) {
	    printf("socket creation failed\n");
	    return -1;
	}
	//
	// initialises the header:
	memset(&req, 0, sizeof(req));
	req.nh.nlmsg_len = NLMSG_LENGTH(sizeof(req.interface));
     	req.nh.nlmsg_flags = NLM_F_REQUEST;
	req.nh.nlmsg_type = RTM_DELRULE;
	if (req.nh.nlmsg_type == RTM_NEWRULE) {
		req.nh.nlmsg_flags |= NLM_F_CREATE|NLM_F_EXCL;
		req.interface.rtm_type = RTN_UNICAST;
	}	
	//
	// initialises the request:
	req.interface.rtm_family = AF_INET;
	req.interface.rtm_protocol = RTPROT_BOOT;
	req.interface.rtm_scope = RT_SCOPE_UNIVERSE;
	//
	// initilialises attributes:
	unsigned int id_table = 100;
	char ip[] = { "192.168.3.122" };
	inet_prefix dst;
	
	addattr_l(&req, RTA_TABLE, &id_table, sizeof(id_table));
	get_prefix(&dst, ip, req.interface.rtm_family);
	req.interface.rtm_src_len = dst.bitlen;
	addattr_l(&req, RTA_SRC, &dst.data, dst.bytelen);	
	printf("send: %d\n", send(fd, &req, req.nh.nlmsg_len, 0));	
	close(fd);
	return EXIT_SUCCESS;
}
