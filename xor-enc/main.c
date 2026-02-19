#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *PROGRAM_NAME = NULL;
bool code_was_set = false;
uint8_t code = 0;

void print_usage(void)
{
	fprintf(stderr,
			"usage: %s [options] <code>\n\n"
			
			"--help, -h                            Display this help message\n"
			"--char=CODE                           Encode/decode with CODE considered as ASCII (default is 0x0c)\n"
			"meaning of <code>: if present, is a one byte value that will be used for encoding/decoding\n"
			, PROGRAM_NAME);
	exit(1);
}

void arg_parse(int32_t argc, char **argv)
{
	PROGRAM_NAME = argv[0];
	for (int32_t arg_n = 1; arg_n < argc; ++arg_n)
	{
		if (strncmp(argv[arg_n], "--", 2) == 0)
		{
			if (strcmp(argv[arg_n]+2, "help") == 0)
				print_usage();
			else if (strncmp(argv[arg_n]+2, "char=", 5) == 0)
			{
				code = (uint8_t)argv[arg_n][2+5];
				code_was_set = true;
			} else
			{
				fprintf(stderr, "%s: argument error: unknown argument '%s'\n",
					PROGRAM_NAME, argv[arg_n]);
				exit(2);
			}
		} else if (argv[arg_n][0] == '-')
		{
			char *c = argv[arg_n]+1;
			while (*c)
			{
				switch (*c) {
				case 'h':
					print_usage();
					break;
				default:
					fprintf(stderr, "%s: argument error: unknown argument '-%c'\n",
						PROGRAM_NAME, *c);
					exit(2);
					break;
				}
				++c;
			}
		} else if (!code_was_set)
		{
			code = (uint8_t)strtoul(argv[arg_n], NULL, 0);
			code_was_set = true;
		} else
		{
			fprintf(stderr, "%s: argument error: illegal argument '%s'\n", PROGRAM_NAME, argv[arg_n]);
			exit(2);
		}
	}
	
	if (!code_was_set)
	{
		code = 0x0c;
		code_was_set = true;
	}
}

int32_t main(int32_t argc, char **argv)
{
	arg_parse(argc, argv);

	int32_t c;
	while ((c = fgetc(stdin)) != EOF)
		fputc(c ^ code, stdout);

	
	return 0;
}
