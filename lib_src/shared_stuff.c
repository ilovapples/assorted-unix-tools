#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "shared_stuff.h"
#include "types.h"

void error(s32 err_code, const char *fmt, ...)
{
	va_list this_va_list;
	va_start(this_va_list, fmt);
	vfprintf(stderr, fmt, this_va_list);
	if (err_code != -5280)
		exit(err_code);
}


#define INIT_BUF_SIZE 256
strbuf read_string_from_stream(FILE *stream)
{
	size_t buf_size = INIT_BUF_SIZE;
	strbuf ret = { NULL, 0 };
	ret.s = calloc(buf_size, sizeof(char));
	if (ret.s == NULL)
		return (strbuf) { NULL, 0 };

	size_t n_read;
	while ((n_read = fread(ret.s + ret.l, 1, buf_size - ret.l - 1, stream)) > 0)
	{
		ret.l += n_read;
		if (ret.l >= buf_size - 1)
		{
			char *temp_buf = realloc(ret.s, buf_size *= 2);
			if (temp_buf == NULL)
			{
				free(ret.s);
				return (strbuf) { NULL, 0 };
			}
			ret.s = temp_buf;
		}
	}

	ret.s[ret.l++] = '\0';

	return ret;
}
