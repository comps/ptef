Name: ptef
Version: PTEF_VERSION
Release: 1%{?dist}
Summary: POSIX-inspired Test Execution Framework

License: MIT
URL: https://github.com/comps/ptef/
Source0: ptef-PTEF_VERSION.tar.gz

# required for python rpm macros to work
BuildRequires: python3-devel

# for tests
BuildRequires: valgrind
BuildRequires: python3

BuildRequires: gcc
BuildRequires: make
BuildRequires: bash-devel
Recommends: bash
Recommends: python3

# el8 doesn't have asciidoctor for some weird reason,
# el7 does, fedora does, so just don't build docs on 8
%if 0%{?rhel} != 8
BuildRequires: asciidoctor
%endif

%description
TBD

%prep
%setup

%build
# -Wno-unused-result
#   we intentionally discard writev() result as it's used only for debugging,
#   unfortunately, there's no __attribute__ that could create an exception,
#   and (void) also doesn't silence it, so just disable it globally
# -Wextra -Werror
#   because I want to be pedantic
CFLAGS="${RPM_OPT_FLAGS} -Wno-unused-result -Wextra -Werror" make

%check
# testing is destructive since it re-builds binaries
# with several different CFLAGS, so back-up original outputs
rm -rf src-backup
mv src src-backup
cp -a src-backup src
CFLAGS="${RPM_OPT_FLAGS} -Wno-unused-result" make test
rm -rf src
mv src-backup src

%install
%make_install \
	bindir="%{_bindir}" \
	libdir="%{_libdir}" \
	includedir="%{_includedir}" \
	datadir="%{_datadir}" \
	docdir="%{_docdir}" \
	mandir="%{_mandir}" \
	python3_sitelib="%{python3_sitelib}"

%files
%attr(755,root,root) %{_bindir}/ptef-{runner,report,mklog}
%attr(755,root,root) %{_libdir}/libptef.so.0
%{_libdir}/libptef.so
%attr(755,root,root) %{_libdir}/libptef-bash.so
%attr(644,root,root) %{_includedir}/ptef.h
%attr(755,root,root) %dir %{_datadir}/ptef
%attr(644,root,root) %{_datadir}/ptef/builtins.sh
%attr(644,root,root) %{_mandir}/man3/ptef_{runner,report,mklog}.3*
%attr(755,root,root) %dir %{_docdir}/ptef
%attr(644,root,root) %{_docdir}/ptef/ptef.{adoc,html}
%pycached %{python3_sitelib}/ptef.py

#%license

%changelog

