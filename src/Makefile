include common.mk

# the order matters here;
# - runner/report/mklog need helpers
# - lib needs runner/report/mklog
# - cli/bash/python need lib
# and rather than writing long and hacky Makefile to allow parallel build,
# just have this simple sequential list
SUBDIRS := helpers runner report mklog lib cli bash python doc

.PHONY: all install clean
all:
	@set -e; for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir; done

install:
	@set -e; for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir install; done

clean:
	@set -e; for subdir in $(SUBDIRS); do $(MAKE) -C $$subdir clean; done
