#!/bin/bash

function wait_lock {
	local lock_info="$1" i=
	# wait for hold-lock to actually lock the fd
	for ((i=0;i<100;i++)); do
		grep -q '^locked$' "$lock_info" && break
		sleep 0.1
	done
	[ "$i" -lt 100 ] || return 1
	return 0
}

export PATH="$PWD:$PATH"

_thisdir=$(dirname "${BASH_SOURCE[0]}")
. "$_thisdir/../testlib.bash"
