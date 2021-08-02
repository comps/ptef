#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>

// ensure we define the same function prototypes
#include <ptef.h>

#define ERR(eno) \
    fprintf(stderr, "errno test nonzero: %d (%s)\n", eno, strerror(eno))

int ptef_runner(int argc, char **argv, char *basename, int jobs, int nomerge) {
    int (*orig)(int, char **, char *, int, int);
    orig = dlsym(RTLD_NEXT, "ptef_runner");
    int rc = (*orig)(argc, argv, basename, jobs, nomerge);
    if (errno) {
        ERR(errno);
        return -1;
    }
    return rc;
}
int ptef_report(char *status, char *testname) {
    int (*orig)(char *, char *);
    orig = dlsym(RTLD_NEXT, "ptef_report");
    int rc = (*orig)(status, testname);
    if (errno) {
        ERR(errno);
        return -1;
    }
    return rc;
}
int ptef_mklog(char *testname) {
    int (*orig)(char *);
    orig = dlsym(RTLD_NEXT, "ptef_mklog");
    int rc = (*orig)(testname);
    if (errno) {
        ERR(errno);
        return -1;
    }
    return rc;
}
