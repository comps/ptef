#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "common.h"

int fstatat_type(int dirfd, char *pathname, enum exec_entry_type *type)
{
    int ret;
    struct stat statbuf;
    if ((ret = fstatat(dirfd, pathname, &statbuf, 0)) == -1)
        return -1;
    switch (statbuf.st_mode & S_IFMT) {
        case S_IFREG:
            *type = EXEC_TYPE_FILE;
            break;
        case S_IFDIR:
            *type = EXEC_TYPE_DIR;
            break;
        default:
            *type = EXEC_TYPE_INVALID;
            break;
    }
    return ret;
}

int dup2_safe(int oldfd, int newfd)
{
    int ret;
    while ((ret = dup2(oldfd, newfd)) == -1) {
        if (errno != EINTR)
            return -1;
    }
    return ret;
}
