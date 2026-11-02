#!/usr/bin/env bash
#
# Build and install Dolphin Color Tags.
#
#   ./scripts/install.sh                 # system-wide, /usr (asks for sudo)
#   ./scripts/install.sh --prefix=/usr   # explicit prefix
#   ./scripts/install.sh --user          # ~/.local, needs a relogin (see below)
#
# SPDX-License-Identifier: GPL-2.0-or-later

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source_dir="$(dirname "$script_dir")"
build_dir="$source_dir/build"
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
            sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'
            exit 0
            ;;
        *)
            echo "Unknown option: $argument" >&2
            exit 2
            ;;
    esac
done

if [[ "$user_install" == 1 && "$prefix" != "$HOME/.local" ]]; then
    echo "--user and --prefix cannot be combined." >&2
    exit 2
fi

missing=()
for tool in cmake c++ ninja; do
    command -v "$tool" >/dev/null 2>&1 || missing+=("$tool")
done
if (( ${#missing[@]} > 0 )); then
    echo "Missing build tools: ${missing[*]}" >&2
    echo
    echo "On Arch / CachyOS / Manjaro:"
    echo "  sudo pacman -S --needed base-devel cmake ninja qt6-base qt6-svg kio"
    exit 1
fi

echo "==> Configuring ($prefix)"
cmake -S "$source_dir" -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$prefix"

echo "==> Building"
cmake --build "$build_dir"

can_write_prefix() {
    if [[ -d "$prefix" ]]; then
        [[ -w "$prefix" ]]
    else
        [[ -w "$(dirname "$prefix")" ]]
    fi
}

echo "==> Installing to $prefix"
if [[ "$(id -u)" == 0 ]] || can_write_prefix; then
    cmake --install "$build_dir"
else
    sudo cmake --install "$build_dir"
fi

# Work out where the plugins landed, for the diagnostics below.
plugin_file="$(find "$prefix" -type f -name colortagoverlay.so 2>/dev/null | head -n 1 || true)"
plugins_root=""
if [[ -n "$plugin_file" ]]; then
    plugins_root="$(dirname "$(dirname "$(dirname "$plugin_file")")")"
fi

echo
if [[ "$user_install" == 1 ]]; then
    echo "User-local install finished."
    echo "The Qt plugin directory is not searched by default, so add it once:"
    echo
    echo "  mkdir -p ~/.config/environment.d"
    echo "  echo 'QT_PLUGIN_PATH=$plugins_root' \\"
    echo "       > ~/.config/environment.d/90-dolphin-colortags.conf"
    echo
    echo "Then log out and back in (environment.d is read at login)."
else
    expected="$(qmake6 -query QT_INSTALL_PLUGINS 2>/dev/null || true)"
    if [[ "$prefix" == /usr || "$prefix" == /usr/local ]] \
        && [[ -n "$expected" && -n "$plugins_root" && "$plugins_root" != "$expected" ]]; then
        echo "Warning: plugins were installed to:"
        echo "  $plugins_root"
        echo "but Qt looks in:"
        echo "  $expected"
        echo "Re-run with -DCOLORTAG_PLUGIN_DIR=... if your distribution uses a different layout."
        echo
    fi
    echo "Installed."
fi

echo "Restart Dolphin to load the plugin:"
echo "  killall dolphin"
echo
echo "Then right-click a file or folder and choose \"Color Tag\"."
