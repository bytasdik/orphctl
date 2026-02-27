#!/bin/bash

# cleanup-orphans.sh
# Removes orphaned packages on Arch Linux every 2 weeks via systemd timer.
# First run installs and enables automation automatically.

SCRIPT_DEST="/usr/local/bin/cleanup-orphans.sh"
SERVICE_FILE="/etc/systemd/system/cleanup-orphans.service"
TIMER_FILE="/etc/systemd/system/cleanup-orphans.timer"

# ── Helper functions ─────────────────────────────────────────────────────────
is_root() { [ "$EUID" -eq 0 ]; }

require_root() {
    if ! is_root; then
        echo "Error: please run as root: sudo $0 $1"
        exit 1
    fi
}

is_installed() { [ -f "$SCRIPT_DEST" ] && systemctl list-unit-files cleanup-orphans.timer &>/dev/null; }

install_automation() {
    echo ">>> First run detected — setting up automation..."

    cp "$0" "$SCRIPT_DEST"
    chmod +x "$SCRIPT_DEST"

    cat > "$SERVICE_FILE" <<UNIT
[Unit]
Description=Remove orphaned pacman packages

[Service]
Type=oneshot
ExecStart=$SCRIPT_DEST
UNIT

    cat > "$TIMER_FILE" <<UNIT
[Unit]
Description=Run orphan cleanup every 2 weeks

[Timer]
OnCalendar=*-*-1,15 03:00:00
Persistent=true

[Install]
WantedBy=timers.target
UNIT

    systemctl daemon-reload
    systemctl enable --now cleanup-orphans.timer

    echo "✔ Automation enabled. Cleanup will run on the 1st and 15th of each month at 3:00 AM."
    echo ""
}

# ── Argument handling ────────────────────────────────────────────────────────
case "$1" in
    --help|-h)
        echo "Usage: sudo cleanup-orphans.sh [OPTION]"
        echo ""
        echo "  (no args)   Remove orphaned packages. Sets up automation on first run."
        echo "  --stop      Disable and remove the automatic cleanup timer."
        echo "  --stats     Show timer status and time until next scheduled cleanup."
        echo "  --help      Show this help message."
        exit 0
        ;;

    --stop)
        require_root "--stop"
        if ! systemctl list-unit-files cleanup-orphans.timer &>/dev/null 2>&1 | grep -q cleanup-orphans.timer; then
            echo "Automation is not currently installed."
            exit 0
        fi
        echo ">>> Disabling and removing cleanup automation..."
        systemctl disable --now cleanup-orphans.timer 2>/dev/null
        rm -f "$SERVICE_FILE" "$TIMER_FILE" "$SCRIPT_DEST"
        systemctl daemon-reload
        echo "✔ Automation removed. You can delete this script manually if you wish."
        exit 0
        ;;

    --stats)
        echo "=== Cleanup Orphans — Timer Status ==="
        echo ""
        if ! systemctl is-enabled cleanup-orphans.timer &>/dev/null; then
            echo "Automation is not enabled. Run sudo $0 to set it up."
            exit 0
        fi
        systemctl status cleanup-orphans.timer --no-pager
        echo ""
        echo "--- Next scheduled run ---"
        systemctl list-timers cleanup-orphans.timer --no-pager
        exit 0
        ;;

    "")
        # Default: run cleanup
        ;;

    *)
        echo "Unknown option: $1"
        echo "Run '$0 --help' for usage."
        exit 1
        ;;
esac

# ── Default: run cleanup ─────────────────────────────────────────────────────
require_root

# Auto-setup on first run
if [ ! -f "$SCRIPT_DEST" ]; then
    install_automation
fi

echo "=== Pacman Orphan Cleanup ==="
orphans=$(pacman -Qdtq 2>/dev/null)

if [ -z "$orphans" ]; then
    echo "No orphaned packages found. Nothing to do."
else
    echo "Orphaned packages found:"
    echo "$orphans"
    echo ""
    pacman -R $orphans
    echo ""
    echo "✔ Done."
fi
