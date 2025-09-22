#include <inttypes.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#define BUF_LEN 512

int32_t main(int32_t argc, char **argv)
{
	if (argc > 1)
	{
		FILE *fptr = fopen(argv[1], "rb");
		if (!fptr) {
			fprintf(stderr, "failed to open '%s'\n", argv[1]);
			return 2;
		}

		char buf[BUF_LEN+1] = {0};
		fread(buf, sizeof(char), BUF_LEN, fptr);
		
		size_t buf_strlen = strlen(buf);
		char *cp = buf;
		while (buf_strlen--) {
			if (iscntrl(*cp))
				printf("<%02x>", *cp);
			else
				printf("%c", *cp);
			cp++;
		}
	} else
	{
		char c;
		while ((c = fgetc(stdin)) != EOF)
		{
			if (iscntrl(c))
				printf("<%02x>", c);
			else
				printf("%c", c);
		}
	}
	
	return 0;
}
