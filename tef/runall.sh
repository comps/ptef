#!/bin/bash
#set -x

TEF_ARGV0_NAME=$(basename "$0")
TEF_ARGV0_DNAME=$(dirname "$0")
TEF_ARGV=("$@")

tef_err() { echo "error: $1" 1>&2; exit 1; }
tef_warn() { echo "error: $1" 1>&2; }

# if $1 is dir, run runner inside it, pass it args
# if $1 is exec file, run it, pass it args
tef_run_child() {
    local args=("$@")
    if [ -d "$1" ]; then
        if [ -x "$1/$TEF_ARGV0_NAME" ]; then
            pushd "$1" >/dev/null
            args[0]="./$TEF_ARGV0_NAME"
            "${args[@]}"
            popd >/dev/null
        fi
    elif [ -x "$1" ]; then
        # not runnable without PATH
        if [ "${1::1}" != "/" -a "${1::2}" != "./" ]; then
            args[0]="./$1"
        fi
        "${args[@]}"
    else
        tef_warn "$1 not dir/exec, skipping"
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

    # no args, run all without args
    if [ "${#TEF_ARGV[@]}" -eq 0 ]; then
        while read -r -d '' child; do
            tef_run_child "$child"
        done < <(find . -mindepth 1 -maxdepth 1 -not -name "$TEF_ARGV0_NAME" \
                        -print0 | sort -z)

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
