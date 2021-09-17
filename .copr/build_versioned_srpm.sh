#!/bin/bash
set -e

function fatal {
	echo "error: $@" >&2
	exit 1
}

function ptef_version {
	local rev="${1:-HEAD}"
	local version=$(git describe --long --tags "$rev")  # v0.7-113-g1a9f9a3
	version="${version#v}"     # 0.7-113-g1a9f9a3
	version="${version//-/.}"  # 0.7.113.g1a9f9a3
	echo -n "$version"
}

function ptef_changelog {
	local rev= entry= ver=
	for rev in $(git rev-list HEAD); do
		# use committer date, not author date, as commits may be
		# out-of-order after a rebase and rpmbuild then throws an error
		# noting: %changelog not in descending chronological order
		entry=$(LANG=C git log -1 \
			--format='* %cd %an <%ae> - %%%%%%VER%%%%%%%n- %s' \
			--date=format:'%a %b %d %Y' \
			"$rev")
		ver=$(ptef_version "$rev")
		ver="$ver-1"  # release is always 1 here
		entry="${entry/\%%%VER%%%/$ver}"
		printf "%s\n\n" "$entry"
	done
}


[ "$#" -gt 0 ] || fatal "usage: $0 <specfile> [destdir]"

specfile="$1"
[ -n "$specfile" ] || fatal "missing specfile arg"
shift

version=$(ptef_version)

# everything below is relative to git root, not to .copr/

rm -rf .rpmbuild
mkdir -p .rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}

mkdir ".rpmbuild/ptef-$version"
cp -a * ".rpmbuild/ptef-$version/."
tar -cvzf ".rpmbuild/SOURCES/ptef-$version.tar.gz" \
	-C .rpmbuild "ptef-$version"

# hardcode version/release, don't use rpm macros, they would
# be missing when this SRPM is actually built elsewhere
content=$(<"$specfile")
content="${content//\{\{\{PTEF_VERSION\}\}\}/$version}"
changelog=$(ptef_changelog)
content="${content/\{\{\{PTEF_CHANGELOG\}\}\}/$changelog}"
printf "%s" "$content" > .rpmbuild/SPECS/ptef.spec

rpmbuild \
	--define "_topdir $PWD/.rpmbuild" \
	-bs .rpmbuild/SPECS/ptef.spec

destdir="$1"
[ -n "$destdir" ] && cp -vf .rpmbuild/SRPMS/*.src.rpm "$destdir"/.

exit 0
