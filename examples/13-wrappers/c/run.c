#include <stdio.h>

#include <ptef.h>

int main(int argc, char **argv)
{
    printf("custom setup logic\n");

    int rc = ptef_runner(argc, argv, 1, NULL, 0);

    printf("custom cleanup logic\n");

    return !!rc;
}
