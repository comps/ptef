#include <errno.h>

#include "common.h"

// true if everything PASSed, false on error or FAIL
__asm__(".symver ptef_runner_v0, ptef_runner@@VERS_0.7");
__attribute__((used))
int ptef_runner_v0(int argc, char **argv, char *default_basename, int jobs,
                   int mark_interval, int flags)
{
    // sanitize
    if (jobs < 1)
        jobs = 1;

    int orig_errno = errno;

    char *basename = getenv_defined("PTEF_BASENAME");
    if (!basename)
        basename = default_basename;

    struct sigaction old_alarm;
    struct itimerval old_timer;
    if (mark_interval > 0)
        if (setup_mark(mark_interval, &old_alarm, &old_timer) == -1)
            return -1;

    int rc;
    if (argc <= 0) {
        rc = for_each_exec(basename, jobs, flags);
    } else {
        if (flags & PTEF_NOMERGE)
            rc = for_each_arg(argc, argv, basename, jobs, flags);
        else
            rc = for_each_merged_arg(argc, argv, basename, jobs, flags);
    }

    if (mark_interval > 0)
        teardown_mark(&old_alarm, &old_timer);

    if (rc != -1)
        errno = orig_errno;

    return rc;
}
