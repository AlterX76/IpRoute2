
#include "IpRoute2.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

int main(int argc, char **argv)
{
	IpRoute2 iproute2;

printf("num params: %d\n", argc);
	if (argc == 2)
		iproute2.RemoveRoute(argv[1], "0.0.0.0/1");
	else if (argc == 3)
		iproute2.AddRoute(argv[2], argv[1]); // ip route add ip/mask dev interf
	else if (argc == 4) 
		iproute2.AddRoute(argv[2], argv[1], atoi(argv[3])); // ip route add ip/mask dev interf table 100
	else if (argc == 5)
		iproute2.AddRoute(argv[2], nullptr, atoi(argv[3]), argv[1]); // ip route add default via ip/mask dev interf table 100
	else {
		iproute2.DeleteRuleTable(100);
	}
	return EXIT_SUCCESS;
}
