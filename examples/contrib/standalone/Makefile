SHELL := /bin/bash
PTEF_URL := https://github.com/comps/ptef/archive/refs/heads/main.tar.gz

.PHONY: all c clean
all: ptef c

ptef:
	mkdir ptef-build && ( \
		cd ptef-build && \
		curl -kL "$(PTEF_URL)" | tar -xvz --strip-components=1 && \
		make && \
		make install DESTDIR="$(PWD)/ptef" \
	)
	rm -rf ptef-build

clean:
	$(MAKE) -C c clean
	rm -rf ptef-build ptef


c : export override CFLAGS	+= -I$(PWD)/ptef/usr/include
c : export override LDFLAGS	+= -L$(PWD)/ptef/usr/lib64

c:
	$(MAKE) -C c
