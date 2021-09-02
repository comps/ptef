#include <errno.h>
#include <libgen.h>

#include "common.h"

// true if everything PASSed, false on error or FAIL
__asm__(".symver ptef_runner_v0, ptef_runner@@VERS_0.7");
__attribute__((used))
int ptef_runner_v0(int argc, char **argv, int jobs, int flags)
{
    if (argc < 1) {
        ERROR("need at least argv[0] for basename");
        return -1;
    }

    // sanitize
    if (jobs < 1)
        jobs = 1;

    int orig_errno = errno;

    char *base = getenv_defined("PTEF_BASENAME");
    if (!base)
        base = basename(argv[0]);

    argc--;
    argv++;

    int rc;
    if (argc <= 0) {
        rc = for_each_exec(base, jobs);
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
