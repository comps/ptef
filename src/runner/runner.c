#include <errno.h>
#include <unistd.h>
#include <libgen.h>

#include "common.h"

static void run_shell(char *shell)
{
    if (dup2_safe(1, 2) == -1) {
        PERROR("dup2(1,2)");
        return;
    }
    if (access(shell, X_OK) == -1) {
        if (errno != ENOENT) {
            PERROR_FMT("access(%s)", shell);
            return;
        }
        shell = getenv_defined("SHELL");
        if (!shell)
            shell = "/bin/sh";
    }
    char *argv[2];
    argv[0] = shell;
    argv[1] = NULL;
    if (execv(argv[0], argv) == -1)
        PERROR_FMT("execv(%s,..)", argv[0]);
}

// true if everything PASSed, false on error or FAIL
__asm__(".symver ptef_runner_v0, ptef_runner@@VERS_0.7");
__attribute__((used))
int ptef_runner_v0(int argc, char **argv, int jobs, char **ignored, int flags)
{
    if (argc < 1) {
        ERROR("need at least argv[0] for basename");
        return -1;
    }

    // running without additional args
    if (argc == 1) {
        char *shell = getenv_defined("PTEF_SHELL");
        if (shell) {
            run_shell(shell);
            return -1;  // shouldn't have returned
        }
    }

    // sanitize
    if (jobs < 1)
        jobs = 1;

    int orig_errno = errno;

    char *base = getenv_defined("PTEF_BASENAME");
    if (!base)
        base = basename(argv[0]);

    int rc;
    if (argc < 2) {
        rc = for_each_exec(base, jobs, ignored);
    } else {
        if (flags & PTEF_NOMERGE)
            rc = for_each_arg(argc, argv, base, jobs);
        else
            rc = for_each_merged_arg(argc, argv, base, jobs);
    }

    if (rc != -1)
        errno = orig_errno;

    return rc;
}
