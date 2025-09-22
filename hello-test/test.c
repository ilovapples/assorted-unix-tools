#include <stdint.h>
#include <stdio.h>

int32_t main(int32_t argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "usage: %s <name>\n", argv[0]);
		return 1;
	}

	printf("Hello, %s!\n", argv[1]);

	return 0;
}
