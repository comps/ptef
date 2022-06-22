SHELL := /bin/bash
PTEF_URL := https://github.com/comps/ptef/archive/refs/heads/main.tar.gz

.PHONY: all clean
all: ptef

ptef:
	mkdir ptef-build && ( \
		cd ptef-build && \
		curl -kL "$(PTEF_URL)" | tar -xvz --strip-components=1 && \
		make && \
		make install DESTDIR="$(PWD)/ptef" \
	)
	rm -rf ptef-build

clean:
	rm -rf ptef-build ptef
