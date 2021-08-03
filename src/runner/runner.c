#include <errno.h>

#include "common.h"

// true if everything PASSed, false on error or FAIL
__asm__(".symver ptef_runner_v0, ptef_runner@@VERS_0");
__attribute__((used))
int ptef_runner_v0(int argc, char **argv, char *basename, int jobs, int nomerge)
{
    // sanitize
    if (jobs < 1)
        jobs = 1;

    int orig_errno = errno;

    int rc;
    if (argc <= 0) {
        rc = for_each_exec(basename, jobs);
    } else {
        if (nomerge)
            rc = for_each_arg(argc, argv, basename, jobs);
        else
            rc = for_each_merged_arg(argc, argv, basename, jobs);
    }

    if (rc != -1)
        errno = orig_errno;

    return rc;
}
