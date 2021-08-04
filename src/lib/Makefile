.PHONY: all
all: libptef.so

CFLAGS ?= -O2 -flto -Wall -g

# GNU LD
libptef.so.0: ../helpers/*.o ../runner/*.o ../report/*.o ../mklog/*.o
	$(CC) -shared -fPIC -Wl,--version-script=libptef.map $(CFLAGS) -o $@ $^

libptef.so: libptef.so.0
	ln -s $< $@

.PHONY: clean
clean:
	rm -f libptef.so.0
	rm -f libptef.so
