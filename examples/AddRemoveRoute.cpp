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
    req.nh.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
    req.nh.nlmsg_type = RTM_NEWROUTE;
    //
    // initialises the request:
    req.interface.rtm_family = AF_INET;
    req.interface.rtm_protocol = RTPROT_BOOT;
    req.interface.rtm_scope = RT_SCOPE_LINK; // used when it is not a default gw (instead use RT_SCOPE_UNIVERSE)
    req.interface.rtm_type = RTN_UNICAST;
    req.interface.rtm_table = RT_TABLE_MAIN; // main table used only if not set RTA_TABLE
    //
    // initilialises attributes:
    unsigned int id_table = 100;
    //addattr_l(&req, RTA_TABLE, &id_table, sizeof(id_table));
    //
    // routing:    
    char dst_ip[] = { "172.24.1.0/24" };
    char gw_ip[] = { "192.168.3.50" };
    int if_index = if_nametoindex("wlan0");
    inet_prefix dst, gw;
    //
    // add destination:
    get_prefix(&dst, dst_ip, req.interface.rtm_family);
    req.interface.rtm_dst_len = dst.bitlen; 
    addattr_l(&req, RTA_DST, &dst.data, dst.bytelen);
    //
    // add gateway:
    req.interface.rtm_scope = RT_SCOPE_UNIVERSE;
    get_prefix(&gw, gw_ip, req.interface.rtm_family);
    req.interface.rtm_src_len = gw.bitlen;    
    addattr_l(&req, RTA_GATEWAY, &gw.data, gw.bytelen);    
    //
    // add output interface:
    addattr_l(&req, RTA_OIF, &if_index, sizeof(if_index));

    printf("send: %d\n", send(fd, &req, req.nh.nlmsg_len, 0));	
    close(fd);
    return EXIT_SUCCESS;
}
