#!/bin/bash

function mkfile {
	make_var_printer PTEF_PREFIX "$@"
}

_thisdir=$(dirname "${BASH_SOURCE[0]}")
. "$_thisdir/../testlib.bash"
