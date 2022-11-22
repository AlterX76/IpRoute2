
#include "IpRoute2.h"

#include <cstdlib>
#include <cstdio>

int main(int argc, char **argv)
{
	printf("Table id: %d\n", IpRoute2::GetIdFromTableName(argv[1]));
	return EXIT_SUCCESS;
}
