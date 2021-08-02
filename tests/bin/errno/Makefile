.PHONY: all
all: wrap.so

CFLAGS ?= -O2 -Wall -g
CFLAGS += -I../../../src/include -fPIC -shared

wrap.so: wrap.c
	$(CC) $(CFLAGS) -o $@ $< -ldl

.PHONY: clean
clean:
	rm -f wrap.so
