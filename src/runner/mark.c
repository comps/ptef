#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
//#include <dirent.h>
//#include <libgen.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <time.h>
#include "common.h"

// alarm()/setitimer() resets on fork(), but passes through execve()
// signal()/sigaction() passes though fork(), but resets on execve()
// thus, we don't need to do any gymnastics - both will be reset during
// execute() and there's no chance of the fork()ed child triggering
// an alarm before it can do execve()

static void signal_printer(int signum)
{
    (void)signum;

    int orig_errno = errno;

    time_t epoch;
    if ((epoch = time(NULL)) == -1) {
        PERROR("time(NULL)");
        goto out;
    }

    struct tm tm_data, *tm;
    if ((tm = localtime_r(&epoch, &tm_data)) == NULL) {
        PERROR("localtime_r");
        goto out;
    }

    char timestr[25];  // 24 + '\0'
    if (strftime(timestr, sizeof(timestr), "%FT%T%z", tm) == 0) {
        ERROR("strftime() returned 0");
        goto out;
    }

    // if our signal got called while another ptef_report was running,
    // abort and don't produce a MARK
    if (ptef_report("MARK", timestr, NULL, PTEF_NOWAIT | PTEF_RAWNAME) == -1)
        if (errno != EAGAIN)
            PERROR_FMT("ptef_report(MARK, %s, PTEF_NOWAIT)", timestr);

out:
    errno = orig_errno;
}

int setup_mark(int interval, struct sigaction *sigold,
               struct itimerval *timerold)
{
    struct sigaction signew = { 0 };
    signew.sa_handler = signal_printer;
    if (sigaction(SIGALRM, &signew, sigold) == -1) {
        PERROR("sigaction(SIGALRM,..)");
        return -1;
    }

    struct itimerval timernew = { 0 };
    timernew.it_interval.tv_sec = interval;
    timernew.it_value.tv_sec = interval;
    if (setitimer(ITIMER_REAL, &timernew, timerold) == -1) {
        PERROR("setitimer(ITIMER_REAL,..)");
        sigaction(SIGALRM, sigold, NULL);
        return -1;
    }

    return 0;
}

void teardown_mark(struct sigaction *sigold, struct itimerval *timerold)
{
    setitimer(ITIMER_REAL, timerold, NULL);
    sigaction(SIGALRM, sigold, NULL);
}

