#include <string.h>
#include <stdint.h>
#include <stdio.h>

int32_t main(int32_t argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: arg1len arg");
		return 1;
	}

	printf("%d", strlen(argv[1]));
	return 0;
}
