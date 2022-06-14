#!/bin/bash

function mkfile {
	make_arg_printer "$@"
}

_thisdir=$(dirname "${BASH_SOURCE[0]}")
. "$_thisdir/../testlib.bash"
