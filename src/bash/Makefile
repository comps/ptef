include ../common.mk

.PHONY: all install clean

ifeq ($(wildcard /usr/include/bash/include),)

all:
	@echo ":::::::::::::::::::::::::::::::::::::::::::::::::::::"
	@echo ":: bash-devel not found, won't build bash builtins ::"
	@echo ":::::::::::::::::::::::::::::::::::::::::::::::::::::"

install: ;

else

all: libptef-bash.so builtins.bash

override CFLAGS += -I/usr/include/bash/include -I../include \
			-fPIC -std=c99 -D_POSIX_C_SOURCE=200809

libptef-bash.o: libptef-bash.c

libptef-bash.so: libptef-bash.o
	$(CC) -shared -L../lib $(LDFLAGS) -o $@ $^ -lptef

builtins.bash: builtins-template.bash
	sed 's|TEMPLATE_LIBDIR|$(libdir)|' $< > $@

install:
	install -d "$(DESTDIR)$(libdir)"
	install -m755 libptef-bash.so "$(DESTDIR)$(libdir)/."
	install -d "$(DESTDIR)$(datadir)/ptef"
	install -m644 builtins.bash "$(DESTDIR)$(datadir)/ptef/."

endif

clean:
	rm -f libptef-bash.o libptef-bash.so
	rm -f builtins.bash
