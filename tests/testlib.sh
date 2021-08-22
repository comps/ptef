export PATH="$PWD/bin:$PATH"
export LD_LIBRARY_PATH="$PWD/bin"
export TEST_BINDIR="$PWD/bin"

#function err { echo >&2; echo "ERROR: $@" >&2; echo >&2; _failed=1; }

#function hide {
#	{ set +x; } 2>/dev/null
#	"$@"
#	{ set -x; } 2>/dev/null
#}

function cmd_in_PATH {
	local cmd=$(command -v "$1")
	[ "${cmd::1}" = "/" ]
}

function link_from_PATH {
	local src=$(command -v "$1")
	[ -e "$src" ] || return 1
	ln -s "$src" $2
}

function make_arg_printer {
	local suffix="${1:+.$1}"
	cat <<-EOF
	#!/bin/bash
	{ echo "argv0: \$0"
	  echo "argc: \$#"
	  echo "argv: \$@"
	} > exec_log${suffix}
	EOF
}

function make_var_printer {
	local varname="$1"
	local suffix="${2:+.$2}"
	cat <<-EOF
	#!/bin/bash
	if env | grep -q $varname; then echo "\$$varname"
	else echo "unset"
	fi > exec_log${suffix}
	EOF

}

function assert_contents {
	local content=$(<"$2")
	# support globbing
	case "$content" in $1) true;; *) false;; esac || {
		{
			echo "assert_contents failed, ${@: -1} contains:"
			echo "$content"
			return 1
		} 1>&2 2>/dev/null
	}
}

function assert_grep {
	grep "$@" >&2 || {
		{
			echo "assert_grep failed, ${@: -1} contains:"
			cat "${@: -1}"
			return 1
		} 1>&2 2>/dev/null
	}
}
function assert_nogrep {
	! grep "$@" >&2 || {
		{
			echo "assert_nogrep failed, ${@: -1} contains:"
			cat "${@: -1}"
			return 1
		} 1>&2 2>/dev/null
	}
}

function assert_ptef_runner {
	$TEST_WRAPPER ptef-runner "$@" 2>"$tmpdir/runner_err"
	! grep '' "$tmpdir/runner_err" >&2
}
function assert_ptef_report {
	$TEST_WRAPPER ptef-report "$@" 2>"$tmpdir/report_err"
	! grep '' "$tmpdir/report_err" >&2
}
function assert_ptef_mklog {
	$TEST_WRAPPER ptef-mklog "$@" 2>"$tmpdir/mklog_err"
	! grep '' "$tmpdir/mklog_err" >&2
}

_added_tests=()
function + {
	_added_tests+=("$*")
}
function _run_test {
	rm -rf testdir && mkdir testdir && \
	rm -rf tmpdir && mkdir tmpdir || return 1
	(
		set -e
		[ -n "$PTEF_RESULTS_FD" ] && exec {PTEF_RESULTS_FD}>&-
		unset PTEF_PREFIX PTEF_LOGS PTEF_RESULTS_FD PTEF_NOLOGS \
			PTEF_COLOR PTEF_IGNORE_FILES PTEF_BASENAME PTEF_RUN
		tmpdir="$PWD/tmpdir"
		cd testdir
		set -x
		"$1"
	)
}

function run_tests {
	local _tst= _tests=
	[ "$#" -gt 0 ] && _tests=("$@") || _tests=("${_added_tests[@]}")
	for _tst in "${_tests[@]}"; do
		[ -n "$PTEF_RUN" ] && ptef-report RUN "$_tst"
		# this must not be inside 'if', otherwise set -e will not
		# work in the subshell
		_run_test "$_tst"
		if [ $? -eq 0 ]; then
			ptef-report PASS "$_tst"
		else
			ptef-report FAIL "$_tst"
		fi
	done
}
