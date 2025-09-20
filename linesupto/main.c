#include <string.h>
#include <inttypes.h>
#include <stdio.h>

#define LINE_BUF_MAX 2048

int32_t main(int32_t argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr,
				"usage: %s <char>\n\n"
				
				"prints each line of its input up to the first instance of <char>\n",
				argv[0]);
		return 1;
	}

	char stop_char = argv[1][0];

	char line_buf[LINE_BUF_MAX+1];
	while (fgets(line_buf, LINE_BUF_MAX, stdin))
		printf("%.*s\n", (int)(strchr(line_buf, stop_char) - line_buf), line_buf);
	
	return 0;
}
