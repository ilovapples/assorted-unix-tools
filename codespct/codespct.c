#include <inttypes.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#define BUF_LEN 512

int32_t main(int32_t argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: inspect filename\n");
		return 1;
	}

	FILE *fptr = fopen(argv[1], "rb");
	if (!fptr) {
		fprintf(stderr, "couldn't open test.txt for some reason\n");
		return 2;
	}

	char buf[BUF_LEN+1] = {0};
	fread(buf, sizeof(char), BUF_LEN, fptr);
	
	printf("buf contents: \n");

	size_t buf_strlen = strlen(buf);
	char *cp = buf;
	while (buf_strlen--) {
		if (iscntrl(*cp))
			printf("{'\\x%02x'}", *cp);
		else
			printf("%c", *cp);
		cp++;
	}
	
	return 0;
}
