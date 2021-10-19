include ../common.mk

.PHONY: all install clean
all: report.o

override CFLAGS += -I../include -I../helpers \
			-fPIC -std=c99 -D_POSIX_C_SOURCE=200809

report.o: report.c

install: ;

clean:
	rm -f report.o
