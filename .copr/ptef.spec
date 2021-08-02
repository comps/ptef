# SPEC file overview:
# https://docs.fedoraproject.org/en-US/quick-docs/creating-rpm-packages/#con_rpm-spec-file-overview
# Fedora packaging guidelines:
# https://docs.fedoraproject.org/en-US/packaging-guidelines/

Name: ptef
Version: PTEF_VERSION
Release: PTEF_RELEASE%{?dist}
Summary: POSIX-inspired Test Execution Framework

License: MIT
URL: https://github.com/comps/ptef
Source0: ptef-PTEF_VERSION.tar.gz

#BuildRequires: bash-devel
Recommends: bash
#Requires: bash
Recommends: python3
#Requires: python3

%description
TBD

%prep
%setup

%build
make

%install
install -m755 src/cli/ptef-runner "%{buildroot}%{_bindir}"/.
install -m755 src/cli/ptef-report "%{buildroot}%{_bindir}"/.
install -m755 src/cli/ptef-mklog "%{buildroot}%{_bindir}"/.
install -m755 src/lib/libptef.so "%{buildroot}%{_libdir}"/libptef.so.0
ln -s libptef.so.0 "%{buildroot}%{_libdir}"/libptef.so

# have bash-builtins.so in %{_libdir}/ptef/.
# have builtins.sh in %{_datadir}/ptef/.
#  - insert %{_libdir}/ptef/ path into builtins.sh here



%files
#%doc
#%license

%changelog

