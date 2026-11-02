#!/usr/bin/env bash
#
# Recreate the KDE Store download archive and its checksum.
#
#   ./make.sh
#
# The archive is built from the current git HEAD, so commit changes before
# releasing. packaging/kde-store itself is excluded through .gitattributes.

set -euo pipefail

here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo="$(cd "$here/../.." && pwd)"

version="$(sed -n 's/^    VERSION \([0-9][0-9.]*\)$/\1/p' "$repo/CMakeLists.txt" | head -n 1)"
if [[ -z "$version" ]]; then
    echo "Could not read the version from CMakeLists.txt" >&2
    exit 1
fi

archive="dolphin-colortags-$version.tar.gz"
mkdir -p "$here/dist"
rm -f "$here/dist/$archive" "$here/dist/SHA256SUMS"

echo "==> Creating $archive from HEAD ($(git -C "$repo" rev-parse --short HEAD))"
git -C "$repo" archive --worktree-attributes --format=tar.gz \
    --prefix="dolphin-colortags-$version/" \
    HEAD -o "$here/dist/$archive"

(
    cd "$here/dist"
    sha256sum "$archive" > SHA256SUMS
)

echo "==> Verifying archive"
tar tzf "$here/dist/$archive" >/dev/null
if tar tzf "$here/dist/$archive" | grep -q '^dolphin-colortags-'"$version"'/packaging/kde-store/'; then
    echo "Error: store kit leaked into the source archive" >&2
    exit 1
fi
if ! tar tzf "$here/dist/$archive" | grep -q '^dolphin-colortags-'"$version"'/src/colortagoverlay.cpp$'; then
    echo "Error: source files missing from the archive" >&2
    exit 1
fi

ls -lh "$here/dist/$archive"
cat "$here/dist/SHA256SUMS"
