#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINE_LEN 32767
#define STR_(_0) #_0
#define STR(_0) STR_(_0)

int32_t main(int32_t argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		return 2;
	}

	const bool read_stdin = strcmp(argv[1], "-") == 0;
	FILE *fp = (read_stdin) ? stdin : fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "%s: could not open file '%s'\n", argv[0], argv[1]);
		return 3;
	}

	uint8_t exit_code;

	char first_line[MAX_LINE_LEN+1];
	if (!fgets(first_line, MAX_LINE_LEN, fp)) {
		first_line[0] = '\0';
	}

	char *const line_end = strchr(first_line, '\0');
	assert(line_end != NULL);

	if (line_end > first_line+1 && line_end[-2] == '\r') {
		printf("dos");
		exit_code = 0;
	} else if (line_end == first_line || line_end[-1] != '\n') {
		fprintf(stderr, "%s: input does not contain a newline within the first " STR(MAX_LINE_LEN) " chars\n",
				argv[0]);
		exit_code = 4;
	} else {
		printf("unix");
		exit_code = 1;
	}

	if (read_stdin) fclose(fp);
	return exit_code;
}
