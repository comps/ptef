#include <sys/types.h>

#include <ptef.h>
#include <ptef_helpers.h>

// for passing file type between the directory scanner and executing
// function when running without args (when scanning a directory),
// to prevent an extra fstatat()
// - when running with args (no directory is scanned), UNKNOWN is used
//   and the executing function will do the fstatat() itself
enum exec_entry_type {
    EXEC_TYPE_UNKNOWN,
    EXEC_TYPE_INVALID,
    EXEC_TYPE_FILE,
    EXEC_TYPE_DIR,
};

int fstatat_type(int dirfd, char *pathname, enum exec_entry_type *type);

int dup2_safe(int oldfd, int newfd);

// for parallel execution - we need to keep track of persistent
// pid-to-testname mapping, so we can ptef_report the correct name
// when a child finishes (might be many iterations later)
struct pid_to_name {
    pid_t pid;
    char *name;
};
// used across repeated execute() calls to track state
struct exec_state {
    int max_jobs;
    int running_jobs;
    struct pid_to_name map[];
};

struct exec_state *create_exec_state(int jobs);
int destroy_exec_state(struct exec_state *state);
int execute(char *exe, enum exec_entry_type typehint, char **argv,
            char *basename, struct exec_state *state);

int for_each_arg(int argc, char **argv, char *basename, int jobs);

int for_each_merged_arg(int argc, char **argv, char *basename, int jobs);

int for_each_exec(char *basename, int jobs, char **ignored);
