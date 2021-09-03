#include <stdlib.h>
#include <unistd.h>

#include <ptef.h>
#include <ptef_helpers.h>

static void print_help(void)
{
    fprintf(stderr,
            "usage: ptef-mklog [OPTIONS] TESTNAME\n"
            "\n"
            "  -r  truncate test log instead of rotating it\n"
            "\n"
            "Creates and opens a log storage for a given TESTNAME, relaying stdin to\n"
            "the log storage until EOF is encountered.\n");
}

#define BUF_SIZE 1024
static int relay_input(int to)
{
    char *buf;
    if ((buf = malloc(BUF_SIZE)) == NULL) {
        PERROR("malloc");
        return -1;
    }
    ssize_t bytes;
    while ((bytes = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        if (write_safe(to, buf, bytes) == -1) {
            free(buf);
            return -1;
        }
    }
    free(buf);
    return 0;
}

int main(int argc, char **argv)
{
    int flags = 0;

    int c;
    while ((c = getopt(argc, argv, "rh")) != -1) {
        switch (c) {
            case 'r':
                flags |= PTEF_NOROTATE;
                break;
            case 'h':
                print_help();
                return 0;
            case '?':
                return 1;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 1) {
        print_help();
        return 1;
    }

    int fd;
    if ((fd = ptef_mklog(argv[0], flags)) == -1)
        return 1;

    if (relay_input(fd) == -1) {
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
