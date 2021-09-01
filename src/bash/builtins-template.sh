# try to make this snippet as compatible as possible
if [ -z $BASH_VERSION ]; then
	echo ptef/bash-builtins.sh is Bash-only.
	echo Do not source it from non-Bash shells.
	exit 1
fi

# Bash only below

[ -z "$LIBPTEF_BASH_SOURCED" ] || return 0

if [ "${BASH_VERSINFO[0]}" -lt 4 -o "${BASH_VERSINFO[0]}" -eq 4 -a "${BASH_VERSINFO[1]}" -lt 4 ]; then
	echo "ptef/bash-builtins.sh supports only Bash 4.4+" >&2
	return 1
fi

LIBPTEF_BASH="${LIBPTEF_BASH:-TEMPLATE_LIBDIR/libptef-bash.so}"
[ "${LIBPTEF_BASH::1}" != "/" ] && LIBPTEF_BASH="$PWD/$LIBPTEF_BASH"

enable -f "$LIBPTEF_BASH" \
	ptef_set_status_colors \
	ptef_report \
	ptef_mklog

# alias runner for compatibility
# see explanation for why this isn't a true builtin in bash-builtins.c
alias ptef_runner='ptef-runner'

LIBPTEF_BASH_SOURCED=1
