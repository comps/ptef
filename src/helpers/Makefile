include ../common.mk

.PHONY: all install clean
all: ptef_helpers.o

override CFLAGS += -I../include -fPIC -std=c99 -D_POSIX_C_SOURCE=200809

ptef_helpers.o: ptef_helpers.c

install: ;

clean:
	rm -f ptef_helpers.o
	rm -f \*.o  # generated when the build fails, sometimes
