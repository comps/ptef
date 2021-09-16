.PHONY: all clean
all:
	curl -L -o ltp.tar.xz https://github.com/linux-test-project/ltp/releases/download/20210524/ltp-full-20210524.tar.xz && \
	tar -xvJf ltp.tar.xz && \
	cd ltp-full-* && \
	./configure --prefix="$$PWD/../ltp-installed" && \
	make -j4 && \
	make install

clean:
	rm -f ltp-full-* ltp.tar.xz ltp-installed
