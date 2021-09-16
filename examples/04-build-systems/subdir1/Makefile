.PHONY: all clean
all: testing

testing:
	echo -e '#!/bin/bash -x\necho running as $$0 >&2\nexit 0' > $@
	chmod +x $@

clean:
	rm -f testing
