include ../common.mk

.PHONY: all install clean
all: ptef.html  # TODO: manpages

ptef.html: ptef.adoc
	asciidoctor -w -d article -o $@ $<

install:
	install -d "$(DESTDIR)$(docdir)/ptef"
	install -m644 ptef.adoc "$(DESTDIR)$(docdir)/ptef/."
	install -m644 ptef.html "$(DESTDIR)$(docdir)/ptef/."
	#install -d "$(DESTDIR)$(mandir)/man2"
	#install -m644 .... "$(DESTDIR)$(mandir)/man2/."

clean:
	rm -f ptef.html
