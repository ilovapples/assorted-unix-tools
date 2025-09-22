#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define MAX_LINE_LEN 32767

int32_t main(int32_t argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		return 2;
	}
	
	FILE *fp = fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "%s: could not open file '%s'\n", argv[0], argv[1]);
		return 3;
	}

	char first_line[MAX_LINE_LEN+1];
	fgets(first_line, MAX_LINE_LEN, fp);

	size_t line_len = strlen(first_line);

	if (first_line[line_len-2] == '\r') {
		printf("dos");
		return 0;
	} else if (first_line[line_len-1] != '\n') {
		fprintf(stderr, "%s: first line is longer than %d single-width characters\n",
				argv[0], MAX_LINE_LEN);
		return 4;
	} else {
		printf("unix");
		return 1;
	}
}
