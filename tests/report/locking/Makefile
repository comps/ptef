.PHONY: all
all: hold-lock

CFLAGS ?= -O2 -Wall -Wextra -Werror -g
override CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809

hold-lock: hold-lock.c

.PHONY: clean
clean:
	rm -f hold-lock
