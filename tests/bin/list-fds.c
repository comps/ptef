#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    char procpath[64], linkpath[1024];
    long max = sysconf(_SC_OPEN_MAX);
    for (long i = 0; i < max; i++) {
        int ret = fcntl(i, F_GETFD);
        if (ret == -1) {
            if (errno == EBADF)
                continue;
            perror("fcntl");
            return 1;
        }
        printf("%ld\n", i);
        snprintf(procpath, sizeof(procpath), "/proc/self/fd/%ld", i);
        ssize_t len = readlink(procpath, linkpath, sizeof(linkpath));
        if (len != -1) {
            if (len == sizeof(linkpath))
                len--;
            linkpath[len] = '\0';
            fprintf(stderr, "%ld: %s\n", i, linkpath);
        }
    }
    return 0;
}
