#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <termios.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <errno.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ptef.h>
#include <ptef_helpers.h>

// in case we get interrupted by a signal
static int intr_safe_setlkw(int fd, struct flock *f)
{
    while (fcntl(fd, F_SETLKW, f) == -1) {
        if (errno != EINTR) {
            PERROR_FMT("fcntl(%d, F_SETLKW)", fd);
            return -1;
        }
    }
    return 0;
}
static int setlk(int fd, struct flock *f)
{
    if (fcntl(fd, F_SETLK, f) == -1) {
        // POSIX allows both, unify them under EAGAIN
        if (errno == EACCES)
            errno = EAGAIN;
        if (errno != EAGAIN)
            PERROR_FMT("fcntl(%d, F_SETLK)", fd);
        return -1;
    }
    return 0;
}
static int lock(int fd, int flags)
{
    if (flags & PTEF_NOLOCK)
        return 0;
    struct flock f = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };
    return flags & PTEF_NOWAIT ? setlk(fd, &f) : intr_safe_setlkw(fd, &f);
}
static int unlock(int fd)
{
    if (fd == -1)
        return -1;
    struct flock f = {
        .l_type = F_UNLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };
    return intr_safe_setlkw(fd, &f);
}

static bool is_terminal(int fd)
{
    struct termios tos;
    return !tcgetattr(fd, &tos);
}

// allocate and return a line buffer
static char *format_line(char *status, char *name, size_t *len, int flags)
{
    size_t status_len = strlen(status);
    size_t name_len = strlen(name);

    // status, ' ', name, '\n', '\0'
    size_t line_len = status_len + name_len + 3;

    char *ptef_prefix;
    size_t ptef_prefix_len = 0;
    if (~flags & PTEF_RAWNAME) {
        ptef_prefix = getenv_defined("PTEF_PREFIX");
        if (ptef_prefix) {
            ptef_prefix_len = strlen(ptef_prefix);
            line_len += ptef_prefix_len + 1;  // incl. '/' suffix
        } else {
            line_len += 1;  // just the leading '/'
        }
    }

    char *line = malloc(line_len);
    if (line == NULL) {
        PERROR("malloc");
        return NULL;
    }

    char *part = line;
    part = memcpy_append(part, status, status_len);
    *part++ = ' ';
    if (~flags & PTEF_RAWNAME) {
        if (ptef_prefix)
            part = memcpy_append(part, ptef_prefix, ptef_prefix_len);
        *part++ = '/';
    }
    part = memcpy_append(part, name, name_len);
    *part++ = '\n';
    *part = '\0';

    *len = line_len-1;  // without '\0'
    return line;
}

#define CLGREEN "\e[1;32m"
#define CLRED   "\e[1;31m"
#define CLBLUE  "\e[1;34m"
#define CLRESET "\e[0m"
static char *default_colors[][2] = {
    { "PASS", CLGREEN "PASS" CLRESET },
    { "FAIL", CLRED "FAIL" CLRESET },
    { "RUN", CLBLUE "RUN" CLRESET " " },
    { NULL }
};

__thread char *(*ptef_status_colors)[2] = default_colors;

// don't use STDOUT_FILENO as the standard specifies numbers
#define TERMINAL_FD 1

__asm__(".symver ptef_report_v0, ptef_report@@VERS_0.7");
__attribute__((used))
int ptef_report_v0(char *status, char *testname, int flags)
{
    int orig_errno = errno;

    int ptef_results_fd_fd = -1;
    char *line = NULL;

    char *status_pretty = status;

    bool use_color;
    char *ptef_color = getenv_defined("PTEF_COLOR");
    if (ptef_color) {
        use_color = strcmp(ptef_color, "1") == 0 ? true : false;
    } else {
        // if fd 1 is terminal, look up status color
        use_color = is_terminal(TERMINAL_FD);
    }

    if (use_color) {
        for (unsigned long i = 0; ptef_status_colors[i][0] != NULL; i++) {
            if (strcmp(ptef_status_colors[i][0], status)==0) {
                status_pretty = ptef_status_colors[i][1];
                break;
            }
        }
    }

    // lock stdout / PTEF_RESULTS_FD
    if (lock(TERMINAL_FD, flags) == -1)
        goto err;
    char *ptef_results_fd = getenv_defined("PTEF_RESULTS_FD");
    if (ptef_results_fd) {
        ptef_results_fd_fd = atoi(ptef_results_fd);
        if (ptef_results_fd_fd == 0 && *ptef_results_fd != '0') {
            ERROR_FMT("atoi(%s) failed conversion", ptef_results_fd);
            goto err;
        }
        if (lock(ptef_results_fd_fd, flags) == -1)
            goto err;
    }

    // write to stdout
    size_t len;
    line = format_line(status_pretty, testname, &len, flags);
    if (!line)
        goto err;

    if (write_safe(TERMINAL_FD, line, len) == -1)
        goto err;

    // duplicate write to PTEF_RESULTS_FD
    if (ptef_results_fd) {
        if (status_pretty == status) {
            // black'n'white line already formatted
            if (write_safe(ptef_results_fd_fd, line, len) == -1)
                goto err;
        } else {
            free(line);
            line = format_line(status, testname, &len, flags);
            if (!line)
                goto err;
            if (write_safe(ptef_results_fd_fd, line, len) == -1)
                goto err;
        }
    }

    unlock(TERMINAL_FD);
    unlock(ptef_results_fd_fd);
    free(line);
    errno = orig_errno;
    return 0;

err:
    // preserve errno that got us here (to err) through unlock/free
    orig_errno = errno;
    unlock(TERMINAL_FD);
    unlock(ptef_results_fd_fd);
    free(line);
    errno = orig_errno;
    return -1;
}
