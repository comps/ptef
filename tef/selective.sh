# include this from actual runners
set -x

PATH="$PATH:."

TEF_ARGV=("$@")
TEF_ARGI=0  #index


# if $1 is dir, run runner inside it, pass it args
# if $1 is exec file, run it, pass it args
tef_run_child() {
    local run="$1"
    local args=("${@:2}")
    if [ -d "$run" ]; then
        local bname=$(basename "$0")
        if [ -x "$run/$bname" ]; then
            pushd "$run" >/dev/null
            [ "${#args[@]}" -gt 0 ] && "./$bname" "$args" || "./$bname"
            popd >/dev/null
        fi
    elif [ -x "$run" ]; then
        [ "${#args[@]}" -gt 0 ] && "./$run" "$args" || "./$run"
    fi
}


tef_collect_args() {

}


tef_run() {
    local name="$1" target="$2"
    [ "$target" ] || target="$name"

    #tef_run_child "$name"

    # no args, run all without args
    if [ "${#TEF_ARGV[@]}" -eq 0 ]; then
        tef_run_child "$target"

    # some args - run only the specified tests
    else 

    fi
}


+() {
    tef_run "$@"
}

#+() {
#    local sub="${BASH_ARGV[0]}"
#    if [ -d "$sub" ]; then
#        local bname=$(basename "$0")
#        if [ -x "$sub/$bname" ]; then
#            "./$sub/$bname"
#        fi
#    elif [ -x "$sub" ]; then
#        "./$sub"
#    fi
#    "$@"
#}
