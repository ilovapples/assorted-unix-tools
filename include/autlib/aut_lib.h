#ifndef APPLE__AUT_LIB_H
#define APPLE__AUT_LIB_H

/// Any program that includes this header requires a `const char *PROGRAM_NAME;`
/// global-scope declaration.

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef APPLE_AUT_BINDIR
#define APPLE_AUT_BINDIR "%H/.local/bin/apple-aut-bin"
#endif

#define APPLE_AUT_BIN_NAME_MAX 128

// "%H" expands into the $HOME environment variable.
char *expand_bindir_path(size_t buf_size, char buffer[static buf_size], char const *bindir, char const *tool_name);

void autlib_print_progress_bar_ex(
        uint64_t completed,
        uint64_t total,
        size_t bar_len,
        char bar_left_delimiter,
        char bar_right_delimiter,
        char filled,
        char unfilled,
        FILE *fp);
// thin wrapper around `autlib_print_progress_bar_ex` with defaults "[]", '=', ' '
void autlib_print_progress_bar(uint64_t completed, uint64_t total, size_t bar_len, FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /* APPLE__AUT_LIB_H */
