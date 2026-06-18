# Arch Linux Install Assistant

A terminal-based installation assistant for Arch Linux, written in C. This tool walks you through a full Arch Linux installation interactively — from disk partitioning to a fully configured desktop environment.

---

## Features

- Automatic UEFI/BIOS detection
- Automatic or manual disk partitioning
- Base system installation via `pacstrap`
- System configuration (locale, hostname, timezone)
- CPU microcode installation (AMD & Intel)
- GRUB bootloader installation with optional dual-boot support
- User and sudo management
- Display server selection (Wayland, Xorg, or both)
- Desktop environment selection with display manager auto-configuration

---

## Supported Desktop Environments

| Desktop Environment | Display Manager |
|---|---|
| KDE Plasma | SDDM |
| GNOME | GDM |
| Cinnamon | LightDM |
| XFCE | LightDM |
| LXQt | SDDM |
| MATE | LightDM |

---

## Requirements

- Must be run from an **Arch Linux live ISO** environment
- GCC (available in the live environment)
- Internet connection

---

## Setup

Clone the repository:

```bash
git clone https://github.com/rgeorgen10/Arch-Linux-Install-Assistant
cd Arch-Linux-Install-Assistant
./main-script
```

---

## Usage

Run the pre-compiled binary as root:

```bash
./main-script
```

The assistant will guide you through each step interactively.

### Installation Steps

1. **Disk Partitioning** — Choose automatic partitioning (recommended) or open `fdisk` manually.
   - Automatic mode creates a boot partition (UEFI only), swap, and root partition.
   - Manual mode lets you configure custom partition layouts, including a separate home partition.
2. **Base System** — Installs the base system using `pacstrap`.
3. **System Configuration** — Sets locale, timezone, hostname, and generates `fstab`.
4. **CPU Microcode** — Detects AMD or Intel CPU and installs the appropriate microcode package.
5. **Bootloader** — Installs GRUB with optional `os-prober` for dual-boot setups.
6. **Networking** — Installs and enables NetworkManager.
7. **Users & Sudo** — Optionally installs `sudo` and creates one or more users with configurable permissions.
8. **Desktop Setup** — Choose a display server and desktop environment to install.

---

## Automatic Partition Layout

### UEFI Systems
| Partition | Size | Format | Mount |
|---|---|---|---|
| Boot | 1 GB | FAT32 | `/boot` |
| Swap | System RAM Size | swap | — |
| Root | Remaining | ext4 | `/mnt` |

### BIOS/Legacy Systems
| Partition | Size | Format | Mount |
|---|---|---|---|
| Swap | System RAM Size | swap | — |
| Root | Remaining | ext4 | `/mnt` |

---

## Known Limitations

- YAY (AUR package manager) installation is not yet implemented.
- Only ext4 is supported as the root filesystem.

---

## Project Structure

```
arch-install-assistant/
├── main-script.c       # Main installation logic
├── helper-functions.h  # Utility functions (disk checks, CPU detection, formatting)
└── README.md
```

---

## License

This project is open source. See `LICENSE` for details.
