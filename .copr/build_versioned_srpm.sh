#!/bin/bash
set -e

version=$(git describe --tags HEAD)  # v0.7 | v0.7-113-g1a9f9a3
if [ "${version%%-*}" = "$version" ]; then
	version="$version.0"      # v0.7 -> v0.7.0
else
	version="${version%-g*}"  # v0.7-113
	version="${version#v}"    # 0.7-113
	version="${version/-/.}"  # 0.7.113
fi

rm -rf .rpmbuild
mkdir -p .rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

mkdir ".rpmbuild/ptef-$version"
cp -a * ".rpmbuild/ptef-$version/."
tar -cvzf ".rpmbuild/SOURCES/ptef-$version.tar.gz" \
	-C .rpmbuild "ptef-$version"

cp .copr/ptef.spec .rpmbuild/SPECS/.

# hardcode version/release, don't use rpm macros, they would
# be missing when this SRPM is actually built elsewhere
sed "s/PTEF_VERSION/$version/g" -i .rpmbuild/SPECS/ptef.spec
rpmbuild \
	--define "_topdir $PWD/.rpmbuild" \
	-bs .rpmbuild/SPECS/ptef.spec
[ -n "$1" ] && cp -vf .rpmbuild/SRPMS/*.src.rpm "$1"/.

exit 0
