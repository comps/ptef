.PHONY: all
all: build

bin/ttee: bin/ttee.c

.PHONY: build
build: bin/ttee bin_clean
	$(MAKE) -C ../src clean && \
	CFLAGS="-Wall -O0 -g3" $(MAKE) -C ../src && \
	cp -v ../src/cli/ptef-{runner,report,mklog} ../src/lib/*.so bin/valgrind/cli/. && \
	$(MAKE) -C ../src clean && \
	$(MAKE) -C ../src && \
	cp -v ../src/cli/ptef-{runner,report,mklog} ../src/lib/*.so bin/cli/. && \
	$(MAKE) -C bin/errno && \
	cp -v ../src/bash/bash-builtins.{so,sh} bin/bash/. && \
	cp -v ../src/python/ptef.py bin/python/.

.PHONY: run
run:
	@./run

.PHONY: bin_clean
bin_clean:
	rm -f bin/valgrind/cli/{ptef-{runner,report,mklog},libptef.so}
	rm -f bin/cli/{ptef-{runner,report,mklog},libptef.so}
	$(MAKE) -C bin/errno clean
	rm -f bin/{ptef-{runner,report,mklog},libptef.so}
	rm -f bin/bash/bash-builtins.{so,sh}
	rm -f bin/python/ptef.py

.PHONY: clean
clean: bin_clean
	rm -f valgrind.log results-{cli,valgrind,errno,strace,bash,python}.log
	rm -f bin/ttee
	rm -rf logs testdir tmpdir
