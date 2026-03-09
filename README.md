# orphctl

A lightweight C++ utility for Arch Linux that automatically removes orphaned packages and cleans your AUR cache every two weeks using a systemd timer.

Orphaned packages are dependencies that were installed alongside other software but are no longer needed by anything. Over time these pile up and waste disk space. orphctl handles cleanup automatically so you never have to think about it.

---

## Installation

### From AUR
```bash
yay -S orphctl
```

### From GitHub Releases
Download the latest binary from the [releases page](https://github.com/bytasdik/orphctl/releases), then run the installer:

```bash
tar -xzvf orphctl-1.0.0-x86_64.tar.gz
chmod +x install.sh
./install.sh
```

### From Source
```bash
git clone https://github.com/bytasdik/orphctl.git
cd orphctl
chmod +x install.sh
./install.sh
```

The installer will compile the binary and place it in `/usr/local/bin/orphctl`. On first run, orphctl sets up the systemd timer automatically.

---

## Usage

```
orphctl [OPTION]
```

| Command | Description |
|---|---|
| `orphctl` | Remove orphaned packages via pacman. Sets up the systemd timer on first run. |
| `orphctl --aur` | Remove orphaned packages via pacman + clean yay cache. |
| `orphctl --stop` | Disable and remove the automatic cleanup timer. |
| `orphctl --stats` | Show timer status and time until next scheduled run. |
| `orphctl --help` | Show usage information. |

No need to prefix with `sudo` — orphctl elevates itself automatically.

---

## How It Works

orphctl runs `pacman -R $(pacman -Qdtq)` to remove all orphaned packages. With `--aur` it also runs `yay -Scc` to clean the AUR cache.

Scheduling is handled entirely by systemd, firing on the **1st and 15th of every month at 3:00 AM**. Thanks to `Persistent=true` in the timer, if your machine was off at the scheduled time it will catch up on the next boot.

The systemd service remembers which mode you used on first run — if you ran `orphctl --aur`, the timer will also run in AUR mode automatically.

---

## Uninstalling

```bash
./uninstall.sh
```

Or if you no longer have the repo:
```bash
orphctl --stop
sudo rm /usr/local/bin/orphctl
```

---

## Requirements

- Arch Linux (or any Arch-based distro with systemd)
- `g++` — for building from source
- `yay` — optional, only needed for `--aur` mode

---

## License

MIT License — see [LICENSE](LICENSE) for details.
