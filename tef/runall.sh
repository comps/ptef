#!/bin/bash
#set -x

TEF_ARGV0_NAME=$(basename "$0")
TEF_ARGV0_DNAME=$(dirname "$0")
TEF_ARGV=("$@")

tef_err() {
    echo "error: $1" >&2
    exit 1
}
tef_warn() {
    echo "warn: $1" >&2
}

setup_io() {
    if [ -t 0 ]; then
        # we have a terminal, redirect verbose messages as well as any
        # unexpected errors to it
        exec 2>&0
    else
        # no terminal, suppress verbose/error messages
        exec 2>/dev/null
    fi

    # if stdout is different (term or not) from the reference terminal,
    # leave it open for binary output, else suppress it
    local term=$(readlink -s "/proc/$$/fd/0")
    local out=$(readlink -s "/proc/$$/fd/1")
    [ "$term" = "$out" ] && exec 1>/dev/null
}

# log a status for a test name (path)
# - to terminal, if available
# - to stdout, if available
log_status() {
    local status="$1" name="${2#./}"
    if [ "$TERM" -a "$(tput colors)" -ge 8 ]; then
        case "$status" in
            PASS) echo -ne "\033[1;32m${status}\033[0m" >&2 ;;
            FAIL) echo -ne "\033[1;31m${status}\033[0m" >&2 ;;
            RUN) echo -ne "\033[1;34m${status} \033[0m" >&2 ;;
            *) echo -n "$status" >&2 ;;
        esac
        echo " $TEF_PREFIX/$name" >&2
    else
        echo "$status $TEF_PREFIX/$name" >&2
    fi
    echo -ne "${1}\0${2}\0"
}

# if $1 is dir, run runner inside it, pass it args
# if $1 is exec file, run it, pass it args
tef_run_child() {
    local args=("$@")
    args[0]="${args[0]#./}" # tidy up
    local base="${args[0]}"

    # prepare child log
    # TODO: this may create empty logdir if no tests are found,
    #       solve this better in python/C implementation
    local logdir=logs
    [ "$TEF_LOGS" ] && logdir="${TEF_LOGS}${TEF_PREFIX}/logs"
    mkdir -p "$logdir"

    # run the child process
    if [ -d "$base" ]; then
        if [ -x "$base/$TEF_ARGV0_NAME" ]; then
            log_status RUN "$base"
            args[0]="./$TEF_ARGV0_NAME"
            {
                pushd "$base" >/dev/null
                TEF_PREFIX="$TEF_PREFIX/$base" "${args[@]}"
                popd >/dev/null
            } &>> "$logdir/$base"
            [ $? -eq 0 ] && log_status PASS "$base" || log_status FAIL "$base"
        fi
    elif [ -x "$base" ]; then
        log_status RUN "$base"
        args[0]="./$1"
        "${args[@]}" &>> "$logdir/$base"
        [ $? -eq 0 ] && log_status PASS "$base" || log_status FAIL "$base"
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

    setup_io

    # prepare binary header
    echo -ne 'tefresults\0'

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
                if [ -d "$prefix" -o -x "$prefix" ]; then
                    tef_run_child "$prefix"
                else
                    tef_warn "$prefix not dir/exec, skipping"
                fi
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

            # skip non-dirs and non-executables
            if [ -d "$prefix" -o -x "$prefix" ]; then
                arr=("$prefix" "${arr[@]}")
                tef_run_child "${arr[@]}"
            else
                tef_warn "$prefix not dir/exec, skipping"
            fi
        done
    fi
}

tef_run

# vim: sts=4 sw=4 et :
