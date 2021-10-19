SHELL := /bin/bash

which = $(shell	x=$$(command -v $(1) 2>/dev/null); \
		[ "$${x::1}" = "/" ] && echo "$$x")

DNF := $(call which,dnf)
ifeq ($(DNF),)
DNF := $(call which,yum)
endif

PTEF_URL := https://github.com/comps/ptef/archive/refs/heads/master.tar.gz


.PHONY: all deps ptef c clean
all: deps ptef c

ifeq ($(shell id -u), 0)
deps:
	$(DNF) -y install gcc make python3 bash bash-devel curl
else
deps: ;
endif

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
