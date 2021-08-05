include ../common.mk

.PHONY: all install clean
all: ;

install:
ifeq ($(python3_sitelib),)
	$(error python3_sitelib undefined, cannot install ptef.py)
else
	install -d "$(DESTDIR)$(python3_sitelib)"
	install -m644 ptef.py "$(DESTDIR)$(python3_sitelib)/."
endif

clean: ;
