#
# Copyright (C) 2018  Red Hat, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

print_help() {
    echo "Usage: $0 [-a argv0] [ARG]..."
    echo
    echo "-a    Override detected argv0 basename (for subdir execution)"
}

TEF_ARGV0_NAME=$(basename "$0")
[ -n "$TEF_LOGS" ] && TEF_LOGS=$(realpath "$TEF_LOGS")  # no relative path

while getopts ":a:h" opt; do
    case "$opt" in
        a)
            TEF_ARGV0_NAME="$OPTARG"
            ;;
        h)
            print_help
            exit 0
            ;;
        :)
            echo "missing argument for: -${OPTARG}" >&2
            print_help >&2
            exit 1
            ;;
        \?)
            echo "unknown argument: -${OPTARG}" >&2
            print_help >&2
            exit 1
            ;;
    esac
done
shift "$((OPTIND-1))"

TEF_ARGS=("$@")

tef_run

# vim: syntax=sh sts=4 sw=4 et :
