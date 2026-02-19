#include "include/aut_lib.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

char const *PROGRAM_NAME;

#define USAGE_MSG "usage: %s <tool> [args..]\n"

int main(int argc, char *argv[argc])
{
    PROGRAM_NAME = argv[0];

    if (argc < 2) {
        fprintf(stderr, "%s: a tool name must be specified ('help' for a complete list)\n" USAGE_MSG, PROGRAM_NAME, PROGRAM_NAME);
        return 1;
    }

    char const *const tool_name = argv[1];

    int exit_code = 0;

    size_t const max_path_len = pathconf(".", _PC_PATH_MAX);
    char *const full_bindir_path = malloc(max_path_len + 1);
    if (!expand_bindir_path(
                max_path_len+1,
                full_bindir_path,
                APPLE_AUT_BINDIR,
                tool_name))
    {
        exit_code = 1;
        goto cleanup0;
    }

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "%s: failed to fork\n", PROGRAM_NAME);
        exit_code = 2;
        goto cleanup0;
    } else if (pid == 0) {
        char *const envp[] = {"CHILD_OF_APPLE_AUT=1", NULL};
        exit_code = execve(full_bindir_path, &argv[1], envp);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
            goto cleanup0;
        }
    }

cleanup0:
    free(full_bindir_path);

    return exit_code;
}
