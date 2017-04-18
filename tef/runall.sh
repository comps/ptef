#!/bin/bash
set -x

TEF_ARGV0_NAME=$(basename "$0")
TEF_ARGV0_DNAME=$(dirname "$0")
TEF_ARGV=("$@")

tef_err() { echo "error: $1" 1>&2; exit 1; }
#tef_warn() { echo "error: $1" 1>&2; }

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
#
#        path=$(realpath -ms --relative-to="$TEF_ARGV0_DNAME" "$path")
#        case "$path" in
#            *..*)
#            tef_warn "given path $path outside the runner scope, skipping" ;;
#        esac
#        echo "$path"
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
            for (( ; i < "${#argv[@]}" ; i++ )); do
                if [ "${argv[$i]%%/*}" = "$prefix" ]; then
                    postfix="${argv[$i]#*/}"
                    # if there's no '/', bash doesn't remove anything, so treat
                    # last path element as special (don't add anything to arr)
                    if [ "$postfix" != "${argv[$i]}" ]; then
                        arr+=("$postfix")
                    fi
                else
                    break
                fi
            done

            arr=("$prefix" "${arr[@]}")
            #echo would run "${arr[@]}"
            tef_run_child "${arr[@]}"
        done
    fi
}

tef_run

# vim: sts=4 sw=4 et :
