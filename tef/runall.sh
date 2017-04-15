#!/bin/bash
set -x

TEF_ARGV0_NAME=$(basename "$0")
TEF_ARGV=("$@")
TEF_ARGI=0  #index

# if $1 is dir, run runner inside it, pass it args
# if $1 is exec file, run it, pass it args
tef_run_child() {
    if [ -d "$1" ]; then
        if [ -x "$1/$TEF_ARGV0_NAME" ]; then
            pushd "$1" >/dev/null
            local args=("$@")
            args[0]="./$TEF_ARGV0_NAME"
            "${args[@]}"
            popd >/dev/null
        fi
    elif [ -x "$1" ]; then
        "$@"
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
        local i=0 arr= prefix= postfix=
        while [ "$i" -lt "${#TEF_ARGV[@]}" ]; do
            arr=()

            # gather all args with a common prefix in sequential order,
            # store their postfixes (arg without prefix)
            prefix="${TEF_ARGV[$i]%%/*}"
            for (( ; i < "${#TEF_ARGV[@]}" ; i++ )); do
                if [ "${TEF_ARGV[$i]%%/*}" = "$prefix" ]; then
                    postfix="${TEF_ARGV[$i]#*/}"
                    # if there's no '/', bash doesn't remove anything, so treat
                    # last path element as special (don't add anything to arr)
                    if [ "$postfix" != "${TEF_ARGV[$i]}" ]; then
                        arr+=("$postfix")
                    fi
                else
                    break
                fi
            done

            arr=("$prefix" "${arr[@]}")
            echo would run "${arr[@]}"
            #tef_run_child "${arr[@]}"
        done
    fi
}

tef_run

# vim: sts=4 sw=4 et :
