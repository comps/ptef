include ../common.mk

.PHONY: all install clean
all: mklog.o

override CFLAGS += -I../include -I../helpers \
			-fPIC -std=c99 -D_POSIX_C_SOURCE=200809

mklog.o: mklog.c

install: ;

clean:
	rm -f mklog.o
