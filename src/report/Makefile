include ../common.mk

.PHONY: all install clean
all: report.o

CFLAGS += -I../include -I../helpers -fPIC -D_POSIX_C_SOURCE=200809

report.o: report.c

install: ;

clean:
	rm -f report.o
