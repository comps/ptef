include ../common.mk

.PHONY: all install clean
all: libptef-bash.so builtins.sh

CFLAGS ?= -O2 -Wall -g -D_POSIX_C_SOURCE=200809
CFLAGS += -I/usr/include/bash/include -I../include -L../lib

libptef-bash.so: libptef-bash.c
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $^ -l:libptef.so.0

builtins.sh: builtins-template.sh
	sed 's|TEMPLATE_LIBDIR|$(libdir)|' $< > $@

install:
	install -d "$(DESTDIR)$(libdir)"
	install -m755 libptef-bash.so "$(DESTDIR)$(libdir)/."
	install -d "$(DESTDIR)$(datadir)/ptef"
	install -m644 builtins.sh "$(DESTDIR)$(datadir)/ptef/."

clean:
	rm -f libptef-bash.so
	rm -f builtins.sh
