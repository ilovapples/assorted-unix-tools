#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>

#include "types.h"

s32 main(s32 argc, char **argv)
{
#if 0
	{
		printf("usage: \n"
			 "   %s text_to_strip\n"
			 "   %s - \n\n"
			 
			 "(with '-' it reads from stdin)\n"
				, argv[0], argv[0]);
		return 1;
	}
#endif
	bool found_non_whitespace = false;

	if (argc < 2 || argv[1][0] == '-')
	{
		int32_t c;
		while ((c = fgetc(stdin)) != EOF)
		{
			if (c == '\x1b')
			{
				while ((c = fgetc(stdin)) != EOF && !isalpha(c))
					;
				c = fgetc(stdin);
			}
			if (!isspace(c) || found_non_whitespace)
			{
				found_non_whitespace = true;
				putc(c, stdout);
			}
		}

		return 0;
	}

	char *c = argv[1];
	while (*c)
	{
		if (*c == '\x1b')
			while (!isalpha(*c++))
				;
		if (!(isspace(*c++) || iscntrl(*c)))
			putc(*c, stdout);
	}

	return 0;
}
