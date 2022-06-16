#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

int main()
{
    DIR *d = opendir("/proc/self/fd");
    if (d == NULL) {
        perror("opendir");
        return 1;
    }
    int dfd = dirfd(d);

    struct dirent *dent;
    while ((dent = readdir(d)) != NULL) {
        if (dent->d_name[0] == '.')
            continue;
        if (atoi(dent->d_name) == dfd)
            continue;
        printf("%s\n", dent->d_name);
        char linkpath[1024];
        ssize_t len = readlinkat(dfd, dent->d_name, linkpath, sizeof(linkpath));
        if (len != -1) {
            if (len == sizeof(linkpath))
                len--;
            linkpath[len] = '\0';
            fprintf(stderr, "%s: %s\n", dent->d_name, linkpath);
        }
    }

    closedir(d);

    return 0;
}
