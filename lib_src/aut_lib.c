#include "include/aut_lib.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

extern char const *const PROGRAM_NAME;

// "%H" expands into the $HOME environment variable.
char *expand_bindir_path(size_t buf_size, char buffer[static buf_size], char const *bindir, char const *tool_name)
{
    char *const HOME_SPEC = "%H";
    size_t const HOME_SPEC_LEN = 2;

    size_t end = 0;

    char const *const home_spec_pos = strstr(bindir, HOME_SPEC);
    if (home_spec_pos) {
        memcpy(buffer, bindir, home_spec_pos - bindir);
        end += home_spec_pos - bindir;
        char *home_env = getenv("HOME");
        if (home_env == NULL) {
            fprintf(stderr, "%s: $HOME environment variable is not defined\n", PROGRAM_NAME);
            return NULL;
        }
        end += strlcat(buffer, home_env, buf_size);
        end += strlcat(buffer, home_spec_pos + HOME_SPEC_LEN, buf_size);
    } else {
        end += strlcpy(buffer, bindir, buf_size);
    }

    if (tool_name) {
        end += strlcat(buffer, "/", buf_size);
        end += strlcat(buffer, tool_name, buf_size);
    }
    
    return buffer + end;
}
