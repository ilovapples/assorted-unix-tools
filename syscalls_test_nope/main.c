#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include "term_set.h"
#include "sysio.h"

int32_t main(void)
{
	set_raw_mode();
	atexit(restore_term);

	char c;

	while ((c = getch()) > 0) {
		if (c == 27) return 0;

		puthex((Byte) c);
	}

	return 0;
}
