include ../common.mk

.PHONY: all install clean
all: ptef_helpers.o

CFLAGS ?= -O2 -flto -Wall -Wextra -Werror -g -D_POSIX_C_SOURCE=200809
CFLAGS += -I../include -fPIC

ptef_helpers.o: ptef_helpers.c

install: ;

clean:
	rm -f ptef_helpers.o
