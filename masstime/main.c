#include <inttypes.h>
#include <stdbool.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "autlib/arg_parse.h"

#define freep(p) (free(p), (p)=NULL)

void err_print(int32_t err_code, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	vfprintf(stderr, fmt, args);

	va_end(args);

	exit(err_code);
}

void print_usage(const char *program_name)
{
	err_print(1,
			"usage: %s [-Nh] --cmd <command_to_time>\n\n"
			
			"options:\n"
			"  --count [int], -N [int]        Execute the given command NUM times (default is 10)\n"
			"  --cmd [string], -c [string]    Specify the command to be timed (required)\n"
			"  --help, -h                     Display this help message\n"
			, program_name);
}

char *setup_cmd_str(const char *str_arr)
{
	const size_t total_len = strlen(str_arr) + sizeof(" 2>&1")-1;
	
	char *ret = malloc(total_len + 1);
	if (ret == NULL) return NULL;

	strlcpy(ret, str_arr, total_len + 1);
	strcat(ret, " 2>&1");

	return ret;
}

double time_program(char *cmd)
{
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);

	// will be writable at some point
	FILE *prog = popen(cmd, "r");
	if (prog == NULL) {
		fprintf(stderr, "failed to run command '%s'", cmd);
		return INFINITY; // error value
	}
	// discard output
	int c;
	while ((c = fgetc(prog)) != EOF)
		;

	pclose(prog);

	clock_gettime(CLOCK_MONOTONIC, &end);

	double ret = (end.tv_sec - start.tv_sec);
	ret += (end.tv_nsec - start.tv_nsec) / 1000000000.0;
	return ret;
}

char *full_cmd = NULL;

void signal_handler(int signum)
{
	free(full_cmd);
	printf("terminating with signal %d\n", signum);
	fputs("\x1b[?25h", stdout);

	exit(5);
}

int main(int argc, char **argv)
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);

	struct arg_parser ap = ap_create(argc, argv);
	const bool help_option = ap_long_short_option(&ap, "help", 'h', NULL, "bool") != NULL;
	const char *const count_option = ap_long_short_option(&ap, "count", 'N', NULL, "int");
	const char *const command_option = ap_long_short_option(&ap, "cmd", 'c', NULL, "string");
	ap_destroy(&ap);

	if (help_option) print_usage(argv[0]);
	if (command_option == NULL)
		err_print(1, "a command to be timed must be passed via the `--cmd` (or `-c`) flag (see `--help` for info)\n");

	uint64_t n_runs = 10;
	if (count_option != NULL) {
		char *end;
		const uint64_t tmp_n_runs = strtoull(count_option, &end, 10);
		if (*count_option == '\0' || *end != '\0') {
			fprintf(stderr, "invalid --count (or -N) argument '%s'; using default value 10", count_option);
		} else n_runs = tmp_n_runs;
	}

	int ret_code = 0;

	full_cmd = setup_cmd_str(command_option);
	if (full_cmd == NULL) {
		fprintf(stderr, "failed to allocate full command string, exiting...");
		ret_code = 1;
		goto cleanup_0;
	}

	printf("executing command \"%s\" %" PRIu64 " times...\n", full_cmd, n_runs);
	const size_t progress_bar_len = 40;
	fputs("\x1b[?25l[                                        ]", stderr);
	fflush(stdout);
	fflush(stderr);

	double sum_of_times = 0.0;
	for (uint64_t i = 0; i < n_runs; ++i)
	{
		const double duration = time_program(full_cmd);
		if (duration == INFINITY) {
			ret_code = 2;
			goto cleanup_0;
		}
		sum_of_times += duration;

		
		fputs("\r[", stderr);
		const size_t filled_progress_len = (double)(progress_bar_len*i) / n_runs + 1;
		assert(filled_progress_len <= progress_bar_len);
		for (size_t p = 0; p < filled_progress_len; ++p) {
			fputc('=', stderr);
		}
		for (size_t p = 0; p < (progress_bar_len - filled_progress_len); ++p) {
			fputc(' ', stderr);
		}
		fprintf(stderr, "] %" PRIu64 "/%" PRIu64, i, n_runs);
		fflush(stderr);
	}
	fputs("\x1b[2K\r", stderr);

	const double time_avg = sum_of_times / n_runs;

	printf("average time: %.10g seconds\n", time_avg);

cleanup_0:
	freep(full_cmd);
	fputs("\x1b[?25h", stderr);

	return ret_code;
}
