.PHONY: all test install clean

all:
	$(MAKE) -C src

test:
	$(MAKE) -C tests build
	$(MAKE) -C tests run

install:
	$(MAKE) -C src install

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tests clean

.PHONY: depinstall

which = $(shell	x=$$(command -v $(1) 2>/dev/null); \
		[ "$${x::1}" = "/" ] && echo "$$x")

TOOL := $(call which,dnf)
ifeq ($(TOOL),)
TOOL := $(call which,yum)
endif

depinstall:
	$(TOOL) -y install gcc make valgrind python3 bash bash-devel asciidoctor
