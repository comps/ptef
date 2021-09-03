"""
This is the python interface of a reference implementation of the
POSIX-inspired Test Execution Framework (PTEF).

It mostly maps 1:1 to the C interface while using pythonic data types,
see the ptef_runner(3), ptef_report(3) and ptef_mklog(3) manpages.

One notable difference is the use of a dedicated set_status_colors()
function to set the ptef_status_colors global variable in the C library,
as python cannot easily define a native 'char **colors[][2]' array.

Some functions use bitmask-based flags. This python interface exposes
them as module variables with names identical to the C #defines, without
the leading PTEF_ part of their names.
"""

import typing, ctypes, sys, os

libptef = ctypes.CDLL("libptef.so.0", use_errno=True)

# runner flags
NOMERGE     = (1 << 0)

# report flags
NOLOCK      = (1 << 0)
NOWAIT      = (1 << 1)
RAWNAME     = (1 << 2)

# mklog flags
NOROTATE    = (1 << 0)

def runner(argv: list = None, jobs: int = 0, flags: int = 0) -> None:
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
    ctypes.set_errno(0)
    rc = libptef.ptef_runner(ctypes.c_int(argc),
                             (ctypes.c_char_p * argc)(*argv_bstrings),
                             ctypes.c_int(jobs),
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
    pairs.append((ctypes.c_char_p(None),ctypes.c_char_p(None))) # NULL-terminate
    # ((ctypes.c_char_p * 2) * 5) , a char[5][2] data type
    char_n_2_type = (ctypes.c_char_p * 2) * len(pairs)
    global _custom_status_colors
    _custom_status_colors = char_n_2_type(*pairs)
    current_map = ctypes.POINTER(char_n_2_type).in_dll(libptef, 'ptef_status_colors')
    current_map.contents = _custom_status_colors

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
        # or print(..., file=f)
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
