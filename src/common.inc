#
# Copyright (C) 2018  Red Hat, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

normalize_path() {
    local path="$1"
    if [ "${path::1}" = "/" ]; then
        path=$(realpath -ms "$path")  # remove trailing '/', '//', '..', etc.
        echo "${path:1}"  # omit leading '/'
    else
        path=$(realpath -ms --relative-to="." "$path")
        echo "${path##*../}"  # trim possible leading '../../../'
    fi
}

# log a status for a test name (path)
log_status() {
    local status="$1" name="$2"

    if [ -t 0 -a "$(tput colors)" -ge 8 ]; then
        case "$status" in
            PASS) echo -ne "\033[1;32m${status}\033[0m" >&0 ;;
            FAIL) echo -ne "\033[1;31m${status}\033[0m" >&0 ;;
            RUN) echo -ne "\033[1;34m${status} \033[0m" >&0 ;;
            *) echo -n "$status" >&0 ;;
        esac
        echo " $TEF_PREFIX/$name" >&0
    else
        echo "$status $TEF_PREFIX/$name" 2>/dev/null >&0
    fi

    if [ "$TEF_RESULTS_FD" ]; then
        echo "$status $TEF_PREFIX/$name" 2>/dev/null >&"$TEF_RESULTS_FD"
    fi

    return 0
}

rotate_logfile() {
    local path="$1" i=
    [ -e "$path" ] || return 1
    for (( i = 7; i > 0; i-- )); do
        if [ -e "${path}.${i}" ]; then
            mv -f "${path}.${i}" "${path%%.$i}.$((i+1))"
        fi
    done
    rm -f "${path}.8"
    mv -f "$path" "${path}.1"
}
create_logfile() {
    local name="$1" path=

    if [ "$TEF_LOGS" ]; then
        [ -d "$TEF_LOGS" ] || mkdir "$TEF_LOGS" || return 1
        [ -d "$TEF_LOGS/$TEF_PREFIX" ] || \
            mkdir -p "$TEF_LOGS/$TEF_PREFIX" || return 1
        path="$TEF_LOGS/$TEF_PREFIX/${name}.log"
    else
        [ -d logs ] || mkdir logs || return 1
        path=logs/"$name"
    fi

    rotate_logfile "$path"
    touch "$path" || return 1
    echo "$path"
}

# if $1 is dir, run runner inside it, pass it args
# if $1 is exec file, run it, pass it args
run_child() {
    local args=("$@")
    args[0]="${args[0]#./}" # tidy up
    local child="${args[0]}"
    local logfile= rc=

    if [ -d "$child" ]; then
        if [ -x "$child/$TEF_ARGV0_NAME" ]; then
            logfile=$(create_logfile "$child") || return 1
            log_status RUN "$child"
            args[0]="./$TEF_ARGV0_NAME"
            {
                pushd "$child" >/dev/null
                TEF_PREFIX="$TEF_PREFIX/$child" "${args[@]}"
                rc=$?
                popd >/dev/null
            } &> "$logfile"
            [ $rc -eq 0 ] && log_status PASS "$child" || log_status FAIL "$child"
        fi
    elif [ -x "$child" ]; then
        logfile=$(create_logfile "$child") || return 1
        log_status RUN "$child"
        args[0]="./$1"
        TEF_PREFIX="$TEF_PREFIX/$child" "${args[@]}" &> "$logfile"
        [ $? -eq 0 ] && log_status PASS "$child" || log_status FAIL "$child"
    fi
}

tef_run() {
    local name="$1" target="$2"
    [ "$target" ] || target="$name"
    local child=

    # no args, run all without args
    if [ "${#TEF_ARGS[@]}" -eq 0 ]; then
        # do two loops to avoid stdin (fd 0) redirection issues;
        # (the < <(find..) would overwrite the original tty on fd 0)
        local children=()
        while read -r -d '' child; do
            children+=("$child")
        done < <(find . -mindepth 1 -maxdepth 1 \
                        -not -name "$TEF_ARGV0_NAME" -and -not -name '.*' \
                        -print0 | sort -z)
        for child in "${children[@]}"; do
            run_child "$child"
        done

    # some args - run only the specified tests
    else
        local argv=() arg= i=0 arr= prefix= postfix=

        for arg in "${TEF_ARGS[@]}"; do
            arg=$(normalize_path "$arg")
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
                    run_child "$prefix"
                else
                    echo "$prefix not dir/exec, skipping" >&2
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
                run_child "${arr[@]}"
            else
                echo "$prefix not dir/exec, skipping" >&2
            fi
        done
    fi
}

# vim: syntax=sh sts=4 sw=4 et :
