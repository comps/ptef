#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int intr_safe_setlkw(int fd, struct flock *f)
{
    while (fcntl(fd, F_SETLKW, f) == -1) {
        if (errno != EINTR)
            return -1;
    }
    return 0;
}
int lock(int fd)
{
    struct flock f = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };
    return intr_safe_setlkw(fd, &f);
}
int unlock(int fd)
{
    struct flock f = {
        .l_type = F_UNLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0,
    };
    return intr_safe_setlkw(fd, &f);
}

int main(int argc, char **argv)
{
    char *usagemsg = "usage: %s fifoname\n"
                     "locks stdout, using fifoname for sync\n";

    if (argc < 2) {
        fprintf(stderr, usagemsg, argv[0]);
        return 1;
    }


    if (fcntl(STDOUT_FILENO, F_GETFD) == -1) {
        perror("fcntl");
        return 1;
    }

    // named FIFOs are f-ing insane in C:
    // sometimes they block: open(..., O_WRONLY)
    // sometimes they don't: open(..., O_RDWR)
    // sometimes they block: read(fd, ...)
    // sometimes they don't: write(fd, ...) without a reader (!!!)
    // sometimes they block: first open(..., O_WRONLY)
    // sometimes they don't: second open(..., O_WRONLY) after close(first)
    // --> just use system(), ffs

    if (lock(STDOUT_FILENO) == -1) {
        perror("setlkw");
        return 1;
    }

    char fifocmd[1024];
    snprintf(fifocmd, sizeof(fifocmd), "echo -n > %s", argv[1]);

    // notify test that we locked the fd
    if (system(fifocmd) != 0) {
        fprintf(stderr, "system(%s) failed", fifocmd);
        return 1;
    }

    // wait for the test to tell us to unlock
    if (system(fifocmd) != 0) {
        fprintf(stderr, "system(%s) failed", fifocmd);
        return 1;
    }

    unlock(STDOUT_FILENO);

    return 0;
}
