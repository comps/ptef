#!/bin/bash
#set -x

TEF_ARGV0_NAME=$(basename "$0")
TEF_ARGV0_DNAME=$(dirname "$0")
TEF_ARGV=("$@")
#TEF_PREFIX="${TEF_PREFIX:-/}"
TEF_TTY_FD=0

tef_err() { echo "error: $1" 1>&2; exit 1; }
tef_warn() { echo "warn: $1" 1>&2; }

is_fd_open() {
    # (doesn't work for 2, obviously)
    command >&${1} 2>/dev/null
}

# log a status for a test name (path)
# - to terminal, if available
# - to stdout, if available
log_status() {
    local status="$1" name="${2#./}"
    if [ -t "$TEF_TTY_FD" ]; then
        echo "$status $TEF_PREFIX/$name" >&${TEF_TTY_FD}
    fi
#    if is_fd_open 1; then
#        echo -ne "${1}\0${2}\0"
#    fi
}

# if $1 is dir, run runner inside it, pass it args
# if $1 is exec file, run it, pass it args
tef_run_child() {
    local args=("$@")
    args[0]="${args[0]#./}" # tidy up
    local base="${args[0]}"

    if [ -d "$base" ]; then
        if [ -x "$base/$TEF_ARGV0_NAME" ]; then
            pushd "$base" >/dev/null
            log_status RUN "$base"
            args[0]="./$TEF_ARGV0_NAME"
            TEF_PREFIX="$TEF_PREFIX/$base" "${args[@]}"
            [ $? -eq 0 ] && log_status PASS "$base" || log_status FAIL "$base"
            popd >/dev/null
        fi
    elif [ -x "$base" ]; then
        log_status RUN "$base"
        args[0]="./$1"
        "${args[@]}"
        [ $? -eq 0 ] && log_status PASS "$base" || log_status FAIL "$base"
    else
        tef_warn "$base not dir/exec, skipping"
    fi
}

# given an absolute path, return relative path from the runner's location
# (essentially just omit the leading '/')
# given a relative path from the caller's CWD, return path relative to
# it from the runner's location
tef_normalize_path() {
    local path="$1"
    if [ "${path::1}" = "/" ]; then
        path=$(realpath -ms "$path")  # remove trailing '/', '//', '..', etc.
        echo "${path:1}"  # omit leading '/'
    else
        realpath -ms --relative-to="$TEF_ARGV0_DNAME" "$path"
    fi
}

tef_run() {
    local name="$1" target="$2"
    [ "$target" ] || target="$name"
    local child=

    # prepare stdout logging of this process/runner
    # (only if stdout != terminal, else close stdout)
    local term=$(readlink -s "/proc/$$/fd/$TEF_TTY_FD")
    local out=$(readlink -s "/proc/$$/fd/1")
    #[ "$term" = "$out" ] && exec 1>&-

    # no args, run all without args
    if [ "${#TEF_ARGV[@]}" -eq 0 ]; then
        # do two loops to avoid stdin (fd 0) redirection issues;
        # (the < <(find..) would overwrite the original tty on fd 0)
        local children=()
        while read -r -d '' child; do
            children+=("$child")
        done < <(find . -mindepth 1 -maxdepth 1 -not -name "$TEF_ARGV0_NAME" \
                        -print0 | sort -z)
        for child in "${children[@]}"; do
            tef_run_child "$child"
        done

    # some args - run only the specified tests
    else
        local argv=() arg= i=0 arr= prefix= postfix=

        for arg in "${TEF_ARGV[@]}"; do
            arg=$(tef_normalize_path "$arg")
            case "$arg" in
                *..*) tef_err "given path $path outside the runner scope" ;;
            esac
            argv+=("$arg")
        done

        while [ "$i" -lt "${#argv[@]}" ]; do
            arr=()

            # gather all args with a common prefix in sequential order,
            # store their postfixes (arg without prefix)

            prefix="${argv[$i]%%/*}"
            postfix="${argv[$i]#*/}"
            [ "$postfix" = "${argv[$i]}" ] && postfix=

            # leaf part hit, no coalescing possible, just run
            if [ -z "$postfix" ]; then
                tef_run_child "$prefix"
                (( i++ ))
                continue
            fi

            # got a valid prefix and postfix
            for (( ; i < "${#argv[@]}" ; i++ )); do
                postfix="${argv[$i]#*/}"
                [ "$postfix" = "${argv[$i]}" ] && postfix=

                # if we hit a non-coalesce-able member, stop
                if [ -z "$postfix" -o "${argv[$i]%%/*}" != "$prefix" ]; then
                    break
                fi
                arr+=("$postfix")
            done

            arr=("$prefix" "${arr[@]}")
            tef_run_child "${arr[@]}"
        done
    fi
}

tef_run

# vim: sts=4 sw=4 et :
