#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>
#include <errno.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>

// free the block if realloc fails
void *realloc_safe(void *ptr, size_t size)
{
    void *new = realloc(ptr, size);
    if (new == NULL)
        free(ptr);
    return new;
}

static bool is_exec(int parentfd, char *name)
{
    int ret;
    if ((ret = faccessat(parentfd, name, X_OK, 0)) == -1) {
        if (errno != EACCES && errno != ENOENT)
            perror("faccessat");
        return false;
    }
    return true;
}

int alphasort_for_qsort(const void *a, const void *b)
{
    return alphasort((const struct dirent **)a, (const struct dirent **)b);
}

// argument-less run
//
// like scandir, but customized for our use case, also without the need
// to pass argv[0] via a global var to a scandir (*filter)
// - also returns the whole dirent, not just a name
static int find_execs(struct dirent ***entlist, char *basename)
{
    DIR *cwd = NULL;
    if ((cwd = opendir(".")) == NULL) {
        perror("opendir");
        return -1;
    }

    int cwdfd = dirfd(cwd);
    int subdir;
    struct dirent *ent, **ents = NULL;
    size_t entcnt = 0;
    while ((ent = readdir(cwd)) != NULL) {
        // skip hidden files, '.' and '..'
        if (ent->d_name[0] == '.')
            continue;

        // skip current executable
        if (strcmp(ent->d_name, basename) == 0)
            continue;

        switch (ent->d_type) {
            case DT_DIR:
                // look for basename in the directory
                if ((subdir = openat(cwdfd, ent->d_name, O_DIRECTORY)) == -1) {
                    perror("openat");
                    continue;
                }
                if (!is_exec(subdir, basename)) {
                    close(subdir);
                    continue;
                }
                close(subdir);
                break;
            case DT_REG:
                // just check for executability
                if (!is_exec(cwdfd, ent->d_name))
                    continue;
                break;
            default:
                continue;
        }

        if ((ents = realloc_safe(ents, (entcnt+1)*sizeof(*ent))) == NULL) {
            perror("realloc");
            goto err;
        }
        entcnt++;

        if ((ents[entcnt-1] = malloc(sizeof(*ent))) == NULL) {
            perror("malloc");
            goto err;
        }
        memcpy(ents[entcnt-1], ent, sizeof(*ent));
    }

    qsort(ents, entcnt, sizeof(ent), alphasort_for_qsort);
    closedir(cwd);
    *entlist = ents;
    return entcnt;

err:
    while (entcnt--)
        free(ents[entcnt]);
    free(ents);
    closedir(cwd);
    return -1;
}

// temporary, for debug
// TODO execution logic - it needs a big 'if' between directory ents and CWD ents
//      as the directory ones will need chdir, etc.
// TODO environment variable export (TEF_LOGS, etc.) before any forking/execution
// TODO some child function for the execve
static void for_each_exec(char *basename)
{
    struct dirent **ents;
    int cnt;

    cnt = find_execs(&ents, basename);
    if (cnt == -1)
        return;

    for (int i = 0; i < cnt; i++) {
        printf("%s\n", ents[i]->d_name);
        free(ents[i]);
    }
    free(ents);
}




static char *sane_arg(char *a)
{
    char *new = a;
    while (*new == '/')
        new++;
    // empty is invalid
    if (new[0] == '\0')
        return NULL;
    // '.' or '..' are invalid
    // probably more efficient than 4 strcmp's
    if (new[0] == '.') {
        if (new[1] == '\0' || new[1] == '/')
            return NULL;
        if (new[1] == '.') {
            if (new[2] == '\0' || new[2] == '/')
                return NULL;
        }
    }
    return new;
}


//char *sanitize(const char *input)
//{
//    int level = 0;
//    char *pos, *lastpos = input;
//    char *result;
//    size_t maxlen = strlen(input);
//    ptrdiff_t len;
//
//    if ((result = malloc(maxlen+1)) == NULL)
//        return NULL;
//
//    //                                                  skip the '/'
//    for (; (pos = strchr(lastpos, '/')) != NULL; lastpos = pos+1) {
//        len = pos - lastpos;
//        // empty path element: //
//        if (len == 0)
//            continue;
//        // single dot: /./
//        if (len == 1 && e[0] == '.')
//            continue;
//        // two dots: /../
//        if (len == 2 && e[0] == '.' && e[1] == '.') {
//            level--;
//            continue;
//    }
//
//    //for (pos = 0; .
//}



// argument-given run
// ...
// this function modifies / destroys the passed argv
static void for_each_arg(int argc, char **argv)
{
    // malloc array equal to argc+1 (for NULL)
    // then go through argv, append modified ptrs to malloc'd array

    char **merged;
    if ((merged = malloc((argc+2)*sizeof(argv))) == NULL) {
        perror("malloc");
        return;
    }
    // leave 0 for execve argv[0] being executable name
    int merged_idx = 1;
    // NULL-terminate, will be moved on each arg addition
    merged[merged_idx] = NULL;

    char *prefix = NULL;

    for (int i = 0; i < argc; i++) {
        char *sane = sane_arg(argv[i]);

        char *slash;
        if (sane)
            slash = strchr(sane, '/');
        else
            slash = NULL;

        if (!sane || !slash || strcmp(sane, prefix) != 0) {
            // finish assembling merged[], pass it for execution
            // ...
            prefix = NULL;  // reset
            merged_idx = 1;
            merged[merged_idx] = NULL;
        } else {
            // starting a new arglist to be merged
            if (!prefix) {
                ptrdiff_t slash_off = slash-sane;
                prefix = strndup(sane, slash_off);
                merged[merged_idx] = sane+slash_off+1;
                merged[merged_idx+1] = NULL;
            } else {
                .
                // compare against previous
                // add to merged
            }
        }

merge:
        ...;
        continue;
standalone:
        ...;
        continue;

    }

err:
    if (merged) {
        ...
    }
}


int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;
    for_each_exec(argv[1]);
    return 0;
}