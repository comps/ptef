include ../common.mk

.PHONY: all install clean
all: runner.o common.o execute.o with_args.o without_args.o

CFLAGS += -I../include -I../helpers -fPIC -D_POSIX_C_SOURCE=200809

runner.o: runner.c
common.o: common.c
execute.o: execute.c
with_args.o: with_args.c
without_args.o: without_args.c

install: ;

clean:
	rm -f runner.o common.o execute.o with_args.o without_args.o
