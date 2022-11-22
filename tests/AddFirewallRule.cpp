
#include "IpTables.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

int main(int argc, char **argv)
{
	NFTables iptables;

	iptables.AddRedirect("nat", "REDSOCKS", argv[1], atoi(argv[2]));
	return EXIT_SUCCESS;
}
