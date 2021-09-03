#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ptef.h>
#include <ptef_helpers.h>

static void print_help(void)
{
    fprintf(stderr,
            "usage: ptef-report [OPTIONS] STATUS TESTNAME\n"
            "\n"
            "  -N      don't lock when writing a result\n"
            "  -n      don't wait for lock, return 2 if somebody else holds it\n"
            "  -r      raw testname, report it as-is, without prefix\n"
            "  -c MAP  use custom color mapping for statuses\n"
            "\n"
            "Reports STATUS for a given TESTNAME, prepending PTEF_PREFIX to the TESTNAME,\n"
            "copying the report to PTEF_RESULTS_FD if defined.\n"
            "Outputs color if stdout is connected to a terminal.\n"
            "\n"
            "Custom color MAP is a \"STATUS NEWSTATUS\" pair, rewriting STATUS to NEWSTATUS\n"
            "(which can contain color escape sequences or additional trailing spaces for\n"
            "alignment with longer statuses). The -c option can be specified multiple times.\n"
            "For example: -c $'FAIL \\e[1;41mFAIL\\e[0m ' -c $'WAIVE \\e[1;33mWAIVE\\e[0m'\n");
}

int main(int argc, char **argv)
{
    int flags = 0;

    char *(*colormap)[2] = NULL;
    int colorcnt = 0;
    char *delim;

    int c;
    while ((c = getopt(argc, argv, "Nnrc:h")) != -1) {
        switch (c) {
            case 'N':
                flags |= PTEF_NOLOCK;
                break;
            case 'n':
                flags |= PTEF_NOWAIT;
                break;
            case 'r':
                flags |= PTEF_RAWNAME;
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
            case 'h':
                print_help();
                free(colormap);
                return 0;
            case '?':
                goto err;
        }
    }

    if (colormap) {
        colorcnt++;
        colormap = realloc_safe(colormap, colorcnt*sizeof(char*[1][2]));
        colormap[colorcnt-1][0] = colormap[colorcnt-1][1] = NULL;
    }

    argc -= optind;
    argv += optind;

    if (argc != 2) {
        print_help();
        goto err;
    }

    if (colormap)
        ptef_status_colors = colormap;
    int ret = ptef_report(argv[0], argv[1], flags);
    free(colormap);
    if (ret == -1 &&
            flags & PTEF_NOWAIT && ~flags & PTEF_NOLOCK && errno == EAGAIN)
        return 2;
    else
        return !!ret;

err:
    free(colormap);
    return 1;
}
