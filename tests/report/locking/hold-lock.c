#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

void handler(int sig)
{
    (void)sig;
}

int main()
{
    if (fcntl(STDOUT_FILENO, F_GETFD) == -1) {
        perror("error: fcntl");
        return 1;
    }

    struct sigaction act = {
        .sa_handler = handler,
    };
    if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("error: sigaction(SIGINT)");
        return 1;
    }

    struct flock f = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };
    while (fcntl(STDOUT_FILENO, F_SETLKW, &f) == -1) {
        if (errno != EINTR) {
            perror("error: fcntl(F_WRLCK)");
            return 1;
        }
    }
    fprintf(stderr, "locked\n");

    struct timespec ts = {
        .tv_sec = 60,
        .tv_nsec = 0,
    };
    if (nanosleep(&ts, NULL) == 0) {
        fprintf(stderr, "error: waited too long\n");
        return 1;
    }

    f.l_type = F_UNLCK;
    if (fcntl(STDOUT_FILENO, F_SETLKW, &f) == -1) {
        perror("error: fcntl(F_UNLCK)");
        return 1;
    }
    fprintf(stderr, "unlocked\n");

    return 0;
}
