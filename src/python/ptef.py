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

def runner(argv: list = None, default_basename: str = None, jobs: int = 0,
           mark_interval: int = 0, flags: int = 0) -> None:
    if argv is None:
        argv = []
    if default_basename is None:
        default_basename = os.path.basename(sys.argv[0])
    argc = len(argv)
    argv_bstrings = (x.encode('utf-8') for x in argv)
    ctypes.set_errno(0)
    rc = libptef.ptef_runner(ctypes.c_int(argc),
                             (ctypes.c_char_p * argc)(*argv_bstrings),
                             ctypes.c_char_p(default_basename.encode('utf-8')),
                             ctypes.c_int(jobs),
                             ctypes.c_int(mark_interval),
                             ctypes.c_int(flags))
    if rc == -1:
        errno = ctypes.get_errno()
        raise OSError(errno, os.strerror(errno), None)

def _dict_to_2d_array(src: dict):
    pairs = [
        (
            ctypes.c_char_p(x.encode('utf-8')),
            ctypes.c_char_p(src[x].encode('utf-8'))
        ) for x in src
    ]
    pairs.append((ctypes.c_char_p(None),ctypes.c_char_p(None))) # NULL-terminate
    # ((ctypes.c_char_p * 2) * 5) , a char[5][2] data type
    return ((ctypes.c_char_p * 2) * len(pairs))(*pairs)

def report(status: str, testname: str, colors: dict = None,
           flags: int = 0) -> None:
    if colors is None:
        colors_array = ctypes.c_char_p(None)
    else:
        colors_array = _dict_to_2d_array(colors)
    rc = libptef.ptef_report(ctypes.c_char_p(status.encode('utf-8')),
                             ctypes.c_char_p(testname.encode('utf-8')),
                             colors_array,
                             ctypes.c_int(flags))
    if rc == -1:
        errno = ctypes.get_errno()
        raise OSError(errno, os.strerror(errno), None)

def mklog(testname: str, flags: int = 0) -> typing.BinaryIO:
    ctypes.set_errno(0)
    fd = libptef.ptef_mklog(ctypes.c_char_p(testname.encode('utf-8')),
                            ctypes.c_int(flags))
    if fd == -1:
        errno = ctypes.get_errno()
        raise OSError(errno, os.strerror(errno), None)
    else:
        return os.fdopen(fd, 'wb')
