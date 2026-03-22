#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// DOES NOT RETURN A C-STRING
char *read_file_alloc(FILE *fp, size_t *length_out)
{
    #define INITIAL_ALLOC_SIZE 512
    size_t allocd_len = 512;
    size_t length = 0;
    char *mem = malloc(INITIAL_ALLOC_SIZE);
    if (!mem) return NULL;

    size_t read_size;
    while ((read_size = fread(mem + length, 1, allocd_len - length, fp)) > 0)
    {
        length += read_size;
        if (read_size == allocd_len - length) {
            char *tmp = realloc(mem, allocd_len *= 2);
            if (!tmp) {
                free(mem);
                return NULL;
            }
            mem = tmp;
        }
    }

    *length_out = length;
    if (length == 0) return mem;

    char *tmp = realloc(mem, length);
    if (!tmp) return mem;

    return tmp;

    #undef INITIAL_ALLOC_SIZE
}

int main(int argc, char **argv)
{
    FILE *fp = (argc > 1) ? fopen(argv[1], "rb") : stdin;
    if (!fp) {
        fprintf(stderr, "failed to open file '%s' for reading\n", argv[1]);
        return 2;
    }

    size_t length;
    char *instrs = read_file_alloc(fp, &length);
    if (argc > 1) fclose(fp);
    if (!instrs) {
        fprintf(stderr, "failed to allocate memory for file contents\n");
        return 3;
    }

    uint8_t data[32768] = {0};
    char *instr_ptr = instrs;
    uint8_t *data_ptr = data;
    size_t n_nested = 0, save_nested = 0;

    while (instr_ptr < instrs + length)
    {
        switch (*instr_ptr) {
            case '#':;
                char *p = strchr(instr_ptr, '\n');
                if (!p) goto break_outer;
                instr_ptr = p;
                break;
            case '>':
                data_ptr = (data_ptr - data == sizeof(data)-1)
                    ? data
                    : data_ptr + 1;
                break;
            case '<':
                data_ptr = (data_ptr == data)
                    ? data_ptr + sizeof(data)-1
                    : data_ptr - 1;
                break;
            case '+':
                ++*data_ptr;
                break;
            case '-':
                --*data_ptr;
                break;
            case '.':
                putc(*data_ptr, stdout);
                break;
            case ',':
                *data_ptr = getc(stdin);
                break;
            case '[':
                if (*data_ptr != 0) break;
                save_nested = n_nested++;
                while (save_nested != n_nested)
                {
                    ++instr_ptr;
                    if (instr_ptr == instrs + length)
                        goto break_outer;
                    else if (*instr_ptr == '[')
                        ++n_nested;
                    else if (*instr_ptr == ']')
                        --n_nested;
                }
                break;
            case ']':
                if (*data_ptr == 0) break;
                save_nested = n_nested++;
                while (save_nested != n_nested)
                {
                    if (instr_ptr == instrs)
                        goto break_outer;
                    --instr_ptr;
                    if (*instr_ptr == ']')
                        ++n_nested;
                    else if (*instr_ptr == '[')
                        --n_nested;
                }
                break;
        }

        ++instr_ptr;

    }
break_outer:

    free(instrs);

    return 0;
}
