#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// config file spec:
//
// ```
// option_name = 'value'
// ```
//
// the delimiter (single quote in this case) is whatever is the first
// non-whitespace character after the '='.
//
// values are read raw from start to end delimiter.
//
// whitespace around '=' does not matter.
//
// no option other than the one being searched for will be checked.
//
// the 'option_name = ' part must be the start of the line it's on.

#define PROGRAM_NAME "config_reader"
#define LINE_BUFFER_SIZE 8192

void fail_with_error_code(int32_t code, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void debug_print(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

const char *const USAGE_MSG =
"usage: %1$s <config_file> <option_name>\n"
"       %1$s <option_name>\n"
"prints to stdout the value of 'option_name' found in 'config_file'\n"
"(defaults to './config.txt' if unspecified)\n";

FILE *fp;

int32_t main(int32_t argc, char **argv) {
      if (argc < 2) {
            fprintf(stderr, USAGE_MSG, argv[0]);
            return 1;
      }

      const char *const config_file = (argc > 2) ? argv[1] : "./config.txt";
      const char *const option_name = (argc > 2) ? argv[2] : argv[1];
      const size_t option_name_len = strlen(option_name);

      fp = fopen(config_file, "rb");
      if (!fp)
            fail_with_error_code(2, "failed to open file '%s' for reading", config_file);

      char line_buffer[LINE_BUFFER_SIZE];

      // loop to find option
      size_t line_num = 1;
      for (;;) {
            if (fgets(line_buffer, LINE_BUFFER_SIZE, fp) == NULL) {
                  if (ferror(fp)) {
                        fail_with_error_code(2,
                                    "an error occurred while reading from file '%s'",
                                    config_file);
                  } else {// eof
                        fail_with_error_code(3,
                                    "failed to find option '%s' in file '%s'",
                                    option_name, config_file);
                  }
            }

            if (strncmp(line_buffer, option_name, option_name_len) == 0) {
                  const char after_option = line_buffer[option_name_len];
                  if (isspace(after_option) || after_option == '=')
                        break;
            }

            ++line_num;
      }

      const size_t found_on_line = line_num;

      // search for '=' char
      char *equals_pos = line_buffer + option_name_len;
      while (isspace((unsigned char)*equals_pos))
            ++equals_pos;
      if (*equals_pos == '\0') {
            fail_with_error_code(3,
                        "end of line encountered immediately after option name '%s' on line %zu",
                        option_name, found_on_line);
      } else if (*equals_pos != '=') {
            fail_with_error_code(3,
                        "expected '=', found '%c', after option name '%s' on line %zu",
                        *equals_pos, option_name, found_on_line);
      }

      // search for delimiter after '='
      char *start_delimiter_pos = equals_pos + 1;
      while (isspace((unsigned char)*start_delimiter_pos))
            ++start_delimiter_pos;
      if (*start_delimiter_pos == '\0') {
            fail_with_error_code(3,
                        "end of line encountered after '=' following option name '%s' on line %zu",
                        option_name, found_on_line);
      }
      const char delimiter = *start_delimiter_pos;

      char *start_search_pos = start_delimiter_pos + 1;
      for (;;) {
            char *const potential_end_pos = strchr(start_search_pos, delimiter);
            if (potential_end_pos != NULL) {
                  *potential_end_pos = '\0';
                  fputs(start_search_pos, stdout);
                  return 0;
            }

            fputs(start_search_pos, stdout);

            start_search_pos = line_buffer;

            if (fgets(line_buffer, LINE_BUFFER_SIZE, fp) == NULL) {
                  if (ferror(fp)) {
                        fail_with_error_code(2,
                                    "an error occurred while reading from file '%s'",
                                    config_file);
                  } else {// eof
                        fail_with_error_code(3,
                                    "unterminated value for option '%s' starts on line %zu",
                                    option_name, found_on_line);
                  }
            }

            ++line_num;
      }

      fclose(fp);

      return 0;
}

void fail_with_error_code(int32_t code, const char *fmt, ...)
{
      fputs(PROGRAM_NAME ": ", stderr);
      va_list ap;
      va_start(ap, fmt);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      fputc('\n', stderr);

      if (fp) fclose(fp);
      exit(code);
}

void debug_print(const char *fmt, ...)
{
      fputs(PROGRAM_NAME " [DEBUG]: ", stderr);
      va_list ap;
      va_start(ap, fmt);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      fputc('\n', stderr);
}
