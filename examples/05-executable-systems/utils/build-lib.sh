function mkexec {
	cat <<-'EOF'
	#!/bin/bash -x
	echo running as $0 >&2
	exit 0
	EOF
}

function recurse {
	local base="$1" subdir=
	for subdir in *; do
		[ -x "$subdir/$base" ] || continue
		( cd "$subdir" && "./$base" ) || return 1
	done
}
