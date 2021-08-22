#ifndef _PTEF_H
#define _PTEF_H

// runner flags
#define PTEF_NOMERGE    (1 << 0)
#define PTEF_NORUN      (1 << 1)

// report flags
#define PTEF_NOLOCK     (1 << 0)
#define PTEF_NOWAIT     (1 << 1)
#define PTEF_RAWNAME    (1 << 2)

int ptef_runner(int argc, char **argv, char *default_basename, int jobs,
                int mark_interval, int flags);
int ptef_report(char *status, char *testname, int flags);
int ptef_mklog(char *testname, int flags);

#endif
