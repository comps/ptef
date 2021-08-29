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
Recommends: bash
Recommends: python3

%description
TBD

%prep
%setup

%build
make

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
	datadir="%{_datadir}" \
	python3_sitelib="%{python3_sitelib}"

#install -d "%{buildroot}%{_docdir}"/ptef
#install -m644 doc/blabla "%{buildroot}%{_docdir}"/ptef/.


%files
%attr(755,root,root) %{_bindir}/ptef-{runner,report,mklog}
%attr(755,root,root) %{_libdir}/libptef.so.0
%{_libdir}/libptef.so
%attr(755,root,root) %{_libdir}/libptef-bash.so
%attr(755,root,root) %dir %{_datadir}/ptef
%attr(644,root,root) %{_datadir}/ptef/builtins.sh
%pycached %{python3_sitelib}/ptef.py

#%docdir %{_docdir}/ptef

# TODO: PTEF.md itself, under %docdir ^^^^

#%license

%changelog

