.PHONY: all clean

all:
	$(MAKE) -C src

test:
	$(MAKE) -C tests build
	$(MAKE) -C tests run

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tests clean
