import typing, ctypes, sys, os

libptef = ctypes.CDLL("libptef.so", use_errno=True)

def runner(argv: list = None, basename: str = None, jobs: int = 1,
           nomerge: bool = False) -> None:
    if argv is None:
        argv = []
    if basename is None:
        basename = os.path.basename(sys.argv[0])
    argc = len(argv)
    argv_bstrings = (x.encode('utf-8') for x in argv)
    libptef.ptef_runner(ctypes.c_int(argc),
                        (ctypes.c_char_p * argc)(*argv_bstrings),
                        ctypes.c_char_p(basename.encode('utf-8')),
                        ctypes.c_int(jobs),
                        ctypes.c_int(nomerge))

def report(status: str, testname: str) -> None:
    libptef.ptef_report(ctypes.c_char_p(status.encode('utf-8')),
                        ctypes.c_char_p(testname.encode('utf-8')))

def mklog(testname: str) -> typing.BinaryIO:
    fd = libptef.ptef_mklog(ctypes.c_char_p(testname.encode('utf-8')))
    return os.fdopen(fd, 'wb')
