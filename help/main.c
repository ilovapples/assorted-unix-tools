#include "include/aut_lib.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

char const *PROGRAM_NAME;

bool is_dot_or_dotdot(char const *filename)
{
    return filename[0] == '.' && (filename[1] == '\0' ||
          (filename[1] == '.' && filename[2] == '\0'));
}

int main(int argc, char *argv[argc])
{
    PROGRAM_NAME = argv[0];

    fprintf(stderr, "Available tools:\n");

    int exit_code = 0;

    size_t const max_path_len = pathconf(".", _PC_PATH_MAX);
    char *const full_bindir_path = malloc(max_path_len + 1);
    char *path_end;
    if (!(path_end = expand_bindir_path(max_path_len+1, full_bindir_path, APPLE_AUT_BINDIR, NULL)))
    {
        exit_code = 1;
        goto cleanup0;
    }

    size_t buffer_remaining = max_path_len - (path_end - full_bindir_path);

    path_end += strlcpy(path_end, "/", buffer_remaining++);

    // logic yoinked from https://stackoverflow.com/a/1274167

    // iterate through bin directory
    struct dirent *dp;
    DIR *dfd;

    if ((dfd = opendir(full_bindir_path)) == NULL) {
        fprintf(stderr, "%s: failed to open directory '%s'\n", PROGRAM_NAME,
            full_bindir_path);
        exit_code = 2;
        goto cleanup0;
    }

    while ((dp = readdir(dfd)) != NULL)
    {
        if (is_dot_or_dotdot(dp->d_name)) continue;

        strlcpy(path_end, dp->d_name, buffer_remaining);

        struct stat stbuf;
        if (stat(full_bindir_path, &stbuf) == -1) {
            fprintf(stderr, "%s: failed to stat file '%s'\n", PROGRAM_NAME, full_bindir_path);
            exit_code = 3;
            goto cleanup1;
        }
;
        if ((stbuf.st_mode & S_IEXEC) == S_IEXEC)
            puts(dp->d_name);
    }

cleanup1:
    closedir(dfd);
cleanup0:
    free(full_bindir_path);

    return exit_code;
}
