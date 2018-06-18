#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int main()
{
    if (lseek(0, 0, SEEK_CUR) != -1 && errno != ESPIPE) {
        perror("lseek");
        return 1;
    }
    return 0;
}
