#ifndef AUT__SHARED_STUFF_H
#define AUT__SHARED_STUFF_H

#define BUF_SIZE 256

#include <stdio.h>

#include "types.h"

void error(s32 err_code, const char *fmt, ...) __attribute__((format (printf, 2, 3)));
memblck read_data_from_stream(FILE *stream);

#endif /* AUT__SHARED_STUFF_H */
