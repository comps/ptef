# This is only for supporting binaries in bin/
# NOT for anything ptef runner related

.PHONY: all
all: ttee

CFLAGS ?= -O2 -Wall -Wextra -Werror -g
override CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809

ttee: ttee.c

.PHONY: clean
clean:
	rm -f ttee
