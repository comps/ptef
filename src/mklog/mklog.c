#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ptef.h>
#include <ptef_helpers.h>

static int openat_safe(int dirfd, const char *pathname, int flags, mode_t mode)
{
    int ret;
    while ((ret = openat(dirfd, pathname, flags, mode)) == -1) {
        if (errno != EINTR)
            return -1;
    }
    return ret;
}

// rotate old logs, create new one, return its fd
static int open_log(int dirfd, char *testname)
{
    // do log rotation (unlinkat(), renameat()) based on this dir fd
    // create the final new logfile via open() O_CREAT | O_WRONLY
    // return it as open fd
    char tmpl_suffix[] = ".log.X";

    size_t testname_len = strlen(testname);

    char *from = NULL, *to = NULL;

    if ((from = malloc(testname_len+sizeof(tmpl_suffix))) == NULL) {
        PERROR("malloc");
        goto err;
    }

    char *pos = from;
    pos = memcpy_append(pos, testname, testname_len);
    memcpy_append(pos, tmpl_suffix, sizeof(tmpl_suffix));

    if ((to = strdup(from)) == NULL) {
        PERROR("strdup");
        goto err;
    }

    // offsets of the single-digit logrotated number: testname + ".log." + 1
    char *fromnr = from+testname_len+5;
    char *tonr = to+testname_len+5;

    // unlink (remove) the oldest log if it exists
    *fromnr = '9';
    if (unlinkat(dirfd, from, 0) == -1 && errno != ENOENT) {
        PERROR_FMT("unlinkat %s", from);
        goto err;
    }

    // rotate everything one iteration
    // unfortunately this is the easiest way without expensive printf
#define MOVE(fromnum,tonum) \
    do { \
        *fromnr = fromnum; \
        *tonr = tonum; \
        if (renameat(dirfd, from, dirfd, to) == -1 && errno != ENOENT) { \
            PERROR_FMT("rename %s to %s", from, to); \
            goto err; \
        } \
    } while (0)
    MOVE('8','9');
    MOVE('7','8');
    MOVE('6','7');
    MOVE('5','6');
    MOVE('4','5');
    MOVE('3','4');
    MOVE('2','3');
    MOVE('1','2');
#undef MOVE

    // manually move ".log" to ".log.1" here
    // re-use the already-allocated "to" by trimming ".log.2" with ".log"
    // "from" already has ".log.1" here
    to[testname_len+4] = '\0';  // testname + ".log" + 1
    if (renameat(dirfd, to, dirfd, from) == -1 && errno != ENOENT) {
        PERROR_FMT("rename %s to %s", to, from);
        goto err;
    }

    // create and open a new log
    int fd = openat_safe(dirfd, to, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {
        PERROR_FMT("openat(..,%s,O_CREAT|O_WRONLY)", to);
        goto err;
    }

    free(from);
    free(to);
    return fd;
err:
    free(from);
    free(to);
    return -1;
}

// create new log, truncating an old one, return its fd
static int open_truncated_log(int dirfd, char *testname)
{
    char suffix[] = ".log";

    size_t testname_len = strlen(testname);

    char *logfile;
    if ((logfile = malloc(testname_len+sizeof(suffix))) == NULL) {
        PERROR("malloc");
        goto err;
    }

    char *pos = logfile;
    pos = memcpy_append(pos, testname, testname_len);
    memcpy_append(pos, suffix, sizeof(suffix));

    // create and open a new log
    int fd = openat_safe(dirfd, logfile, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        PERROR_FMT("openat(..,%s,O_CREAT|O_WRONLY|O_TRUNC)", logfile);
        goto err;
    }

    free(logfile);
    return fd;
err:
    free(logfile);
    return -1;
}

// recursive mkdir start at srcfd (O_DIRECTORY)
static int mkpath(int srcfd, char *path)
{
    char *buff;
    if ((buff = malloc(strlen(path)+1)) == NULL) {
        PERROR("malloc");
        return -1;
    }

    char *pos = buff;
    char *start = path, *end = path;
    while ((end = strchr(start, '/')) != NULL) {
        if (end == start) {
            start++;
            continue;
        }
        pos = memcpy_append(pos, start, end-start+1);
        *pos = '\0';
        if (mkdirat(srcfd, buff, 0755) == -1 && errno != EEXIST) {
            PERROR_FMT("mkdirat %s", buff);
            goto err;
        }
        start = end+1;
    }
    if (*start != '\0') {
        strcpy(pos, start);
        if (mkdirat(srcfd, buff, 0755) == -1 && errno != EEXIST) {
            PERROR_FMT("mkdirat %s", buff);
            goto err;
        }
    }

    free(buff);
    return 0;
err:
    free(buff);
    return -1;
}

static int open_create_dir(char *name)
{
    int fd = open(name, O_DIRECTORY);
    if (fd == -1) {
        if (errno != ENOENT) {
            PERROR_FMT("open %s", name);
            return -1;
        }
        // if we lost a race for creating the directory (EEXIST),
        // just continue on, no big deal, the dir is created now
        if (mkdir(name, 0755) == -1 && errno != EEXIST) {
            PERROR_FMT("mkdir %s", name);
            return -1;
        }
        if ((fd = open(name, O_DIRECTORY)) == -1) {
            PERROR_FMT("open %s", name);
            return -1;
        }
    }
    return fd;
}

// open/create PTEF_LOGS, create path inside it, open final leaf dir
static int open_ptef_logs(char *ptef_logs, char *ptef_prefix)
{
    char *combined = NULL;

    // try opening it, create if necessary
    int logsfd = open_create_dir(ptef_logs);
    if (logsfd == -1)
        return -1;

    if (!ptef_prefix)
        ptef_prefix = "";

    // 'mkdir -p' any directories inside PTEF_LOGS
    if (mkpath(logsfd, ptef_prefix) == -1)
        goto err;

    size_t ptef_logs_len = strlen(ptef_logs);
    size_t ptef_prefix_len = strlen(ptef_prefix);

    // concat PTEF_LOGS + '/' + PTEF_PREFIX + '\0'
    combined = malloc(ptef_logs_len+ptef_prefix_len+2);
    if (combined == NULL) {
        PERROR("malloc");
        goto err;
    }

    char *pos = combined;
    pos = memcpy_append(pos, ptef_logs, ptef_logs_len);
    *pos++ = '/';
    pos = memcpy_append(pos, ptef_prefix, ptef_prefix_len);
    *pos = '\0';

    // open the concatenated result
    close_safe(logsfd);
    logsfd = open(combined, O_DIRECTORY);
    if (logsfd == -1) {
        PERROR_FMT("open %s", combined);
        goto err;
    }

    free(combined);
    return logsfd;
err:
    free(combined);
    if (logsfd != -1)
        close_safe(logsfd);
    return -1;
}

// create and open a log file for testname, return its fd
int ptef_mklog(char *testname, int flags)
{
    int orig_errno = errno;

    int logsfd;

    char *ptef_logs = getenv_defined("PTEF_LOGS");
    if (ptef_logs)
        logsfd = open_ptef_logs(ptef_logs, getenv_defined("PTEF_PREFIX"));
    else
        logsfd = open_create_dir("logs");

    if (logsfd == -1)
        return -1;

    int fd;
    if (flags & PTEF_NOROTATE)
        fd = open_truncated_log(logsfd, testname);
    else
        fd = open_log(logsfd, testname);

    close_safe(logsfd);

    if (fd != -1)
        errno = orig_errno;

    return fd;
}
