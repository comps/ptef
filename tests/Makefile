#SUBDIRS	:= bin args basename fs_walk logging misc mklog prefix report vars
SUBDIRS	:= bin misc/open_fds

# this is only for preparing the "variants" (cli, valgrind, etc.) in bin/
# and not any unrelated helper tools, those are handled by bin/Makefile

.PHONY: all
all: subdirs bin_clean
	$(MAKE) -C ../src clean && \
	CFLAGS="-Wall -O0 -g3" $(MAKE) -C ../src && \
	cp -v ../src/cli/ptef-{runner,report,mklog} ../src/lib/libptef.so.0.0 bin/valgrind/cli/. && \
	$(MAKE) -C ../src clean && \
	$(MAKE) -C ../src && \
	cp -v ../src/cli/ptef-{runner,report,mklog} ../src/lib/libptef.so.0.0 bin/cli/. && \
	cp -v ../src/python/ptef.py bin/python/. && \
	if [ -f ../src/bash/libptef-bash.so ]; then cp -v ../src/bash/{libptef-bash.so,builtins.bash} bin/bash/.; fi

.PHONY: subdirs
subdirs:
	@for i in $(SUBDIRS); do \
		$(MAKE) -C $$i $(MAKECMDGOALS) || exit $$?; \
	done

.PHONY: run
run:
	@./run

.PHONY: bin_clean
bin_clean:
	rm -f bin/valgrind/cli/{ptef-{runner,report,mklog},libptef.so.0.0}
	rm -f bin/cli/{ptef-{runner,report,mklog},libptef.so.0.0}
	rm -f bin/{ptef-{runner,report,mklog},libptef.so.0.0}
	rm -f bin/python/ptef.py
	rm -f bin/bash/{libptef-bash.so,builtins.bash}

.PHONY: clean
clean: subdirs bin_clean
	rm -f valgrind.log results/cur-*.log
	rm -rf logs
