include ../common.mk

.PHONY: all install clean
all: report.o

CFLAGS ?= -O2 -flto -Wall -g
CFLAGS += -I../include -I../helpers -fPIC

report.o: report.c

install: ;

clean:
	rm -f report.o
