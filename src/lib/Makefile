include ../common.mk

.PHONY: all install clean
all: libptef.so

CFLAGS ?= -O2 -flto -Wall -g

# GNU LD
libptef.so.0: ../helpers/*.o ../runner/*.o ../report/*.o ../mklog/*.o
	$(CC) -shared -fPIC -Wl,--version-script=libptef.map $(CFLAGS) -o $@ $^

libptef.so: libptef.so.0
	ln -sf $< $@

install:
	install -d "$(DESTDIR)$(libdir)"
	install -m755 libptef.so.0 "$(DESTDIR)$(libdir)/."
	cp -df libptef.so "$(DESTDIR)$(libdir)/."

clean:
	rm -f libptef.so.0
	rm -f libptef.so
