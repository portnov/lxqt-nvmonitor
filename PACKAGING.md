# Ubuntu/Debian Packaging

## Prerequisites

```bash
sudo apt build-dep lxqt-nvmonitor
sudo apt install debhelper cmake libkf6windowsystem-dev liblxqt2-dev qt6-base-dev qt6-base-private-dev
```

## Building the Package

The plugin requires `lxqtpanelpluginconfigdialog.h`, which is **not** installed by the `lxqt-panel` package. You must provide the lxqt-panel source tree:

```bash
# Clone lxqt-panel source (same version as installed)
git clone https://github.com/lxqt/lxqt-panel.git
cd lxqt-panel && git checkout 2.3.2  # match installed version

# Build the package
cd ../lxqt-nvmonitor
./debian/build-package.sh ../lxqt-panel
```

Or manually with `debuild`:

```bash
cd lxqt-nvmonitor
debuild -us -uc -b LXQT_PANEL_SRC=/home/portnov/src/lxqt/lxqt-panel
```

> **Important:** Pass an **absolute path** to `LXQT_PANEL_SRC`. The build script converts relative paths automatically.

## CMake Header Resolution

The `CMakeLists.txt` uses a two-step header search:

1. **System path** (`/usr/include/lxqt`) — checked first for `lxqtpanelpluginconfigdialog.h`
2. **Source tree** (`-DLXQT_PANEL_SRC_DIR=...`) — fallback when system header is missing

If neither location has the header, CMake fails with a clear error message.

## Installation Paths

CMake uses `GNUInstallDirs` for correct multiarch paths on Debian/Ubuntu:

| File | Path |
|------|------|
| `libnvmonitor.so` | `usr/lib/x86_64-linux-gnu/lxqt-panel/` |
| `nvmonitor.desktop` | `usr/share/lxqt/lxqt-panel/` |

The `debian/rules` file passes `-DCMAKE_INSTALL_LIBDIR=lib/$(DEB_HOST_GNU_TYPE)` to ensure the correct architecture-specific library directory.

## Package Dependencies

| Dependency | Purpose |
|-----------|---------|
| `lxqt-panel (>= 2.3.0)` | Panel plugin infrastructure |
| `nvidia-compute-utils` | Provides `libnvidia-ml.so` (NVML library) |
| `nvidia-driver` (suggested) | Full NVIDIA driver with GPU monitoring support |

## File Layout

```
debian/
├── build-package.sh          # Helper script for building
├── changelog                 # Package changelog
├── control                   # Package metadata and dependencies
├── copyright                 # DEP-5 copyright file
├── rules                     # debhelper build rules
├── source/
│   └── format                # 3.0 (quilt)
└── watch                     # Upstream version tracking
```

> **Note:** No `.install` file is needed — CMake handles installation directly via `install()` commands, and `override_dh_install:` in `rules` skips the redundant `dh_install` step.

## Upstream Release

To prepare for a new upstream release:

```bash
dch --newversion X.Y-Zubuntu1 "New upstream release."
```
