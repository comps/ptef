SHELL := /bin/bash

which = $(shell	x=$$(command -v $(1) 2>/dev/null); \
		[ "$${x::1}" = "/" ] && echo "$$x")

DNF := $(call which,dnf)
ifeq ($(DNF),)
DNF := $(call which,yum)
endif

.PHONY: all srpm
all: srpm

srpm:
	$(DNF) -y install git-core && \
	./.copr/build_versioned_srpm.sh "$(spec)" "$(outdir)"
