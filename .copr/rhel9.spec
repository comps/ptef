Name: ptef
Version: {{{PTEF_VERSION}}}
Release: 1%{?dist}
Summary: Portable Test Execution Framework

License: MIT
URL: https://github.com/comps/ptef
Source: ptef-{{{PTEF_VERSION}}}.tar.gz

# required for python rpm macros to work
BuildRequires: python3-devel

# for tests - disabled, see %%check below
#BuildRequires: valgrind
#BuildRequires: python3

BuildRequires: gcc
BuildRequires: make
BuildRequires: bash-devel

Recommends: bash
Recommends: python3

%description
A simple (KISS) specification and an example implementation of a test "runner"
framework for system testing and/or integration and execution of test suites.

The "portable" refers to its inspiration from POSIX concepts and C API - the
specification can be implemented purely using POSIX.1-2008 and the reference
implementation does so (no GNU extensions).

%changelog
{{{PTEF_CHANGELOG}}}

%prep
%setup

%build
# -Wno-unused-result
#   we intentionally discard writev() result as it's used only for debugging,
#   unfortunately, there's no __attribute__ that could create an exception,
#   and (void) also doesn't silence it, so just disable it globally
make \
	CFLAGS="${RPM_OPT_FLAGS} -Wno-unused-result -Wextra" \
	LDFLAGS="${RPM_LD_FLAGS}"

# disable completely on copr, which builds non-native arches in chroot,
# failing to actually run non-native binaries
#%%check
## testing is destructive since it re-builds binaries
## with several different CFLAGS, so back-up original outputs
#rm -rf src-backup
#mv src src-backup
#cp -a src-backup src
#CFLAGS="${RPM_OPT_FLAGS} -Wno-unused-result" make test
#rm -rf src
#mv src-backup src

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
%license LICENSE
%attr(755,root,root) %{_bindir}/ptef-runner
%attr(755,root,root) %{_bindir}/ptef-report
%attr(755,root,root) %{_bindir}/ptef-mklog
%attr(755,root,root) %{_libdir}/libptef.so.0.0
%{_libdir}/libptef.so.0
%{_libdir}/libptef.so
%attr(644,root,root) %{_includedir}/ptef.h
%attr(755,root,root) %{_libdir}/libptef-bash.so
%attr(755,root,root) %dir %{_datadir}/ptef
%attr(644,root,root) %{_datadir}/ptef/builtins.bash
%attr(644,root,root) %{_mandir}/man3/ptef_runner.3*
%attr(644,root,root) %{_mandir}/man3/ptef_report.3*
%attr(644,root,root) %{_mandir}/man3/ptef_mklog.3*
%attr(755,root,root) %dir %{_docdir}/ptef
%attr(644,root,root) %{_docdir}/ptef/ptef.adoc
%pycached %{python3_sitelib}/ptef.py
