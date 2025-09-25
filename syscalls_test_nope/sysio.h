#ifndef SYSIO_H
#define SYSIO_H

#include <stdint.h>

#define BIT(i) (1<<(i))
#define errset(f, e) (((f)->err & (e)) != 0)
#define seterr(f, e) ((f)->err |= (e))
#define clrerr(f, e) ((f)->err &= ~(e))

typedef struct _file {
	int fd;
	uint32_t err;
	int8_t unavailable;
} File;

static struct _file files[20] = {
	{0, 0, 1},
	{1, 0, 1},
	{2, 0, 1}
};

#define stdin  0
#define stdout 1
#define stderr 2
#define fstdin  (&files[0])
#define fstdout (&files[1])
#define fstderr (&files[2])

#define getcp(c) read(fstdin, (c), 1)
#define putcp(c) write(fstdout, (c), 1)

typedef unsigned char Byte;

enum {
	ERR_READ  = BIT(0),
	ERR_WRITE = BIT(1)
};

int16_t getch(void);
void putch(Byte c);

void puthex(Byte c);

#endif /* SYSIO_H */
