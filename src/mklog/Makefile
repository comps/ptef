include ../common.mk

.PHONY: all install clean
all: mklog.o

CFLAGS ?= -O2 -flto -Wall -Wextra -Werror -g -D_POSIX_C_SOURCE=200809
CFLAGS += -I../include -I../helpers -fPIC

mklog.o: mklog.c

install: ;

clean:
	rm -f mklog.o
