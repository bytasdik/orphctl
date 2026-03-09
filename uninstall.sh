#!/bin/bash

# uninstall.sh — orphctl uninstaller
# Stops the systemd timer and removes all orphctl files from the system

BINARY_DEST="/usr/local/bin/orphctl"
SERVICE_FILE="/etc/systemd/system/orphctl.service"
TIMER_FILE="/etc/systemd/system/orphctl.timer"

# ── Checks ────────────────────────────────────────────────────────────────────

if [ "$EUID" -ne 0 ]; then
    echo ">>> Elevation required — relaunching with sudo..."
    exec sudo bash "$0" "$@"
fi

if [ ! -f "$BINARY_DEST" ]; then
    echo "orphctl is not installed."
    exit 0
fi

# ── Confirm ───────────────────────────────────────────────────────────────────

echo "This will remove:"
echo "  $BINARY_DEST"
echo "  $SERVICE_FILE"
echo "  $TIMER_FILE"
echo ""
read -p "Are you sure? [y/N] " confirm

if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
    echo "Aborted."
    exit 0
fi

# ── Remove systemd timer ──────────────────────────────────────────────────────

if systemctl list-unit-files orphctl.timer &>/dev/null; then
    echo ">>> Stopping and disabling orphctl timer..."
    systemctl disable --now orphctl.timer 2>/dev/null
    echo "✔ Timer stopped."
fi

# ── Remove files ──────────────────────────────────────────────────────────────

echo ">>> Removing orphctl files..."
rm -f "$BINARY_DEST" "$SERVICE_FILE" "$TIMER_FILE"
systemctl daemon-reload

echo "✔ orphctl has been fully removed."