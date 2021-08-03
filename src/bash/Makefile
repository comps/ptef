.PHONY: all
all: libptef-bash.so

CFLAGS ?= -O2 -Wall -g
CFLAGS += -I/usr/include/bash/include -I../include

libptef-bash.so: libptef-bash.c
	$(CC) -shared -fPIC -L../lib $(CFLAGS) -o $@ $^ -lptef

.PHONY: clean
clean:
	rm -f libptef-bash.so
