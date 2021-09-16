#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"

// allocate a copy of str with leading/trailing slashes trimmed
static char *strdup_trim(char *str)
{
    while (*str == '/')
        str++;
    size_t len = strlen(str);
    while (len > 0 && str[len-1] == '/')
        len--;
    return strndup(str, len);
}

static bool sane_arg(char *a)
{
    if (*a == '\0')
        return false;
    // '.' or '..' are invalid
    // probably more efficient than 4 strcmp's
    if (a[0] == '.') {
        if (a[1] == '\0' || a[1] == '/')
            return false;
        if (a[1] == '.') {
            if (a[2] == '\0' || a[2] == '/')
                return false;
        }
    }
    return true;
}

struct splitarg {
    char *prefix;
    char *rest;
};

static struct splitarg *sanitize_args(int argc, char **argv)
{
    int i = 1;
    struct splitarg *new;
    if ((new = malloc(argc*sizeof(struct splitarg))) == NULL) {
        PERROR("malloc");
        goto err;
    }
    // leave new[0] uninitialized so indices match argv[]
    for (; i < argc; i++) {
        char *arg = strdup_trim(argv[i]);
        if (arg == NULL) {
            PERROR("strndup");
            goto err;
        }
        if (!sane_arg(arg)) {
            free(arg);
            ERROR_FMT("arg failed sanity check: %s\n", argv[i]);
            errno = EINVAL;
            goto err;
        }
        new[i].prefix = arg;
        char *slash = strchr(arg, '/');
        if (slash) {
            *slash = '\0';
            new[i].rest = slash+1;  // skip prefix + '/'
        } else {
            new[i].rest = NULL;
        }
    }
    return new;
err:
    for (; i > 1; i--)
        free(new[i-1].prefix);  // 'rest' shares the same allocation
    free(new);
    return NULL;
}

int for_each_arg(int argc, char **argv, char *basename, int jobs)
{
    struct exec_state *state;
    if ((state = create_exec_state(jobs)) == NULL) {
        PERROR("create_exec_state");
        return -1;
    }

    // [0] is execute-reserved for argv[0],
    // [1] is the one non-merged arg
    // [2] is NULL terminator
    char *arg[3] = { NULL };

    struct splitarg *sargs = sanitize_args(argc, argv);
    if (!sargs)
        goto err;

    for (int i = 1; i < argc; i++) {
        char *prefix = sargs[i].prefix;
        char *rest = sargs[i].rest;

        // if the 'right' side after slash doesn't exist (because the slash
        // doesn't exist), just pass NULL, indicating a run without args
        arg[1] = rest ? rest : NULL;

        if (execute(prefix, EXEC_TYPE_UNKNOWN, arg, basename, state) == -1)
            goto err;
    }

    for (int i = 1; i < argc; i++)
        free(sargs[i].prefix);
    free(sargs);
    return destroy_exec_state(state);

err:
    if (sargs) {
        for (int i = 1; i < argc; i++)
            free(sargs[i].prefix);
        free(sargs);
    }
    destroy_exec_state(state);
    return -1;
}

int for_each_merged_arg(int argc, char **argv, char *basename, int jobs)
{
    struct exec_state *state;
    if ((state = create_exec_state(jobs)) == NULL) {
        PERROR("create_exec_state");
        return -1;
    }

    char **merged = NULL;

    struct splitarg *sargs = sanitize_args(argc, argv);
    if (!sargs)
        goto err;

    // array for storing chunks of argv[] that share the same prefix
    // (+1 is for the terminating NULL ptr, execute()'s argv[0] is in argc)
    if ((merged = malloc((argc+1)*sizeof(*argv))) == NULL) {
        PERROR("malloc");
        goto err;
    }
    int merged_idx = 1;
    merged[merged_idx] = NULL;  // terminate

    char *merge_prefix = NULL;

    for (int i = 1; i < argc; i++) {
        char *prefix = sargs[i].prefix;
        char *rest = sargs[i].rest;

        // if a merge is ongoing and the prefix of the current arg is different
        // from the one being merged, finish the merge
        if (merge_prefix && (!rest || strcmp(prefix, merge_prefix) != 0)) {
            merged[merged_idx] = NULL;  // terminate
            if (execute(merge_prefix, EXEC_TYPE_UNKNOWN, merged, basename,
                        state) == -1)
                goto err;
            merge_prefix = NULL;
            merged_idx = 1;
            merged[merged_idx] = NULL;  // re-terminate at [1]
        }

        // if standalone (no slash), just execute without a merge
        if (!rest) {
            // 'merged' here provides an allocated [0] required by execute()
            // and a NULL terminator on [1], indicating no arguments
            if (execute(prefix, EXEC_TYPE_UNKNOWN, merged, basename,
                        state) == -1)
                goto err;
            continue;
        }

        // start a new merge
        if (!merge_prefix)
            merge_prefix = prefix;

        // add the current arg to the ongoing merge
        merged[merged_idx++] = rest;
    }

    // finish any merge-in-progress
    if (merge_prefix) {
        merged[merged_idx] = NULL;  // terminate
        if (execute(merge_prefix, EXEC_TYPE_UNKNOWN, merged, basename,
                    state) == -1)
            goto err;
    }

    free(merged);
    for (int i = 1; i < argc; i++)
        free(sargs[i].prefix);
    free(sargs);
    return destroy_exec_state(state);

err:
    free(merged);
    if (sargs) {
        for (int i = 1; i < argc; i++)
            free(sargs[i].prefix);
        free(sargs);
    }
    destroy_exec_state(state);
    return -1;
}
