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

	unsigned int id_table = 100;
	if (id_table < 256)
	    req.interface.rtm_table = id_table;
	printf("send: %d\n", send(fd, &req, req.nh.nlmsg_len, 0));	
	close(fd);
	return EXIT_SUCCESS;
}
