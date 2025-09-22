#include <string.h>
#include <stdint.h>
#include <stdio.h>

int32_t main(int32_t argc, char **argv)
{
	if (argc > 1)
	{
		printf("%zu", strlen(argv[1]));
		return 0;
	}

	register size_t n = 0;
	while (fgetc(stdin) != EOF)
		++n;

	printf("%zu", n);

	return 0;
}
