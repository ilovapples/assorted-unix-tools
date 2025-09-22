#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/statvfs.h>

#include "types.h"
#include "shared_stuff.h"

#define U64_MAX_STR_LEN 20
#define DEFAULT_PATH "/"

#define BIT(n) (1<<n)
#define SET_FLAG(f) (runtime_flags |= f)
#define FLAG_SET(f) ((runtime_flags & f) != 0)
#define IFDBG if (FLAG_SET(DEBUG))

enum {
	DEBUG = BIT(0),
	FRIENDLY_OUTPUT = BIT(1),
	USE_TIB = BIT(2),
	ONLY_UNIT = BIT(3),
	NO_UNIT = BIT(4),
};

static u8 runtime_flags = 0;
static u8 raw_output = 0;

static char *PROG_NAME, *DEVICE_PATH = DEFAULT_PATH;

void print_usage_msg(void)
{
	printf("usage: %s [options] [--device=<mounted_path_of_device>]\n\n"
		 
		 "  -d, --debug                            enable debug output\n"
		 "  { -D<DEVICE_MOUNT_PATH> |\n"
		 "    -D <DEVICE_MOUNT_PATH> |\n"
		 "    --device=<DEVICE_MOUNT_PATH> }       display info for the device mounted at DEVICE_MOUNT_PATH\n"
		 "  -h, --human-readable                   display sizes in larger sizes like KiB, MiB, or GiB\n"
		 "  { -r<ROW_NUM> |\n"
		 "    -r <ROW_NUM> |\n"
		 "    --raw=<ROW_NUM> |\n"
		 "    --row=<ROW_NUM> }                    display *only* the value for row 1, 2, or 3 (ROW_NUM) (in the order they appear without this option)\n"
		 "  -T, --tebibytes                        allow values to be displayed in TiB (tebibyte) units (only with -h)\n"
		 "  -U, --unit                             only display the unit of the selected row (only with -r and -h, mutually exclusive with -u)\n"
		 "  -u, --no-unit                          don't display the unit of the selected row (only with -r and -h, mutually exclusive with -U)\n"
		 "  --help                                 display this help message\n"
			, PROG_NAME);
	exit(0);
}

void arg_parse(s32 argc, char **argv)
{
	PROG_NAME = argv[0];
	for (char **cur_arg = argv + 1; cur_arg < argv + argc; cur_arg++) {
		if ((*cur_arg)[0] == '-') {
			if ((*cur_arg)[1] == '-') {
				IFDBG printf("checking argv[%zu] == \"%s\", \"--\" arg\n", cur_arg - argv, *cur_arg);
				if (strcmp(*cur_arg + 2, "debug") == 0)
					SET_FLAG(DEBUG);
				else if (strcmp(*cur_arg + 2, "human-readable") == 0)
					SET_FLAG(FRIENDLY_OUTPUT);
				else if (strcmp(*cur_arg + 2, "tebibytes") == 0)
					SET_FLAG(USE_TIB);
				else if (strcmp(*cur_arg + 2, "unit") == 0)
					SET_FLAG(ONLY_UNIT);
				else if (strcmp(*cur_arg + 2, "no-unit") == 0)
					SET_FLAG(NO_UNIT);
				else if (strncmp(*cur_arg + 2, "raw=", 4) == 0
					|| strncmp(*cur_arg + 2, "row=", 4) == 0)
					raw_output = *(*cur_arg + 2 + 4) - '0';
				else if (strncmp(*cur_arg + 2, "device=", 7) == 0 && strlen(*cur_arg) > 9)
					DEVICE_PATH = *cur_arg + 2 + 7;
				else if (strcmp(*cur_arg + 2, "help") == 0)
					print_usage_msg();
			} else {
				IFDBG printf("checking argv[%zu] == \"%s\", \"-\" arg\n", cur_arg - argv, *cur_arg);
				char *cur_char = *cur_arg;
				bool will_terminate = false;
				while (*(++cur_char) && !will_terminate) {
					switch (*cur_char) {
					case '-':
						error(-5280, "'-' characters are not allowed in short options past the first\n");
						will_terminate = true;
						break;
					case 'd':
						SET_FLAG(DEBUG);
						break;
					case 'h':
						SET_FLAG(FRIENDLY_OUTPUT);
						break;
					case 'T':
						SET_FLAG(USE_TIB);
						break;
					case 'U':
						if (FLAG_SET(NO_UNIT))
							error(-5280, "-U and -u (--unit and --no-unit) are mutually exclusive options.");
						SET_FLAG(ONLY_UNIT);
						break;
					case 'u':
						if (FLAG_SET(ONLY_UNIT))
							error(-5280, "-u and -U (--no-unit and --unit) are mutually exclusive options.");
						SET_FLAG(NO_UNIT);
						break;
					case 'r':
						if (isdigit(*(cur_char+1))) {
							raw_output = *(cur_char++ + 1) - '0';
						} else if (*(cur_arg+1) && isdigit(**(cur_arg+1))) {
							raw_output = **(cur_arg++ + 1) - '0';
							will_terminate = true;
						} else 
							print_usage_msg();
						break;
					case 'D':
						will_terminate = true;
						if (*(cur_char+1)) {
							DEVICE_PATH = cur_char + 1;
							break;
						} else if (*(cur_arg+1)) {
							DEVICE_PATH = *(cur_arg++ + 1);
							break;
						}
						print_usage_msg();
					default:
						error(-5280, "unknown option \"-%c\"\n", *cur_char);
						print_usage_msg();
					}
				}
			}
		}
	}
}

typedef u64 SIZE_UNIT;
#define B 1
#define KiB 1024
#define MiB 1048576
#define GiB 1073741824
#define TiB 1099511627776

SIZE_UNIT obtain_optimal_byte_size(u64 size_in_bytes)
{
	if (size_in_bytes >= TiB && FLAG_SET(USE_TIB))
		return TiB;
	if (size_in_bytes >= GiB)
		return GiB;
	if (size_in_bytes >= MiB)
		return MiB;
	if (size_in_bytes >= KiB)
		return KiB;
	return B;
}

char *convert_size_unit_to_unit_str(SIZE_UNIT size_unit)
{
	switch (size_unit) {
	case TiB:
		return "TiB";
	case GiB:
		return "GiB";
	case MiB:
		return "MiB";
	case KiB:
		return "KiB";
	default:
		return "B";
	}
}

s32 main(s32 argc, char **argv)
{
	arg_parse(argc, argv);


	struct statvfs file_stats;

	if (statvfs(DEVICE_PATH, &file_stats) < 0)
		error(1, "Failed to obtain file system statistics for the device '%s' (check for typos).\n", DEVICE_PATH); 
	
	u64 total_space_bytes = file_stats.f_blocks * file_stats.f_frsize;
	u64 free_space_bytes = file_stats.f_bfree * file_stats.f_frsize;
	u64 used_space_bytes = file_stats.f_frsize * (file_stats.f_blocks - file_stats.f_bfree);

	float used_percent = (double) used_space_bytes / total_space_bytes * 100;

	IFDBG {
		printf("Filesystem block size: %zu\n", file_stats.f_bsize);
		printf("Filesystem fragment size: %zu\n", file_stats.f_frsize);
		printf("Total blocks in '%s': %u\n", DEVICE_PATH, file_stats.f_blocks);
		printf("Number of free blocks: %u\n", file_stats.f_bfree);
		printf("runtime_flags = %#x\n", runtime_flags);
	}


	#define PERCENT_FORMAT "%4.2f"
	#define SIZE_UNIT_FORMAT "%.4f"
		
	if (FLAG_SET(FRIENDLY_OUTPUT)) {
		SIZE_UNIT total_space_size_multiple = obtain_optimal_byte_size(total_space_bytes),
			    free_space_size_multiple  = obtain_optimal_byte_size(free_space_bytes);
		float total_space_coeff = 0.0, free_space_coeff = 0.0, used_space_coeff = 0.0;
		if (!FLAG_SET(ONLY_UNIT)) {
			total_space_coeff = (double) total_space_bytes / total_space_size_multiple;
			free_space_coeff = (double) free_space_bytes / free_space_size_multiple;
			used_space_coeff = (double) used_space_bytes / free_space_size_multiple;
		}
		
		char *total_space_size_mult_str = convert_size_unit_to_unit_str(total_space_size_multiple),
		     *free_space_size_mult_str = convert_size_unit_to_unit_str(free_space_size_multiple);

		if (raw_output) {
			#define NUM_ROWS 4
			if (raw_output > NUM_ROWS)
				error(3, "invalid row input: %d\n", raw_output);

			float val = 0.0;
			char *unit_str = NULL;

			switch (raw_output) {
			case 1:
				val = total_space_coeff;
				unit_str = total_space_size_mult_str;
				break;
			case 2:
				val = free_space_coeff;
				unit_str = free_space_size_mult_str;
				break;
			case 3:
				val = used_space_coeff;
				unit_str = free_space_size_mult_str;
				break;
			case 4:
				val = used_percent;
				break;
			}

			if (FLAG_SET(ONLY_UNIT)) {
				if (raw_output == 4)
					error(2, "Row 4 (percent used) has no unit.\n");
				printf("%s", unit_str);
			} else if (FLAG_SET(NO_UNIT)) {
				printf((raw_output == 4) ? PERCENT_FORMAT : SIZE_UNIT_FORMAT, val);
			} else {
				if (raw_output == 4)
					printf(PERCENT_FORMAT "%%", val);
				else
					printf(SIZE_UNIT_FORMAT " %s", val, unit_str);
			}
		} else {
			printf("Total size of '%s': %8.4f %s\n", DEVICE_PATH, total_space_coeff, total_space_size_mult_str);
			printf("Free space in '%s': %8.4f %s\n", DEVICE_PATH, free_space_coeff, free_space_size_mult_str);
			printf("Used space in '%s': %8.4f %s\n", DEVICE_PATH, used_space_coeff, free_space_size_mult_str);
			printf("Percent used: " PERCENT_FORMAT "%%\n", used_percent);
		}
	} else {
		char total_space_str_buf[U64_MAX_STR_LEN+1];
		size_t total_space_str_len = snprintf(total_space_str_buf, U64_MAX_STR_LEN, "%" PRIu64, total_space_bytes);
		
		switch (raw_output) {
		case 1:
			printf("%s", total_space_str_buf);
			break;
		case 2:
			printf("%" PRIu64, free_space_bytes);
			break;
		case 3:
			printf("%" PRIu64, used_space_bytes);
			break;
		case 4:
			printf(PERCENT_FORMAT, used_percent);
			break;
		default:
			printf("Total size of '%s': %s B\n", DEVICE_PATH, total_space_str_buf);
			printf("Free space in '%s': %*" PRIu64 " B\n", DEVICE_PATH, (int)total_space_str_len, free_space_bytes);
			printf("Used space in '%s': %*" PRIu64 " B\n", DEVICE_PATH, (int)total_space_str_len, used_space_bytes);
			printf("Percent used: " PERCENT_FORMAT "%%\n", used_percent);
			break;
		}
	}


	return 0;
}
