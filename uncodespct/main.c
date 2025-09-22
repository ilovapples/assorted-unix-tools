#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "shared_stuff.h"
#include "types.h"

int32_t main(int32_t argc, char **argv)
{
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
	char spec_char_buf[3];
	char *spec_char_top = spec_char_buf;
	while ((c = fgetc(fp)) != EOF)
	{
		if (c == '<')
		{
			while ((c = fgetc(fp)) != '>' && c != EOF)
				*spec_char_top++ = (char)c;
			spec_char_top = spec_char_buf;
			fputc(strtol(spec_char_buf, NULL, 16), stdout);
		} else
			fputc(c, stdout);
	}
	return 0;
}
