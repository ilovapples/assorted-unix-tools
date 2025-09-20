#include <stdint.h>
#include <unistd.h>
#include "sysio.h"

int16_t getch(void)
{
	clrerr(fstdin, ERR_READ);
	Byte c;
	if (read(fstdin->fd, &c, 1) != 1) {
		seterr(fstdin, ERR_READ);
		return -1;
	}

	return (int16_t)c;
}

void putch(Byte c)
{
	clrerr(fstdout, ERR_WRITE);
	if (write(fstdout->fd, &c, 1) != 1)
		seterr(fstdout, ERR_WRITE);
}

void puthex(Byte c)
{
	static const char HEX_KEY[16] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
	};
	
	clrerr(fstdout, ERR_WRITE);
	char buf[2];
	buf[0] = HEX_KEY[c>>4];
	buf[1] = HEX_KEY[c & 0x0F];

	if (write(fstdout->fd, &buf, 2) != 2)
		seterr(fstdout, ERR_WRITE);
}
