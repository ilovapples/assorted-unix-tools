#include <stdbool.h>
#include <stdio.h>

char const *PROGRAM_NAME = "add-line-nums";

int main(int argc, char **argv)
{
    char const *filename = (argc > 1) ? argv[1] : NULL;
    FILE *fp = (filename) ? fopen(filename, "rb") : stdin;
    if (!fp) {
        fprintf(stderr, "%s: failed to open file '%s' for reading", PROGRAM_NAME, filename);
        return 1;
    }

    size_t line_num = 0;
    int c;
    bool line_changed = true;
    while ((c = fgetc(fp)) != EOF) {
        if (line_changed) {
            printf("%zu:", ++line_num);
            line_changed = false;
        }
        if (c == '\n') line_changed = true;
        fputc(c, stdout);
    }

    if (filename) fclose(fp);

    return 0;
}
