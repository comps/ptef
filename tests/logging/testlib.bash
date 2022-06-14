#!/bin/bash

function mkfile {
	make_var_printer PTEF_LOGS "$@"
	echo 'echo "stderr output" >&2'
}

_thisdir=$(dirname "${BASH_SOURCE[0]}")
. "$_thisdir/../testlib.bash"
