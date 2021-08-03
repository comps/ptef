Name: ptef
Version: PTEF_VERSION
Release: PTEF_RELEASE%{?dist}
Summary: POSIX-inspired Test Execution Framework

License: MIT
URL: https://github.com/comps/ptef
Source0: ptef-PTEF_VERSION.tar.gz

# for tests
BuildRequires: valgrind
#%ifarch x86_64
#%endif
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
# adjust path to libptef-bash shared lib
sed 's_/usr/lib64_%{_libdir}_' -i src/bash/builtins.sh

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
install -d "%{buildroot}%{_bindir}"
install -m755 src/cli/ptef-runner "%{buildroot}%{_bindir}"/.
install -m755 src/cli/ptef-report "%{buildroot}%{_bindir}"/.
install -m755 src/cli/ptef-mklog "%{buildroot}%{_bindir}"/.
install -d "%{buildroot}%{_libdir}"
install -m755 src/lib/libptef.so.0 "%{buildroot}%{_libdir}"/.
install -m755 src/lib/libptef.so "%{buildroot}%{_libdir}"/.
install -m755 src/bash/libptef-bash.so "%{buildroot}%{_libdir}"/.
install -d "%{buildroot}%{_datadir}"/ptef
install -m644 src/bash/builtins.sh "%{buildroot}%{_datadir}"/ptef/.
install -d "%{buildroot}%{python3_sitelib}"
install -m644 src/python/ptef.py "%{buildroot}%{python3_sitelib}"/.
#install -d "%{buildroot}%{_docdir}"/ptef
#install -m644 doc/blabla "%{buildroot}%{_docdir}"/ptef/.

%files
%attr(755,root,root) %{_bindir}/ptef-{runner,report,mklog}
%attr(755,root,root) %{_libdir}/libptef.so.0
%attr(755,root,root) %{_libdir}/libptef.so
%attr(755,root,root) %{_libdir}/libptef-bash.so
%attr(755,root,root) %dir %{_datadir}/ptef
%attr(644,root,root) %{_datadir}/ptef/builtins.sh
%pycached %{python3_sitelib}/ptef.py
#%docdir %{_docdir}/ptef

# TODO: PTEF.md itself, under %docdir ^^^^

#%license

%changelog

