.PHONY: all
all: list-fds

CFLAGS ?= -O2 -Wall -Wextra -Werror -g
override CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809

list-fds: list-fds.c

.PHONY: clean
clean:
	rm -f list-fds
