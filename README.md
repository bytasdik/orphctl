# cleanup-orphans

A simple Bash script for Arch Linux that automatically removes orphaned `pacman` packages every two weeks using a systemd timer.

Orphaned packages are dependencies that were installed alongside other software but are no longer needed by anything. Over time these pile up and waste disk space. This script handles cleanup automatically so you never have to think about it.

---

## Installation

Download the script, make it executable, and run it once as root. That's it — it installs itself and sets up the systemd timer automatically on first run.

```bash
chmod +x cleanup-orphans.sh
sudo bash cleanup-orphans.sh
```

On first run the script will:
- Copy itself to `/usr/local/bin/cleanup-orphans.sh`
- Create a systemd service and timer
- Enable and start the timer
- Run the orphan cleanup immediately

After that, cleanup runs automatically on the **1st and 15th of every month at 3:00 AM**.

---

## Usage

```
sudo cleanup-orphans.sh [OPTION]
```

| Command | Description |
|---|---|
| `sudo cleanup-orphans.sh` | Run cleanup now. Sets up automation on first run. |
| `sudo cleanup-orphans.sh --stop` | Disable and remove the automatic cleanup timer. |
| `sudo cleanup-orphans.sh --stats` | Show timer status and time until next scheduled run. |
| `cleanup-orphans.sh --help` | Show usage information. |

### Examples

Check when the next cleanup is scheduled:
```bash
sudo cleanup-orphans.sh --stats
```

Disable automatic cleanup entirely:
```bash
sudo cleanup-orphans.sh --stop
```

---

## How It Works

The script runs `pacman -R $(pacman -Qdtq)` which:
1. Uses `pacman -Qdtq` to list all orphaned packages quietly
2. Passes them to `pacman -R` to remove them

Scheduling is handled entirely by **systemd**, so it respects your system even if it was powered off at the scheduled time — it will catch up on the next boot thanks to `Persistent=true` in the timer.

---

## Uninstalling

```bash
sudo cleanup-orphans.sh --stop
sudo rm /usr/local/bin/cleanup-orphans.sh
```

---

## Requirements

- Arch Linux (or any Arch-based distro)
- systemd
- Root / sudo access

---

## License

Do whatever you want with it.
