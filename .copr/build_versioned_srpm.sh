#!/bin/bash
set -e

full=$(git describe --tags HEAD)
tag=$(git describe --tags --abbrev=0 HEAD)
release=${full##$tag-}
release=$(grep -o '^[0-9]\+' <<<"$release")
version=$(sed 's/^v//' <<<"$tag")

rm -rf .rpmbuild
mkdir -p .rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
tar --transform "s/^/ptef-${version}\//" \
	-cvzf ".rpmbuild/SOURCES/ptef-${version}.tar.gz" *
cp .copr/ptef.spec .rpmbuild/SPECS/.
# hardcode version/release, don't use rpm macros, they would
# be missing when this SRPM is actually built elsewhere
sed "s/PTEF_VERSION/$version/g;s/PTEF_RELEASE/$release/g" \
	-i .rpmbuild/SPECS/ptef.spec
rpmbuild \
	--define "_topdir $PWD/.rpmbuild" \
	-bs .rpmbuild/SPECS/ptef.spec
[ -n "$1" ] && cp -vf .rpmbuild/SRPMS/*.src.rpm "$1"/.

exit 0
