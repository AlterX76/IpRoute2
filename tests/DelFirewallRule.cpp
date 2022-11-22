#include "IpTables.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

int main(int argc, char **argv)
{
	NFTables iptables;

	printf("result: %d\n", iptables.DelRedirect("nat", "REDSOCKS", argv[1]));
	return EXIT_SUCCESS;
}
