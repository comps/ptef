"""
This is the python interface of a reference implementation of the
Portable Test Execution Framework (PTEF).

It mostly maps 1:1 to the C interface while using pythonic data types,
see the ptef_runner(3), ptef_report(3) and ptef_mklog(3) manpages.

One notable difference is the use of a dedicated set_status_colors()
function to set the ptef_status_colors global variable in the C library,
as python cannot easily define a native 'char **colors[][2]' array.
Same for set_exit_statuses().

Some functions use bitmask-based flags. This python interface exposes
them as module variables with names identical to the C #defines, without
the leading PTEF_ part of their names.
"""

import os
import sys
import typing
import ctypes
import argparse
import textwrap


libptef = ctypes.CDLL("libptef.so.0", use_errno=True)

# runner flags
NOMERGE     = (1 << 0)

# report flags
NOLOCK      = (1 << 0)
NOWAIT      = (1 << 1)
RAWNAME     = (1 << 2)

# mklog flags
NOROTATE    = (1 << 0)


def runner(argv: list = None, jobs: int = 0, ignored: list = None,
           flags: int = 0) -> None:
    """
    Launch the PTEF runner logic, as implemented by ptef_runner(3).

    Note the lack of 'argc' - this python wrapper does len(argv) for you.
    Also note that 'argv' must contain the program name, or 'argv[0]',
    so it must always contain at least one element. If it is left None
    or empty, a default of 'sys.argv' is used.
    """
    if not argv:  # None or []
        argv = [sys.argv[0]]
    argc = len(argv)
    argv_bstrings = (x.encode('utf-8') for x in argv)

    if not ignored:
        ignored = []
    ignored_bstrings = [x.encode('utf-8') for x in ignored]
    ignored_bstrings.append(None)  # NULL-terminate

    ctypes.set_errno(0)
    rc = libptef.ptef_runner(ctypes.c_int(argc),
                             (ctypes.c_char_p * argc)(*argv_bstrings),
                             ctypes.c_int(jobs),
                             (ctypes.c_char_p * len(ignored_bstrings))(*ignored_bstrings),
                             ctypes.c_int(flags))
    if rc == -1:
        errno = ctypes.get_errno()
        raise OSError(errno, os.strerror(errno), None)


_custom_status_colors = None
def set_status_colors(src: dict):
    """
    Set a new color map to be used for statuses when reporting results.

    This takes a dict() that gets transformed to 'char *colors[][2]' and set
    as the ptef_status_colors global variable in the loaded C library.
    See ptef_report(3) for more details.

    The dict values can contain ASCII color codes, extra spaces, etc.
    Ie.
    {
        "FAIL":  "\\033[1;41mFAIL\\033[0m ",
        "WAIVE": "\\033[1;33mWAIVE\\033[0m",
    }
    """
    pairs = [
        (
            ctypes.c_char_p(x.encode('utf-8')),
            ctypes.c_char_p(src[x].encode('utf-8'))
        ) for x in src
    ]
    pairs.append((ctypes.c_char_p(None), ctypes.c_char_p(None))) # NULL-terminate
    # ((ctypes.c_char_p * 2) * 5) , a char[5][2] data type
    char_n_2_type = (ctypes.c_char_p * 2) * len(pairs)
    global _custom_status_colors
    _custom_status_colors = char_n_2_type(*pairs)
    current_map = ctypes.POINTER(char_n_2_type).in_dll(libptef, 'ptef_status_colors')
    current_map.contents = _custom_status_colors


_custom_exit_statuses = None
_custom_exit_statuses_default = None
def set_exit_statuses(src: dict, default: str):
    """
    Set a custom exit-code-to-status map, used only by runner() for reporting
    results when running executables.

    This takes a dict() with int exit codes as keys and their str statuses as
    dict values. A default status (for exit codes not present in the dict)
    must also be supplied.
    See ptef_runner(3) for more details.

    Obviously, the int keys must never be < 0 or > 255.
    Ie.
    {
        0: "PASS",
        2: "WARN",
        3: "ERROR",
    }
    with "FAIL" passed as default
    """
    new = [None]*256
    for key, value in src.items():
        if key < 0 or key > 255:
            raise IndexError("key must be within 0-255")
        new[key] = ctypes.c_char_p(value.encode('utf-8'))
    default_bstr = default.encode('utf-8')
    char_256_type = ctypes.c_char_p * 256
    char_default_type = ctypes.c_char * len(default)
    global _custom_exit_statuses
    global _custom_exit_statuses_default
    _custom_exit_statuses = char_256_type(*new)
    _custom_exit_statuses_default = char_default_type(*default_bstr)
    current_map = ctypes.POINTER(char_256_type).in_dll(libptef, 'ptef_exit_statuses')
    current_map.contents = _custom_exit_statuses
    current_default = ctypes.POINTER(ctypes.c_char_p).in_dll(libptef, 'ptef_exit_statuses_default')
    current_default.contents = _custom_exit_statuses_default


def report(status: str, testname: str, flags: int = 0) -> None:
    """
    Produce a report line on stdout, as implemented by ptef_report(3).
    """
    rc = libptef.ptef_report(ctypes.c_char_p(status.encode('utf-8')),
                             ctypes.c_char_p(testname.encode('utf-8')),
                             ctypes.c_int(flags))
    if rc == -1:
        errno = ctypes.get_errno()
        raise OSError(errno, os.strerror(errno), None)


def mklog(testname: str, flags: int = 0) -> typing.BinaryIO:
    """
    Create and open a log file for a test, as implemented by ptef_mklog(3).

    Instead of returning an open file descriptor number, like the C function
    would, this interface transforms it into a python File-like object, so
    you can do ie.

    with ptef.mklog(test) as f:
        f.write(...)
        # or subprocess.run(['...'], stderr=f)
    """
    ctypes.set_errno(0)
    fd = libptef.ptef_mklog(ctypes.c_char_p(testname.encode('utf-8')),
                            ctypes.c_int(flags))
    if fd == -1:
        errno = ctypes.get_errno()
        raise OSError(errno, os.strerror(errno), None)
    else:
        return os.fdopen(fd, 'wb')


class RunnerCLI:
    """
    Emulate the command line interface of the ptef-runner utility, which uses
    the C-based getopt(3) and a lot of related logic to provide a standard
    CLI for accesing most of the PTEF runner features.

    Use this class if you want to write your own CLI-based ptef-runner like
    utility and would like to use the semi-standard command line options,
    possibly extending them with your features.

    runner = ptef.RunnerCLI()
    runner.run()
    """
    epilog = textwrap.dedent(r"""
        Executes the PTEF runner logic from CWD, executing executables and traversing
        subdirectories.
        If TEST is specified, runs only that test, without searching for executables.

        The -i option can be specified multiple times.

        Custom exit code MAP is a space-separated "NUMBER:STATUS" set of pairs,
        with the last separated element specifying a default fallback STATUS.
        For example: -x '0:PASS 2:WARN 3:ERROR FAIL'
    """)
    def __init__(self, **argparse_args):
        self.parser = argparse.ArgumentParser(
            epilog=self.epilog, allow_abbrev=False, add_help=False,
            usage="%(prog)s [OPTIONS] [TEST]...",
            formatter_class=argparse.RawTextHelpFormatter, **argparse_args)
        self.add_option_args()
        self.add_remainder_arg()

    def add_option_args(self):
        p = self.parser
        p.add_argument('-h', action='store_true', help=argparse.SUPPRESS)
        p.add_argument('-a', metavar='BASE', help="runner basename, overriding autodetection from basename(argv[0])")
        p.add_argument('-A', metavar='BASE', help="set and export PTEF_BASENAME, overriding even -a")
        p.add_argument('-j', type=int, default=0, metavar='NR', help="number of parallel jobs (tests)")
        p.add_argument('-i', action='append', metavar='IGN', help="ignore a file/dir named IGN when searching for executables")
        p.add_argument('-x', metavar='MAP', help="use a non-standard exit-code-to-status mapping")
        p.add_argument('-r', action='store_true', help="set PTEF_RUN and export it")
        p.add_argument('-s', action='store_true', help="set PTEF_SILENT and export it")
        p.add_argument('-v', action='store_true', help="set PTEF_NOLOGS and export it (\"verbose\")")
        p.add_argument('-d', action='store_true', help="set PTEF_SHELL to 1 and export it (\"debug\")")
        p.add_argument('-m', action='store_true', help="don't merge arguments of subrunners (always pass 1 arg)")

    def add_remainder_arg(self):
        self.parser.add_argument('rest', nargs=argparse.REMAINDER, help=argparse.SUPPRESS)

    def postprocess_args(self, args):
        # I wish python had better handling of unknown remainder args :(
        if len(args.rest) > 0 and args.rest[0] == '--':
            args.rest.pop(0)
        args.rest.insert(0, self.parser.prog)

        if args.h:
            self.parser.print_help(file=sys.stderr)
            sys.exit(0)

        if args.A is not None:
            os.environ['PTEF_BASENAME'] = args.A
            args.a = args.A

        if args.a is not None:
            args.rest[0] = args.a

        if args.x is not None:
            pairs = args.x.split(' ')
            if len(pairs) < 2:
                self.parser.error(f"need at least two map elements: {args.x}")
            default = pairs.pop(-1)
            code_map = dict()
            for pair in pairs:
                code_status = pair.split(':', 1)
                if len(code_status) < 2:
                    self.parser.error(f"missing colon separator: {pair}")
                code, status = code_status
                if not status:
                    self.parser.error(f"empty status string: {pair}")
                try:
                    code = int(code)
                except ValueError:
                    self.parser.error(f"invalid exit code nr: {code}")
                code_map[code] = status
            set_exit_statuses(code_map, default)

        if args.r:
            os.environ['PTEF_RUN'] = str(1)

        if args.s:
            os.environ['PTEF_SILENT'] = str(1)

        if args.v:
            os.environ['PTEF_NOLOGS'] = str(1)

        if args.d:
            os.environ['PTEF_SHELL'] = str(1)

        self.flags = 0
        if args.m:
            self.flags |= NOMERGE

    def run(self, *args):
        """
        Parse a list of command line arguments and run the PTEF runner logic.
        """
        args = self.parser.parse_args(*args)
        self.postprocess_args(args)
        runner(argv=args.rest, jobs=args.j, ignored=args.i, flags=self.flags)
