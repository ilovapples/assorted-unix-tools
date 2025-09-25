#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include "term_set.h"

static struct termios orig_term;

void set_raw_mode(void)
{
	if (!isatty(STDIN_FILENO)) {
		write(2, "error: stdin is not a TTY\n", 26);
		exit(1);
	}
	
	if (tcgetattr(STDIN_FILENO, &orig_term) == -1) {
		write(2, "error: tcgetattr\n", 18);
		exit(1);
	}
	struct termios raw_term = orig_term;
	raw_term.c_lflag &= ~(ICANON | ECHO);
	raw_term.c_cc[VMIN] = 1;
	raw_term.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_term) == -1) {
		write(2, "error: tcsetattr\n", 18);
		exit(1);
	}

	atexit(restore_term);
}

void restore_term(void)
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_term);
}
