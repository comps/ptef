include ../common.mk

.PHONY: all install clean
all: ;

install:
	install -d "$(DESTDIR)$(mandir)/man3"
	install -m644 ptef_runner.3 "$(DESTDIR)$(mandir)/man3/."
	install -m644 ptef_report.3 "$(DESTDIR)$(mandir)/man3/."
	install -m644 ptef_mklog.3 "$(DESTDIR)$(mandir)/man3/."

clean: ;
