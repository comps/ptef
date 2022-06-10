.PHONY: all clean
all: ltp-installed

ltp-installed:
	curl -L -o ltp.tar.xz https://github.com/linux-test-project/ltp/releases/download/20220527/ltp-full-20220527.tar.xz && \
	tar -xvJf ltp.tar.xz && \
	cd ltp-full-* && \
	./configure --prefix="$$PWD/../ltp-installed" && \
	make -j4 && \
	make install

clean:
	rm -rf ltp-full-* ltp.tar.xz ltp-installed
