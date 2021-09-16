.PHONY: all clean
all: hello world
	$(MAKE) -C subdir1

hello:
	echo -e '#!/bin/bash -x\necho running as $$0 >&2\nexit 0' > $@
	chmod +x $@

world: hello
	cp $< $@

clean:
	rm -f hello world
	$(MAKE) -C subdir1 $@
