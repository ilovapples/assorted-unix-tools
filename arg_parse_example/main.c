#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

size_t ap_options_column_2_offset;

#include "arg_parse.h"
#include "util.h"

int32_t main(int32_t argc, char **argv) {
      ap_options_column_2_offset = 34;

      struct arg_parser cleanup(ap_destroy) ap = ap_create(argc, argv);
      bool help_option = ap_long_short_option(&ap, "help", 'h', "show help message", "bool") != NULL;
      const char *const option_name = ap_positional_option(&ap, "option_name", "name of option to search for", "string");
      const char *filename = ap_positional_option(&ap, "filename", "path to config file (optional, defaults to 'config.txt')", "string");
      filename = (filename) ? filename : "./config.txt";
      if (help_option || !option_name) {
            if (!help_option) fprintf(stderr, "arg_parse_example: argument error: an option name must be passed\n");
            ap_print_declared_options(ap, stderr);
            return 1;
      }

      printf("filename = '%s', option_name = '%s'\n", filename, option_name);

      return 0;
}
