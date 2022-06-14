#!/bin/bash

function mkfile {
	make_arg_printer "$@"
}
function mk2execs {
	mkfile > exec1
	mkfile > exec2
	chmod +x exec1 exec2
}
function mk3execs {
	mkfile > exec1
	mkfile > exec2
	mkfile > exec3
	chmod +x exec1 exec2 exec3
}

_thisdir=$(dirname "${BASH_SOURCE[0]}")
. "$_thisdir/../testlib.bash"
