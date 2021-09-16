function get_pid_owner {
	ps --no-headers -o user:1 -p "$1"
}
function get_pid_parent {
	ps --no-headers -o ppid:1 -p "$1"
}

# other functions here
