include ../common.mk

.PHONY: all install clean
all: ptef.html

ptef.html: ptef.adoc
	asciidoctor -w -d article -o $@ $<

install:
	install -d "$(DESTDIR)$(docdir)/ptef"
	install -m644 ptef.adoc "$(DESTDIR)$(docdir)/ptef/."
	install -m644 ptef.html "$(DESTDIR)$(docdir)/ptef/."

clean:
	rm -f ptef.html
