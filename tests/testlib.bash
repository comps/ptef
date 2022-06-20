_thisdir=$(dirname "${BASH_SOURCE[0]}")
_thisdirabs=$(cd "$_thisdir"; echo "$PWD")
export PATH="$_thisdirabs/bin:$PATH"
export LD_LIBRARY_PATH="$_thisdirabs/bin"
export TEST_BINDIR="$_thisdirabs/bin"

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
	} >> exec_log${suffix}
	EOF
}

function make_var_printer {
	local varname="$1"
	local suffix="${2:+.$2}"
	cat <<-EOF
	#!/bin/bash
	if env | grep -q $varname; then echo "\$$varname"
	else echo "unset"
	fi >> exec_log${suffix}
	EOF

}

function assert_contents {
	{
		local content=
		IFS= read -r -d '' content < "$2" || true  # will hit EOF
		# supports globbing - see bash(1) on [[
		[[ $content == $1 ]] || {
			echo "assert_contents failed, ${@: -1} contains:"
			echo "$content"
			return 1
		}
	} 1>&2 2>/dev/null
}

function assert_grep {
	{
		grep "$@" || {
			echo "assert_grep failed, ${@: -1} contains:"
			cat "${@: -1}"
			return 1
		}
	} 1>&2 2>/dev/null
}
function assert_nogrep {
	{
		! grep "$@" || {
			echo "assert_nogrep failed, ${@: -1} contains:"
			cat "${@: -1}"
			return 1
		}
	} 1>&2 2>/dev/null
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

function clear_cleanup {
	{
		function _cleanup_buffer {
			:
		}
	} 1>&2 2>/dev/null
}
clear_cleanup
function execute_cleanup {
	_cleanup_buffer
	clear_cleanup
}
function append_cleanup {
	{
		eval "function _cleanup_buffer {
			$(declare -f _cleanup_buffer | sed '1,2d;$d')
			$*
		}"
	} 1>&2 2>/dev/null
}
function prepend_cleanup {
	{
		eval "function _cleanup_buffer {
			$*
			$(declare -f _cleanup_buffer | sed '1,2d;$d')
		}"
	} 1>&2 2>/dev/null
}

# for bash scripts that aren't actually tests
[ -n "$TESTLIB_NOOP" ] && return


set -e

[ -n "$PTEF_RESULTS_FD" ] && exec {PTEF_RESULTS_FD}>&-

unset PTEF_BASENAME PTEF_COLOR PTEF_LOGS PTEF_NOLOGS PTEF_PREFIX \
	PTEF_RESULTS_FD PTEF_RUN PTEF_SHELL PTEF_SILENT

rm -rf testdir
mkdir testdir
rm -rf tmpdir
mkdir tmpdir

append_cleanup "(cd \"$PWD\" && grep -D skip -R . testdir tmpdir >&2) || true"
trap '{ echo -------------------------; } >&2 2>/dev/null; execute_cleanup' EXIT

tmpdir="$PWD/tmpdir"
cd testdir

set -x
