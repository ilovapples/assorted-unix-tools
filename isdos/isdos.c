#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINE_LEN 32767

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

	char first_line[MAX_LINE_LEN+1];
	fgets(first_line, MAX_LINE_LEN, fp);

	size_t line_len = strlen(first_line);

	uint8_t exit_code;
	if (first_line[line_len-2] == '\r') {
		printf("dos");
		exit_code = 0;
	} else if (first_line[line_len-1] != '\n') {
		fprintf(stderr, "%s: first line is longer than %d single-width characters\n",
				argv[0], MAX_LINE_LEN);
		exit_code = 4;
	} else {
		printf("unix");
		exit_code = 1;
	}

	if (read_stdin) fclose(fp);
	return exit_code;
}
