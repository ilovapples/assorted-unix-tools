#include <signal.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define freep(p) (free(p), (p)=NULL)

const char *PROG_NAME = NULL;
char **CMD_START = NULL;

uint64_t count = 10;

void err_print(int32_t err_code, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vfprintf(stderr, fmt, args);

	va_end(args);

	exit(err_code);
}

void print_usage(void)
{
	err_print(1,
			"usage: %s [options] -- <cmd_with_args>\n\n"
			
			"--count=NUM, -CNUM		Execute the given command NUM times (default is 10)\n", PROG_NAME);
}

void arg_parse(int32_t argc, char **argv)
{
	int32_t arg_n = 0;
	PROG_NAME = argv[arg_n++];
	for (; arg_n < argc; ++arg_n)
	{
		if (strcmp(argv[arg_n], "--") == 0)
		{
			if (argv[arg_n] == NULL)
				err_print(1, "argument error: '--' option must be followed by a command\n");
			CMD_START = &argv[++arg_n];
			break;
		} else if (strncmp(argv[arg_n], "--", 2) == 0)
		{
			if (strncmp(argv[arg_n]+2, "count=", 6) == 0)
				count = strtoul(argv[arg_n]+2+6, NULL, 0);
			else
				err_print(1, "argument error: unknown option '%s'\n", argv[arg_n]);
		} else if (argv[arg_n][0] == '-')
		{
			char *c = argv[arg_n]+1;
			bool terminate_loop = false;
			while (*c && !terminate_loop)
			{
				switch (*c) {
				case 'C':
					count = strtoul(c+1, NULL, 0);
					terminate_loop = true;
					break;
				}
				++c;
			}
		}
	}

	if (CMD_START == NULL)
		print_usage();
}

char *flatten_str_arr(char **str_arr)
{
	size_t total_len = 0;
	char **s_p;
	for (s_p = str_arr; *s_p != NULL; ++s_p)
		total_len += strlen(*s_p)+1+2; // +1 for a space, +2 for quotes around it 
	++total_len;
	
	char *ret = malloc(sizeof(char) * total_len);
	for (s_p = str_arr; *s_p != NULL; ++s_p)
	{
		strcat(ret, "'");
		strcat(ret, *s_p);
		strcat(ret, "'");
		strcat(ret, " ");
	}

	return ret;
}

double time_program(char *cmd)
{
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);

	system(cmd);

	clock_gettime(CLOCK_MONOTONIC, &end);

	double ret = (end.tv_sec - start.tv_sec);
	ret += (end.tv_nsec - start.tv_nsec) / 1000000000.0;
	return ret;
}

double *times = NULL;
char *flat_cmd = NULL;

void signal_handler(int signum)
{
	free(times);
	free(flat_cmd);
	printf("terminating with signal %d\n", signum);

	exit(5);
}

int32_t main(int32_t argc, char **argv)
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);

	arg_parse(argc, argv);

	flat_cmd = flatten_str_arr(CMD_START);

	printf("executing command '%s' %lu times...\n", flat_cmd, count);

	times = calloc(count, sizeof(double));

	for (uint64_t i = 0; i < count; ++i)
	{
		times[i] = time_program(flat_cmd);
		//puts("\033[7m%\033[0m");
		fputc('\n', stdout);
	}

	double time_avg = 0;

	for (uint64_t i = 0; i < count; ++i)
		time_avg += times[i];
	time_avg /= count;

	printf("average time: %.10g seconds\n", time_avg);

	freep(times);
	freep(flat_cmd);

	return 0;
}
