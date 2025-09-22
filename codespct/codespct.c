#include <inttypes.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "shared_stuff.h"
#include "types.h"

#define BUF_LEN 512

int32_t main(int32_t argc, char **argv)
{
	/* same interface but different file (path vs stdin) hmm*/
	FILE *fp = stdin;
	if (argc > 1)
	{
		fp = fopen(argv[1], "rb");
		if (!fp)
		{
			fprintf(stderr, "%s: failed to open '%s'\n", argv[0], argv[1]);
			return 1;
		}
	}

	int32_t c;
	while ((c = fgetc(fp)) != EOF)
	{
		if (iscntrl(c) || c == '<' || c == '>')
			printf("<%02x>", c);
		else
			fputc(c, stdout);
	}
	
	return 0;
}
