#include <stdlib.h>
#include <string.h>
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
            "  -c MAP   use custom color mapping for statuses\n"
            "  -x MAP   use a non-standard exit-code-to-status mapping\n"
            "  -r       set PTEF_RUN and export it\n"
            "  -s       set PTEF_SILENT and export it\n"
            "  -v       set PTEF_NOLOGS and export it (\"verbose\")\n"
            "  -d       set PTEF_SHELL to 1 and export it (\"debug\")\n"
            "  -m       don't merge arguments of subrunners (always pass 1 arg)\n"
            "\n"
            "Executes the PTEF runner logic from CWD, executing executables and traversing\n"
            "subdirectories.\n"
            "If TEST is specified, runs only that test, without searching for executables.\n"
            "\n"
            "The -i option can be specified multiple times.\n"
            "\n"
            "Custom color MAP is a \"STATUS NEWSTATUS\" pair, rewriting STATUS to NEWSTATUS\n"
            "(which can contain color escape sequences or additional trailing spaces for\n"
            "alignment with longer statuses). The -c option can be specified multiple times.\n"
            "For example: -c $'FAIL \\e[1;41mFAIL\\e[0m ' -c $'WAIVE \\e[1;33mWAIVE\\e[0m'\n"
            "\n"
            "Custom exit code MAP is a space-separated \"NUMBER:STATUS\" set of pairs,\n"
            "with the last separated element specifying a default fallback STATUS.\n"
            "For example: -x '0:PASS 2:WARN 3:ERROR FAIL'\n");
}

int parse_exit_code_map(char *optarg, char **map, char **def)
{
    int nr;
    char *trailer, *pair, *status, *saveptr;

    if ((trailer = strrchr(optarg, ' ')) == NULL) {
        ERROR_FMT("need at least two map elements: %s\n", optarg);
        return -1;
    }
    *trailer++ = '\0';

    pair = strtok_r(optarg, " ", &saveptr);
    if (!pair) {
        ERROR_FMT("first code:status pair empty: %s\n", optarg);
        return -1;
    }

    while (pair != NULL) {
        if ((status = strchr(pair, ':')) == NULL) {
            ERROR_FMT("missing colon separator: %s\n", pair);
            return -1;
        }
        *status++ = '\0';
        if (*status == '\0') {
            ERROR_FMT("empty status string: %s\n", pair);
            return -1;
        }
        if ((nr = strtoi_safe(pair)) == -1 || nr > 255) {
            if (!errno && nr > 255)
                errno = ERANGE;
            PERROR_FMT("invalid exit code nr: %s", pair);
            return -1;
        }
        map[nr] = status;
        pair = strtok_r(NULL, " ", &saveptr);
    }

    *def = trailer;

    return 0;
}

int main(int argc, char **argv)
{
    char *argv0 = NULL;
    int jobs = 1;
    int flags = 0;

    char **ignored = NULL;
    int ignored_cnt = 0;

    char *(*colormap)[2] = NULL;
    int colorcnt = 0;
    char *delim;

    char **code_map = NULL;
    char *code_default;

    int c;
    while ((c = getopt(argc, argv, "a:A:j:i:c:x:rsvdmh")) != -1) {
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
                jobs = strtoi_safe(optarg);
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
            case 'c':
                delim = strchr(optarg, ' ');
                if (!delim) {
                    ERROR_FMT("MAP has no space: %s\n", optarg);
                    goto err;
                }
                colorcnt++;
                colormap = realloc_safe(colormap, colorcnt*sizeof(char*[1][2]));
                *delim = '\0';
                colormap[colorcnt-1][0] = optarg;
                colormap[colorcnt-1][1] = delim+1;  // worst case, it is '\0'
                break;
            case 'x':
                free(code_map);
                if ((code_map = malloc(256*sizeof(char*))) == NULL) {
                    PERROR("malloc");
                    goto err;
                }
                memset(code_map, 0, 256*sizeof(char*));
                if (parse_exit_code_map(optarg, code_map, &code_default) == -1)
                    goto err;
                ptef_exit_statuses = code_map;
                ptef_exit_statuses_default = code_default;
                break;
            case 'r':
                if (setenv("PTEF_RUN", "1", 1) == -1) {
                    PERROR("setenv");
                    goto err;
                }
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
            case 'd':
                if (setenv("PTEF_SHELL", "1", 1) == -1) {
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

    if (colormap) {
        colorcnt++;
        colormap = realloc_safe(colormap, colorcnt*sizeof(char*[1][2]));
        colormap[colorcnt-1][0] = colormap[colorcnt-1][1] = NULL;
        ptef_status_colors = colormap;
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
    free(code_map);
    return !!ret;

err:
    free(ignored);
    free(code_map);
    return 1;
}
