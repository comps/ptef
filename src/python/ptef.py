import typing, ctypes, sys, os

libptef = ctypes.CDLL("libptef.so.0", use_errno=True)

def runner(argv: list = None, default_basename: str = None, jobs: int = 1,
           nomerge: bool = False) -> None:
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
                             ctypes.c_int(nomerge))
    if rc == -1:
        errno = ctypes.get_errno()
        raise OSError(errno, os.strerror(errno), None)

def report(status: str, testname: str) -> None:
    ctypes.set_errno(0)
    rc = libptef.ptef_report(ctypes.c_char_p(status.encode('utf-8')),
                             ctypes.c_char_p(testname.encode('utf-8')))
    if rc == -1:
        errno = ctypes.get_errno()
        raise OSError(errno, os.strerror(errno), None)

def mklog(testname: str) -> typing.BinaryIO:
    ctypes.set_errno(0)
    fd = libptef.ptef_mklog(ctypes.c_char_p(testname.encode('utf-8')))
    if fd == -1:
        errno = ctypes.get_errno()
        raise OSError(errno, os.strerror(errno), None)
    else:
        return os.fdopen(fd, 'wb')
