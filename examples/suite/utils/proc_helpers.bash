proc_list_pids() {
	ps --no-headers -eo pid
}
proc_list_sametty_pids() {
	ps --no-headers -t "$(tty)" -o pid
}
proc_list_user_pids() {
	ps --no-headers -a -o pid
}

proc_wallclock_runtime() {
	ps --no-headers -p "$1" -o etimes
}
proc_rss() {
	ps --no-headers -p "$1" -o rss
}
proc_state() {
	ps --no-headers -p "$1" -o state
}
proc_comm() {
	ps --no-headers -p "$1" -o comm
}

proc_show_rss() {
	ps -p "$1" -o pid,rss,cmd
}
