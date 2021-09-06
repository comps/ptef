include ../common.mk

ifeq ($(python3_sitelib),)
python3_sitelib := $(shell python3 -c "import sysconfig; print(sysconfig.get_path('purelib'))")
$(warning python3_sitelib undefined, guessed site-packages as: $(python3_sitelib))
endif

.PHONY: all install clean
all: ;

install:
	install -d "$(DESTDIR)$(python3_sitelib)"
	install -m644 ptef.py "$(DESTDIR)$(python3_sitelib)/."

clean: ;
