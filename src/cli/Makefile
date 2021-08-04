.PHONY: all
all: ptef-runner ptef-report ptef-mklog

CFLAGS ?= -O2 -flto -Wall -g
CFLAGS += -I../include -I../helpers

ptef-runner.o: ptef-runner.c
ptef-report.o: ptef-report.c
ptef-mklog.o: ptef-mklog.c

ptef-runner: ptef-runner.o ../helpers/*.o
	$(CC) -L../lib $(CFLAGS) -o $@ $^ -l:libptef.so.0

ptef-report: ptef-report.o
	$(CC) -L../lib $(CFLAGS) -o $@ $^ -l:libptef.so.0

ptef-mklog: ptef-mklog.o ../helpers/*.o
	$(CC) -L../lib $(CFLAGS) -o $@ $^ -l:libptef.so.0

.PHONY: clean
clean:
	rm -f ptef-runner.o ptef-report.o ptef-mklog.o
	rm -f ptef-runner ptef-report ptef-mklog
