#!/bin/bash

# install.sh — orphctl installer
# Compiles orphctl from source and installs it to /usr/local/bin

BINARY_DEST="/usr/local/bin/orphctl"
SOURCE="orphctl.cpp"

# ── Checks ────────────────────────────────────────────────────────────────────

if [ "$EUID" -ne 0 ]; then
    echo ">>> Elevation required — relaunching with sudo..."
    exec sudo bash "$0" "$@"
fi

if [ ! -f "$SOURCE" ]; then
    echo "Error: $SOURCE not found. Run this script from the orphctl source directory."
    exit 1
fi

if ! command -v g++ &>/dev/null; then
    echo "Error: g++ not found. Install it with: sudo pacman -S gcc"
    exit 1
fi

# ── Compile ───────────────────────────────────────────────────────────────────

echo ">>> Compiling orphctl..."
g++ -O2 -o orphctl "$SOURCE"

if [ $? -ne 0 ]; then
    echo "Error: compilation failed."
    exit 1
fi

echo "✔ Compiled successfully."

# ── Install ───────────────────────────────────────────────────────────────────

echo ">>> Installing to $BINARY_DEST..."
cp orphctl "$BINARY_DEST"
chmod +x "$BINARY_DEST"

echo "✔ Installed. You can now run: orphctl"
echo "   Run 'orphctl --help' for usage."