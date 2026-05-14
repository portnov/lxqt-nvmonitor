#!/bin/bash
# Build lxqt-nvmonitor Debian package
#
# Usage:
#   ./debian/build-package.sh [lxqt-panel-source-dir]
#
# Example:
#   ./debian/build-package.sh /home/portnov/src/lxqt/lxqt-panel

set -euo pipefail

SCRIPT_DIR=$(dirname $(realpath $0))

LXQT_PANEL_SRC="${1:-../lxqt-panel}"

# Convert to absolute path
LXQT_PANEL_SRC="$(realpath "$LXQT_PANEL_SRC")"

if [ ! -d "$LXQT_PANEL_SRC/panel" ]; then
    echo "Error: lxqt-panel source directory not found: $LXQT_PANEL_SRC"
    echo "Usage: $0 [path-to-lxqt-panel-source]"
    exit 1
fi

echo "Building lxqt-nvmonitor package..."
echo "  lxqt-panel source: $LXQT_PANEL_SRC"

debuild -us -uc -b LXQT_PANEL_SRC="$LXQT_PANEL_SRC"
