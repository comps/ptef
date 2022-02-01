include ../common.mk

.PHONY: all install clean
all: libptef.so

# GNU LD
libptef.so.0.0: ../helpers/*.o ../runner/*.o ../report/*.o ../mklog/*.o
	$(CC) -shared -fPIC \
		-Wl,-soname,$@ \
		$(LDFLAGS) -o $@ $^

libptef.so.0: libptef.so.0.0
	ln -sf $< $@

libptef.so: libptef.so.0
	ln -sf $< $@

install:
	install -d "$(DESTDIR)$(libdir)"
	install -m755 libptef.so.0.0 "$(DESTDIR)$(libdir)/."
	cp -df libptef.so.0 libptef.so "$(DESTDIR)$(libdir)/."

clean:
	rm -f libptef.so.0.0 libptef.so.0 libptef.so
