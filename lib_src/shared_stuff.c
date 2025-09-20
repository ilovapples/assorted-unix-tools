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

