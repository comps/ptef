include ../common.mk

.PHONY: all install clean
all: ptef-runner ptef-report ptef-mklog

CFLAGS += -I../include -I../helpers -std=c99 -D_POSIX_C_SOURCE=200809

ptef-runner.o: ptef-runner.c
ptef-report.o: ptef-report.c
ptef-mklog.o: ptef-mklog.c

ptef-runner: ptef-runner.o ../helpers/*.o
	$(CC) -L../lib $(LDFLAGS) -o $@ $^ -lptef

ptef-report: ptef-report.o ../helpers/*.o
	$(CC) -L../lib $(LDFLAGS) -o $@ $^ -lptef

ptef-mklog: ptef-mklog.o ../helpers/*.o
	$(CC) -L../lib $(LDFLAGS) -o $@ $^ -lptef

install:
	install -d "$(DESTDIR)$(bindir)"
	install -m755 ptef-runner "$(DESTDIR)$(bindir)/."
	install -m755 ptef-report "$(DESTDIR)$(bindir)/."
	install -m755 ptef-mklog "$(DESTDIR)$(bindir)/."

clean:
	rm -f ptef-runner.o ptef-report.o ptef-mklog.o
	rm -f ptef-runner ptef-report ptef-mklog
