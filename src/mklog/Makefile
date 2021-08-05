include ../common.mk

.PHONY: all install clean
all: mklog.o

CFLAGS ?= -O2 -flto -Wall -g
CFLAGS += -I../include -I../helpers -fPIC

mklog.o: mklog.c

install: ;

clean:
	rm -f mklog.o
