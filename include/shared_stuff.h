#ifndef SHARED_STUFF_H
#define SHARED_STUFF_H

#define BUF_SIZE 256

#include <stdio.h>

#include "types.h"

void error(s32 err_code, const char *fmt, ...);
strbuf read_string_from_stream(FILE *stream);

#endif /* SHARED_STUFF_H */
