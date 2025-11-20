#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// uses a static buffer, so previous return values are invalidated when called again
const char *escape_char(char *out, char c)
{
      static char escape_buf[5] = {0};
      snprintf(escape_buf, 5, (isprint(c)) ? "%c" : "<%.2x>", c);
      if (out) strcpy(out, escape_buf);
      return (out) ? out : escape_buf;
}

void print_usage(const char *prog_name)
{
      fprintf(stderr, "usage: %s <against> [source]\n", prog_name);
}

int main(int argc, char **argv)
{
      const char *filename = NULL; // always second argument if present; otherwise stdin
      const char *against = NULL; // always first argument
      if (argc > 2) {
            against = argv[1];
            filename = argv[2];
      } else if (argc > 1) {
            against = argv[1];
      } else {
            fprintf(stderr, "%s: at least one filename must be passed\n", argv[0]);
            print_usage(argv[0]);
            return 1;
      }

      FILE *sfp = (filename != NULL) ? fopen(filename, "rb") : stdin;
      if (sfp == NULL) {
            fprintf(stderr, "%s: failed to open source file '%s'\n", argv[0], filename);
            return 2;
      }
      FILE *afp = fopen(against, "rb");
      if (afp == NULL) {
            fprintf(stderr, "%s: failed to open file '%s'\n", argv[0], against);
            return 2;
      }

      for (size_t n_match = 0;; ++n_match) {
            const int sc = fgetc(sfp);
            const int ac = fgetc(afp);
            if (sc == EOF) {
                  printf("The source '%s' matches with the check file '%s' up to the end of"
                              " the source.\n",
                              (filename) ? filename : "<stdin>", against);
                  break;
            } else if (ac == EOF) {
                  printf("The source '%s' matches with the check file '%s' up to the end of"
                              " the check file.\n",
                              (filename) ? filename : "<stdin>", against);
                  break;
            }

            if (sc != ac) {
                  char src_char_buf[5];
                  escape_char(src_char_buf, sc);
                  printf("%zu characters matched before non-match '%s' != '%s' found.\n",
                              n_match, src_char_buf, escape_char(NULL, ac));
                  break;
            }
      }


      if (filename != NULL) fclose(sfp);
      fclose(afp);
      return 0;
}
