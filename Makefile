.PHONY: all runners clean
all: runners
	install src/runner bin/tef-runner
	install src/report bin/tef-report
	install src/mklog bin/tef-mklog

runners:
	$(MAKE) -C src

clean:
	rm -f bin/tef-*
	$(MAKE) -C src clean
