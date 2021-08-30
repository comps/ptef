Name: ptef
Version: PTEF_VERSION
Release: PTEF_RELEASE%{?dist}
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
BuildRequires: asciidoctor
Recommends: bash
Recommends: python3

%description
TBD

%prep
%setup

%build
# we intentionally discard writev() result as it's used only for debugging,
# unfortunately, there's no __attribute__ that could create an exception,
# and (void) also doesn't silence it, so just disable it globally
CFLAGS="${RPM_OPT_FLAGS} -Wno-unused-result" make

%check
# testing is destructive since it re-builds binaries
# with different CFLAGS, so back-up original outputs
rm -rf src-backup
mv src src-backup
cp -a src-backup src
make test
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

