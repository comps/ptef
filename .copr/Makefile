.PHONY: all srpm
all: srpm

srpm:
	dnf -y install git && \
	./.copr/build_versioned_srpm.sh "$(outdir)"
