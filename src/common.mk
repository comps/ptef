SHELL		:= /bin/bash

MACHINE		:= $(shell uname -m)

CFLAGS		?= -O2 -Wall -Wextra -g

# https://www.gnu.org/prep/standards/html_node/Directory-Variables.html
# also similar to what RPM uses
prefix		?= /usr
exec_prefix	?= $(prefix)
bindir		?= $(exec_prefix)/bin
sbindir		?= $(exec_prefix)/sbin
includedir	?= $(prefix)/include

ifneq ($(filter $(MACHINE),i386 i686 ppc s390),)
libdir		?= $(exec_prefix)/lib
else
libdir		?= $(exec_prefix)/lib64
endif

libexecdir	?= $(exec_prefix)/libexec
datarootdir	?= $(prefix)/share
datadir		?= $(datarootdir)
mandir		?= $(datadir)/man
docdir		?= $(datadir)/doc
sysconfdir	?= /etc
localstatedir	?= /var
sharedstatedir	?= /var/lib
