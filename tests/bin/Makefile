# This is only for supporting binaries in bin/
# NOT for anything ptef runner related

.PHONY: all
all: ttee list-fds hold-lock

CFLAGS ?= -O2 -Wall -g -D_POSIX_C_SOURCE=200809

ttee: ttee.c
list-fds: list-fds.c
hold-lock: hold-lock.c

.PHONY: clean
clean:
	rm -f ttee list-fds hold-lock
