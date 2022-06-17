Name: ptef
Version: {{{PTEF_VERSION}}}
Release: 1%{?dist}
Summary: Portable Test Execution Framework

License: MIT
URL: https://github.com/comps/ptef
Source: ptef-{{{PTEF_VERSION}}}.tar.gz

# >1 on non-native (emulated) hw platforms in mock/copr,
# https://rpm-software-management.github.io/mock/Release-Notes-2.11
%if 0%{?_platform_multiplier} <= 1
%global running_on_native 1
%else
%global running_on_native 0
%endif

# required for python rpm macros to work
BuildRequires: python3-devel

%if %{running_on_native} && 0%{?!skip_tests:1}
BuildRequires: valgrind
BuildRequires: python3
%endif

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

%if %{running_on_native} && 0%{?!skip_tests:1}
%check
# testing is destructive since it re-builds binaries
# with several different CFLAGS, so back-up original outputs
rm -rf src-backup
mv src src-backup
cp -a src-backup src
TEST_PRINT_LOGS=1 make test \
	CFLAGS="${RPM_OPT_FLAGS} -Wno-unused-result -Wextra" \
	LDFLAGS="${RPM_LD_FLAGS}"
rm -rf src
mv src-backup src
%endif

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
