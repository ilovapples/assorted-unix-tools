#ifndef APPLE__AUT_LIB_H
#define APPLE__AUT_LIB_H

#include <stdbool.h>
#include <stddef.h>

#ifndef APPLE_AUT_BINDIR
#define APPLE_AUT_BINDIR "%H/.local/bin/apple-aut-bin"
#endif

#define APPLE_AUT_BIN_NAME_MAX 128

// "%H" expands into the $HOME environment variable.
char *expand_bindir_path(size_t buf_size, char buffer[static buf_size], char const *bindir, char const *tool_name);

#endif /* APPLE__AUT_LIB_H */
