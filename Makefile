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
