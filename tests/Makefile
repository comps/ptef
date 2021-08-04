.PHONY: all
all: build

bin/ttee: bin/ttee.c

.PHONY: build
build: bin/ttee bin_clean
	$(MAKE) -C ../src clean && \
	CFLAGS="-Wall -O0 -g3" $(MAKE) -C ../src && \
	cp -v ../src/cli/ptef-{runner,report,mklog} ../src/lib/libptef.so.0 bin/valgrind/cli/. && \
	$(MAKE) -C ../src clean && \
	$(MAKE) -C ../src && \
	cp -v ../src/cli/ptef-{runner,report,mklog} ../src/lib/libptef.so.0 bin/cli/. && \
	cp -v ../src/bash/{libptef-bash.so,builtins.sh} bin/bash/. && \
	cp -v ../src/python/ptef.py bin/python/.

.PHONY: run
run:
	@./run

.PHONY: bin_clean
bin_clean:
	rm -f bin/valgrind/cli/{ptef-{runner,report,mklog},libptef.so.0}
	rm -f bin/cli/{ptef-{runner,report,mklog},libptef.so.0}
	rm -f bin/{ptef-{runner,report,mklog},libptef.so.0}
	rm -f bin/bash/{libptef-bash.so,builtins.sh}
	rm -f bin/python/ptef.py

.PHONY: clean
clean: bin_clean
	rm -f valgrind.log results-{cli,valgrind,strace,bash,python}.log
	rm -f bin/ttee
	rm -rf logs testdir tmpdir
