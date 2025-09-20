#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include <ctype.h>

#include "types.h"
#include "shared_stuff.h"

#define xdig2num(d) (('a'<=d && d<='f') \
			? d - 'a' + 0xA \
			: d - '0')

#define WESC_RGB L"\x1b[38;2;"
#define WESC_END L"\x1b[0m"

s32 main(s32 argc, char **argv)
{
	setlocale(LC_ALL, "");

	if (argc != 2)
		error(1, "Incorrect usage. Requires a color argument in hex format.\n");

	char color_buf[6+1] = {0}, *cur_pos = color_buf;
	char *arg_color = argv[1];
	while (*arg_color) {
		if (cur_pos >= color_buf + 6)
			break;
		else if (isxdigit(*arg_color))
			*cur_pos++ = *arg_color;
		arg_color++;
	}
	while (cur_pos < color_buf + 6)
		*cur_pos++ = '0';
	
	char to_lower_buf[6+1] = {0};
	char *c1 = to_lower_buf, *c2 = color_buf;
	while (*c1++ = tolower(*c2++))
		;

	u8 rv = (xdig2num(to_lower_buf[0]) << 4) + xdig2num(to_lower_buf[1]);
	u8 gv = (xdig2num(to_lower_buf[2]) << 4) + xdig2num(to_lower_buf[3]);
	u8 bv = (xdig2num(to_lower_buf[4]) << 4) + xdig2num(to_lower_buf[5]);
	
	const wchar_t full_block_wchar = L'\x2588';
	wprintf(WESC_RGB L"%u;%u;%um%lc%4$lc" WESC_END L"\n", rv, gv, bv, full_block_wchar);

	return 0;
}
