#ifndef _PTEF_H
#define _PTEF_H

// runner flags
#define PTEF_NOMERGE    (1 << 0)

// report flags
#define PTEF_NOLOCK     (1 << 0)
#define PTEF_NOWAIT     (1 << 1)
#define PTEF_RAWNAME    (1 << 2)

// mklog flags
#define PTEF_NOROTATE   (1 << 0)

extern __thread char *(*ptef_status_colors)[2];

int ptef_runner(int argc, char **argv, int jobs, char **ignored, int flags);
int ptef_report(char *status, char *testname, int flags);
int ptef_mklog(char *testname, int flags);

#endif
