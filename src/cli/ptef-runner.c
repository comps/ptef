#include <stdlib.h>
#include <unistd.h>

#include <ptef.h>
#include <ptef_helpers.h>

static void print_help(void)
{
    fprintf(stderr,
            "usage: ptef-runner [OPTIONS] [TEST]...\n"
            "\n"
            "  -a BASE  runner basename, overriding autodetection from basename(argv[0])\n"
            "  -A BASE  set and export PTEF_BASENAME, overriding even -a\n"
            "  -j NR    number of parallel jobs (tests)\n"
            "  -i IGN   ignore a file/dir named IGN when searching for executables\n"
            "  -s       set PTEF_SILENT and export it\n"
            "  -v       set PTEF_NOLOGS and export it\n"
            "  -r       set PTEF_RUN and export it\n"
            "  -m       don't merge arguments of subrunners (always pass 1 arg)\n"
            "\n"
            "Executes the PTEF runner logic from CWD, executing executables and traversing\n"
            "subdirectories.\n"
            "If TEST is specified, runs only that test, without searching for executables.\n"
            "\n"
            "The -i option can be specified multiple times.\n");
}

int main(int argc, char **argv)
{
    char *argv0 = NULL;
    int jobs = 1;
    char **ignored = NULL;
    int ignored_cnt = 0;
    int flags = 0;

    int c;
    while ((c = getopt(argc, argv, "a:A:j:i:svrmh")) != -1) {
        switch (c) {
            case 'a':
                argv0 = optarg;
                break;
            case 'A':
                if (setenv("PTEF_BASENAME", optarg, 1) == -1) {
                    PERROR("setenv");
                    goto err;
                }
                argv0 = optarg;  // avoid unnecessary basename() later
                break;
            case 'j':
                jobs = atoi(optarg);
                if (jobs < 1) {
                    ERROR_FMT("invalid job cnt: %s\n", optarg);
                    goto err;
                }
                break;
            case 'i':
                ignored_cnt++;
                ignored = realloc_safe(ignored, ignored_cnt*sizeof(char*));
                ignored[ignored_cnt-1] = optarg;
                break;
            case 's':
                if (setenv("PTEF_SILENT", "1", 1) == -1) {
                    PERROR("setenv");
                    goto err;
                }
                break;
            case 'v':
                if (setenv("PTEF_NOLOGS", "1", 1) == -1) {
                    PERROR("setenv");
                    goto err;
                }
                break;
            case 'r':
                if (setenv("PTEF_RUN", "1", 1) == -1) {
                    PERROR("setenv");
                    goto err;
                }
                break;
            case 'm':
                flags |= PTEF_NOMERGE;
                break;
            case 'h':
                print_help();
                return 0;
            case '?':
                return 1;
        }
    }

    if (ignored) {
        ignored_cnt++;
        ignored = realloc_safe(ignored, ignored_cnt*sizeof(char*));
        ignored[ignored_cnt-1] = NULL;
    }

    // shift argv[0] before the remaining arguments,
    // adjust argc/argv to appear as if there was no getopt()
    argv[optind-1] = argv[0];
    argc -= optind-1;
    argv += optind-1;

    if (argv0)
        argv[0] = argv0;

    int ret = ptef_runner(argc, argv, jobs, ignored, flags);
    free(ignored);
    return !!ret;

err:
    free(ignored);
    return 1;
}
