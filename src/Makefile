.PHONY: all clean

all: runner report mklog

runner:
	{ echo '#!/bin/bash' && cat common.inc && cat $@.inc; } > $@
report:
	{ echo '#!/bin/bash' && cat common.inc && cat $@.inc; } > $@
mklog:
	{ echo '#!/bin/bash' && cat common.inc && cat $@.inc; } > $@

clean:
	rm -f runner report mklog
