#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <errno.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <ptef.h>

static void print_help(void)
{
    fprintf(stderr,
            "usage: ptef-report [OPTIONS] STATUS TESTNAME\n"
            "\n"
            "  -N  don't lock when writing a result\n"
            "  -n  don't wait for lock, return 2 if somebody else holds it\n"
            "\n"
            "Reports STATUS for a given TESTNAME, prepending PTEF_PREFIX\n"
            "to the TESTNAME, copying the report to PTEF_RESULTS_FD if defined.\n"
            "Outputs color if stdout is connected to a terminal.\n");
}

int main(int argc, char **argv)
{
    int flags = 0;

    int c;
    while ((c = getopt(argc, argv, "Nnh")) != -1) {
        switch (c) {
            case 'N':
                flags |= PTEF_NOLOCK;
                break;
            case 'n':
                flags |= PTEF_NOWAIT;
                break;
            case 'h':
                print_help();
                return 0;
            case '?':
                return 1;
        }
    }
    argv += optind;

    int ret = ptef_report(argv[0], argv[1], flags);
    if (ret == -1 &&
            flags & PTEF_NOWAIT && ~flags & PTEF_NOLOCK && errno == EAGAIN)
        return 2;
    else
        return !!ret;
}
