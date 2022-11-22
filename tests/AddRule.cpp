
#include "IpRoute2.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

int main(int argc, char **argv)
{
	IpRoute2 iproute2;

	iproute2.AddRule(argv[1], 100);
	return EXIT_SUCCESS;
}
