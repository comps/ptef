#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "common.h"

// retry on EINTR
static pid_t waitpid_safe(pid_t pid, int *wstatus, int options)
{
    pid_t ret;
    while ((ret = waitpid(pid, wstatus, options)) == -1) {
        if (errno != EINTR)
            return -1;
    }
    return ret;
}

static void execute_child(char *basename, char **argv, char *dir)
{
    int logfd = -1;
    char *tmp = NULL;
    int errout = -1;

    // always export the runner basename, don't overwrite
    if (setenv("PTEF_BASENAME", basename, 0) == -1) {
        PERROR("setenv(PTEF_BASENAME, ..)");
        goto err;
    }

    // if we're descending into subdir, use the directory name,
    // else use the executable name
    char *testname = dir ? dir : argv[0];

    char *ptef_nologs = getenv_defined("PTEF_NOLOGS");

    // open a log file, duplicate it to stderr
    // - this has to be done prior to prefix adjustment below
    //   as ptef_mklog() already appends the test name and .log
    if (!ptef_nologs) {
        if ((logfd = ptef_mklog(testname, 0)) == -1) {
            PERROR_FMT("ptef_mklog(%s)", testname);
            goto err;
        }
    }

    char *ptef_prefix = getenv_defined("PTEF_PREFIX");
    if (!ptef_prefix)
        ptef_prefix = "";

    // add the current testname (subdir name) to PTEF_PREFIX
    size_t testname_len = strlen(testname);
    size_t ptef_prefix_len = strlen(ptef_prefix);
    // +2 for '/' and '\0'
    if ((tmp = malloc(ptef_prefix_len+testname_len+2)) == NULL) {
        PERROR("malloc");
        goto err;
    }
    char *pos;
    pos = memcpy_append(tmp, ptef_prefix, ptef_prefix_len);
    *pos++ = '/';
    pos = memcpy_append(pos, testname, testname_len);
    *pos = '\0';
    if (setenv("PTEF_PREFIX", tmp, 1) == -1) {
        PERROR("setenv(PTEF_PREFIX, ..)");
        goto err;
    }

    if (dir) {
        // if PTEF_LOGS has relative path, prepend '../'
        char *ptef_logs = getenv_defined("PTEF_LOGS");
        if (ptef_logs && *ptef_logs != '/') {
            free(tmp);
            size_t ptef_logs_len = strlen(ptef_logs);
            // +4 for '../' and '\0'
            if ((tmp = malloc(ptef_logs_len+4)) == NULL) {
                PERROR("malloc");
                goto err;
            }
            pos = memcpy_append(tmp, "../", 3);
            pos = memcpy_append(pos, ptef_logs, ptef_logs_len);
            *pos = '\0';
            if (setenv("PTEF_LOGS", tmp, 1) == -1) {
                PERROR("setenv(PTEF_LOGS, ..)");
                goto err;
            }
        }

        // cd into a subrunner directory
        if (chdir(dir) == -1) {
            PERROR_FMT("chdir(%s)", dir);
            goto err;
        }
    }

    if (ptef_nologs) {
        // skip output redirection logic below, execv() directly
        if (execv(argv[0], argv) == -1)
            PERROR_FMT("execv(%s,..)", argv[0]);
        // shouldn't be reached
        goto err;
    }

    // replace stderr with logfd from ptef_mklog
    //
    // if we have O_CLOEXEC, dup the original stderr to a higher-up fd,
    // so we can still report errors from execv() to the original output
    //
    // if we don't have O_CLOEXEC, these errors will go to the logfile
#ifdef O_CLOEXEC
    if ((errout = dup(DEFAULT_ERROR_FD)) == -1) {
        PERROR("dup(" DEFAULT_ERROR_FD_STR ")");
        goto err;
    }
    // dup2() closes newfd if open
    if (dup2_safe(logfd, DEFAULT_ERROR_FD) == -1) {
        PERROR_FMT_FD(errout, "dup2(%d," DEFAULT_ERROR_FD_STR ")", logfd);
        goto err;
    }
    if (fcntl(errout, F_SETFD, FD_CLOEXEC) == -1) {
        PERROR_FD(errout, "fcntl(.., F_SETFD, O_CLOEXEC)");
        goto err;
    }
#else
    if (dup2_safe(logfd, DEFAULT_ERROR_FD) == -1) {
        PERROR_FMT("dup2(%d," DEFAULT_ERROR_FD_STR ")", logfd);
        goto err;
    }
#endif

    // logfd has been dup2()'d to 2, close the original fd from ptef_mklog()
    close_safe(logfd);
    logfd = -1;

    if (execv(argv[0], argv) == -1) {
#ifdef O_CLOEXEC
        PERROR_FMT_FD(errout, "execv(%s,..)", argv[0]);
#else
        PERROR_FMT("execv(%s,..)", argv[0]);
#endif
        goto err;
    }

    // this shouldn't be reachable,
    // so fall through to 'err' and exit with failure
err:
    free(tmp);
    close_safe(logfd);
    close_safe(errout);
    exit(1);
}

static char *std_exit_statuses[256] = {
    [0] = "PASS",
};
char **ptef_exit_statuses = std_exit_statuses;
char *ptef_exit_statuses_default = "FAIL";

static int start_job(pid_t pid, char *name, struct exec_state *state)
{
    if (getenv_defined("PTEF_RUN"))
        if (ptef_report("RUN", name, 0) == -1)
            return -1;
    // find a (guaranteed) free slot in pid-to-name map and use it up
    struct pid_to_name *map = state->map;
    int i;
    for (i = 0; map[i].pid != -1; i++);
    map[i].pid = pid;
    map[i].name = name;
    state->running_jobs++;
    return 0;
}
static int finish_job(pid_t pid, struct exec_state *state, int exitcode)
{
    struct pid_to_name *map = state->map;
    // since we always use the first found '-1', it is guaranteed that we'll
    // find our pid on idx < running_jobs, or not at all
    int i, maxjobs = state->max_jobs;
    for (i = 0; i < maxjobs && map[i].pid != pid; i++);
    if (i >= maxjobs) {
        ERROR_FMT("pid %d not ours\n", pid);
        // technically not our problem, we can just skip it
        return 0;
    }
    char *status = ptef_exit_statuses[exitcode];
    if (!status)
        status = ptef_exit_statuses_default;
    int report_rc = ptef_report(status, map[i].name, 0);
    // TODO: won't be needed once free() guarantees no errno changes
    int report_errno = errno;
    // reset the entry
    free(map[i].name);
    map[i].pid = -1;
    state->running_jobs--;
    errno = report_errno;
    return report_rc;
}

// allocate enough for exec_state itself + for all the pid-to-name
// mappings we'll ever need (there won't be more than maxjobs children)
struct exec_state *create_exec_state(int maxjobs)
{
    struct exec_state *state;
    state = malloc(sizeof(struct exec_state)
                   + sizeof(struct pid_to_name) * maxjobs);
    if (state != NULL) {
        state->max_jobs = maxjobs;
        state->running_jobs = 0;
        struct pid_to_name empty = { -1, NULL };
        for (int i = 0; i < state->max_jobs; i++) {
            memcpy(&state->map[i], &empty, sizeof(empty));
        }
    }
    return state;
}
int destroy_exec_state(struct exec_state *state)
{
    int ret = 0;
    pid_t child;
    int wstatus;
    // finish all jobs
    while (state->running_jobs > 0) {
        if ((child = waitpid_safe(-1, &wstatus, 0)) > 0) {
            // yes, this can repeatedly call finish_job() on error,
            // but what's the worst that can happen? .. we'd need to
            // collect all children later in this func anyway
            if (finish_job(child, state, WEXITSTATUS(wstatus)) == -1)
                ret = -1;
        } else {
            PERROR("waitpid");
            ret = -1;
        }
    }
    free(state);
    return ret;
}

// execute an executable file in CWD or a directory with an executable file
// named after basename
// - argv must have [0] allocated and unused (to be used for argv[0])
//   and be terminated at [1] or later with NULL
int execute(char *file, enum exec_entry_type typehint, char **argv,
            char *basename, struct exec_state *state)
{
    if (typehint == EXEC_TYPE_UNKNOWN) {
        if (fstatat_type(AT_FDCWD, file, &typehint) == -1) {
            PERROR_FMT("fstatat %s", file);
            return -1;
        }
    }

    char *child_dir;
    switch (typehint) {
        case EXEC_TYPE_FILE:
            child_dir = NULL;
            argv[0] = file;
            break;
        case EXEC_TYPE_DIR:
            child_dir = file;
            argv[0] = basename;  // PTEF_BASENAME or argv0 of the runner
            break;
        default:
            ERROR_FMT("invalid exec type %d\n", typehint);
            return -1;
    }

    // reap any zombies that already exited between execute() calls
    pid_t child;
    int wstatus;
    while ((child = waitpid_safe(-1, &wstatus, WNOHANG)) > 0)
        if (finish_job(child, state, WEXITSTATUS(wstatus)) == -1)
            return -1;
    if (child == -1 && errno != ECHILD) {
        PERROR("waitpid WNOHANG");
        return -1;
    }

    // duplicate filename for storage in pid<->testname map array
    // this belongs to start_job(), but we don't want to deal with a fork()ed
    // child if this fails, so do it early
    char *name_duped;
    if ((name_duped = strdup(file)) == NULL) {
        PERROR("strdup");
        return -1;
    }

    child = fork();
    switch (child) {
        case -1:
            free(name_duped);
            return -1;
        case 0:
            execute_child(basename, argv, child_dir);
            break;
        default:
            if (start_job(child, name_duped, state) == -1) {
                free(name_duped);
                return -1;
            }
            break;
    }

    // if we're at maximum of running jobs, block until one exits
    if (state->running_jobs >= state->max_jobs) {
        if ((child = waitpid_safe(-1, &wstatus, 0)) > 0) {
            if (finish_job(child, state, WEXITSTATUS(wstatus)) == -1)
                return -1;
        } else {
            PERROR("waitpid");
            return -1;
        }
    }

    return 0;
}
