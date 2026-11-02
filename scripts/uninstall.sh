#!/usr/bin/env bash
#
# Remove a Dolphin Color Tags installation.
#
#   ./scripts/uninstall.sh                 # system-wide, /usr
#   ./scripts/uninstall.sh --prefix=/usr
#   ./scripts/uninstall.sh --user
#
# SPDX-License-Identifier: GPL-2.0-or-later

set -euo pipefail

prefix="/usr"
user_install=0

for argument in "$@"; do
    case "$argument" in
        --user)
            user_install=1
            prefix="$HOME/.local"
            ;;
        --prefix=*)
            prefix="${argument#--prefix=}"
            ;;
        -h|--help)
            sed -n '2,10p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *)
            echo "Unknown option: $argument" >&2
            exit 2
            ;;
    esac
done

run() {
    if [[ "$user_install" == 1 || "$(id -u)" == 0 ]]; then
        "$@"
    else
        sudo "$@"
    fi
}

plugin_file="$(find "$prefix" -type f -name colortagoverlay.so 2>/dev/null | head -n 1 || true)"
plugins_root=""
if [[ -n "$plugin_file" ]]; then
    plugins_root="$(dirname "$(dirname "$(dirname "$plugin_file")")")"
fi

echo "==> Removing files from $prefix"
run rm -f "$prefix/bin/colortag"
run rm -f "$prefix"/share/icons/hicolor/scalable/emblems/colortag*.svg
run rm -rf "$prefix/share/doc/dolphin-colortags"
if [[ -n "$plugins_root" ]]; then
    run rm -f "$plugins_root/kf6/overlayicon/colortagoverlay.so"
    run rm -f "$plugins_root/kf6/kfileitemaction/colortagaction.so"
    run rmdir "$plugins_root/kf6/overlayicon" 2>/dev/null || true
fi

if [[ "$user_install" == 1 ]]; then
    echo "You can remove the QT_PLUGIN_PATH entry from ~/.config/environment.d if it is no longer needed."
fi

echo "Restart Dolphin:"
echo "  killall dolphin"
echo
echo "Color tags stored in user.colortag extended attributes were left untouched."
echo "Remove them with, for example:"
echo "  find ~ -type f -exec setfattr -x user.colortag {} + 2>/dev/null"
