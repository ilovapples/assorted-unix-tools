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
memblck read_string_from_stream(FILE *stream)
{
	size_t buf_size = INIT_BUF_SIZE;
	memblck ret = { NULL, 0 };
	ret.m = malloc(buf_size);
	if (ret.m == NULL)
		return (memblck) { NULL, 0 };

	size_t n_read;
	while ((n_read = fread(ret.m + ret.l, 1, buf_size - ret.l - 1, stream)) > 0)
	{
		ret.l += n_read;
		if (ret.l >= buf_size - 1)
		{
			void *temp_buf = realloc(ret.m, buf_size *= 2);
			if (temp_buf == NULL)
			{
				free(ret.m);
				return (memblck) { NULL, 0 };
			}
			ret.m = temp_buf;
		}
	}

	return ret;
}
