include ../common.mk

ASCIIDOCTOR := $(shell	x=$$(command -v asciidoctor 2>/dev/null); \
			[ "$${x::1}" = "/" ] && echo 1)

.PHONY: all install clean

ifeq ($(ASCIIDOCTOR),)

all:
	@echo "::::::::::::::::::::::::::::::::::::::::::::::::::::::"
	@echo ":: asciidoctor not found, won't build documentation ::"
	@echo "::::::::::::::::::::::::::::::::::::::::::::::::::::::"

install:
	install -d "$(DESTDIR)$(docdir)/ptef"
	install -m644 ptef.adoc "$(DESTDIR)$(docdir)/ptef/."

else

all: ptef.html

ptef.html: ptef.adoc
	asciidoctor -d article -o $@ $<

install:
	install -d "$(DESTDIR)$(docdir)/ptef"
	install -m644 ptef.adoc "$(DESTDIR)$(docdir)/ptef/."
	install -m644 ptef.html "$(DESTDIR)$(docdir)/ptef/."

endif

clean:
	rm -f ptef.html
