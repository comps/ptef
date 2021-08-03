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
#include <ptef_helpers.h>

#include "common.h"

// transform sep-separated string into malloc'd char**
char **str_to_array(char *input, char sep)
{
    // str len and separator cnt
    size_t len = 0, cnt = 0;
    char *tmp = input;
    while (*tmp++ != '\0') {
        len++;
        if (*tmp == sep)
            cnt++;
    }
    cnt++;  // last array member after last sep

    // allocate cnt*sizeof(char**) for the array
    // plus 1x sizeof(char**) for a terminating NULL
    // plus len*sizeof(char) for a copy of input (we'll be modifying it)
    // plus 1x sizeof(char) for a terminating '\0' of the string

    char **arr;
    if ((arr = malloc((cnt+1)*sizeof(char**) + len+1)) == NULL) {
        PERROR("malloc");
        return NULL;
    }

    tmp = ((char *)arr) + (cnt+1)*sizeof(char**);
    memcpy(tmp, input, len+1);  // incl. '\0'

    size_t i;
    for (i = 0; i+1 < cnt; i++) {  // i+1 to avoid last member
        arr[i] = tmp;
        for (; *tmp != sep; tmp++);
        *tmp++ = '\0';
    }
    arr[i++] = tmp;  // last member
    arr[i] = NULL;

    return arr;
}

// used to wrap enum exec_entry_type into an dirent-like
// structure, for convenience
struct exec_entry {
    enum exec_entry_type type;
    char name[256];  // same as struct dirent
};

static int exec_entry_sort_cmp(const void *a, const void *b)
{
    struct exec_entry **enta = (struct exec_entry **)a;
    struct exec_entry **entb = (struct exec_entry **)b;
    return strcoll((const char *)(*enta)->name, (const char *)(*entb)->name);
}

// like scandir, but customized for our use case, also without the need
// to pass argv[0] via a global var to a scandir (*filter)
// - also returns struct exec_entry, not just a name
//
// filter valid executables/dirs here, so that anything we pass to
// execute() can be treated as an error if execution fails
static bool is_exec(int parentfd, char *name)
{
    int ret;
    if ((ret = faccessat(parentfd, name, X_OK, 0)) == -1) {
        if (errno != EACCES && errno != ENOENT)
            PERROR_FMT("faccessat %s", name);
        return false;
    }
    return true;
}

static int
find_execs(struct exec_entry ***entries, char *basename, char **ignored)
{
    DIR *cwd = NULL;
    if ((cwd = opendir(".")) == NULL) {
        PERROR("opendir CWD");
        return -1;
    }

    int cwdfd = dirfd(cwd);
    struct exec_entry **ents = NULL;
    size_t entcnt = 0;

    struct dirent *dent;
    while ((dent = readdir(cwd)) != NULL) {
        // skip hidden files, '.' and '..'
        if (dent->d_name[0] == '.')
            continue;

        // skip current executable
        if (strcmp(dent->d_name, basename) == 0)
            continue;

        // skip user-ignored names
        if (ignored) {
            int i;
            for (i = 0; ignored[i] != NULL; i++) {
                if (strcmp(dent->d_name, ignored[i]) == 0)
                    break;
            }
            if (ignored[i] != NULL)
                continue;
        }

        enum exec_entry_type enttype;

        // even though GNU readdir(3) provides dent->d_type which would save us
        // one fstatat() syscall here, it is not POSIX *and* does not resolve
        // symlinks, so we have to use fstatat() anyway
        if (fstatat_type(cwdfd, dent->d_name, &enttype) == -1)
            continue;
        if (enttype == EXEC_TYPE_INVALID)
            continue;

        int subdir;
        switch (enttype) {
            case EXEC_TYPE_DIR:
                // look for our basename in the directory
                if ((subdir = openat(cwdfd, dent->d_name, O_DIRECTORY)) == -1) {
                    PERROR_FMT("openat %s", dent->d_name);
                    continue;
                }
                if (!is_exec(subdir, basename)) {
                    close(subdir);
                    continue;
                }
                close(subdir);
                break;
            case EXEC_TYPE_FILE:
                // just check for executability
                if (!is_exec(cwdfd, dent->d_name))
                    continue;
                break;
            default:
                continue;
        }

        if ((ents = realloc_safe(ents, (entcnt+1)*sizeof(*ents))) == NULL) {
            PERROR("realloc");
            goto err;
        }
        entcnt++;

        struct exec_entry *ent;
        if ((ent = malloc(sizeof(*ent))) == NULL) {
            PERROR("malloc");
            goto err;
        }
        strncpy(ent->name, dent->d_name, sizeof(ent->name));
        ent->type = enttype;

        ents[entcnt-1] = ent;
    }

    qsort(ents, entcnt, sizeof(*ents), exec_entry_sort_cmp);
    closedir(cwd);
    *entries = ents;
    return entcnt;

err:
    while (entcnt--)
        free(ents[entcnt]);
    free(ents);
    closedir(cwd);
    return -1;
}

int for_each_exec(char *basename, int jobs)
{
    struct exec_state *state;
    if ((state = create_exec_state(jobs)) == NULL) {
        PERROR("create_exec_state");
        return -1;
    }

    struct exec_entry **ents = NULL;
    char **ignored = NULL;

    char *ignore_files = getenv_defined("PTEF_IGNORE_FILES");
    if (ignore_files)
        if ((ignored = str_to_array(ignore_files, '\n')) == NULL)
            goto err;

    int cnt = find_execs(&ents, basename, ignored);
    if (cnt == -1)
        goto err;

    char *argv[2] = { NULL };
    int i;
    for (i = 0; i < cnt; i++) {
        if (execute(ents[i]->name, ents[i]->type, argv, basename, state) == -1)
            goto err;
        free(ents[i]);
    }

    free(ents);
    free(ignored);
    return destroy_exec_state(state);

err:
    for (; i < cnt; i++)
        free(ents[i]);
    free(ents);
    free(ignored);
    destroy_exec_state(state);
    return -1;
}
