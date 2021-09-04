include ../common.mk

.PHONY: all install clean

ifeq ($(wildcard /usr/include/bash/include),)

all:
	@echo ":::::::::::::::::::::::::::::::::::::::::::::::::::::"
	@echo ":: bash-devel not found, won't build bash builtins ::"
	@echo ":::::::::::::::::::::::::::::::::::::::::::::::::::::"

install: ;

else

all: libptef-bash.so builtins.sh

CFLAGS += -I/usr/include/bash/include -I../include -L../lib
CFLAGS += -std=c99 -D_POSIX_C_SOURCE=200809

libptef-bash.so: libptef-bash.c
	$(CC) -shared -fPIC $(CFLAGS) -o $@ $^ -l:libptef.so.0

builtins.sh: builtins-template.sh
	sed 's|TEMPLATE_LIBDIR|$(libdir)|' $< > $@

install:
	install -d "$(DESTDIR)$(libdir)"
	install -m755 libptef-bash.so "$(DESTDIR)$(libdir)/."
	install -d "$(DESTDIR)$(datadir)/ptef"
	install -m644 builtins.sh "$(DESTDIR)$(datadir)/ptef/."

endif

clean:
	rm -f libptef-bash.so
	rm -f builtins.sh
