#include <stdio.h>
#include <fcntl.h>

int main()
{
    if (open("/dev/null", O_RDONLY) < 0) {
        perror("open");
        return 1;
    }
    return 0;
}
