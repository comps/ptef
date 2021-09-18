include ../common.mk

.PHONY: all install clean

all: ;

clean: ;

define python_install_steps
	install -d "$(DESTDIR)$(python3_sitelib)"
	install -m644 ptef.py "$(DESTDIR)$(python3_sitelib)/."
endef

# if empty, try autodetecting python3_sitelib
ifeq ($(python3_sitelib),)

python3_sitelib := $(shell python3 -c "import sysconfig; print(sysconfig.get_path('purelib'))")

# if autodetection failed
ifeq ($(python3_sitelib),)

install:
	@echo ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
	@echo ":: python3 not found or too old, won't install ptef.py ::"
	@echo ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::"

else

install:
	@echo ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
	@echo ":: python3_sitelib undefined, guessed site-packages as:"
	@echo ":: $(python3_sitelib)"
	@echo ":::::::::::::::::::::::::::::::::::::::::::::::::::::::::"
	$(python_install_steps)

endif

# python3_sitelib defined externally
else

install:
	$(python_install_steps)

endif
