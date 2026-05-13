# NVIDIA GPU Monitor Plugin for LXQt Panel

An LXQt panel plugin that displays NVIDIA GPU metrics as a scrolling graph:
- **GPU Utilization** — GPU processing unit usage percentage
- **VRAM Load** — memory bandwidth utilization percentage
- **VRAM Usage Percent** — VRAM capacity used (used/total)
- **GPU Temperature** — current GPU temperature

Data is sourced from the NVML library (`libnvidia-ml.so`), similar to [btop](https://github.com/aristocratos/btop).

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/lxqt
make -j$(nproc)
```

## Installation

### Method 1: CMake install

```bash
sudo cmake --install .
```

### Method 2: Manual installation

```bash
# Copy the .so file to the plugins directory
sudo cp libnvmonitor.so /usr/lib/lxqt-panel/

# Copy the .desktop file
sudo cp nvmonitor.desktop /usr/share/lxqt/lxqt-panel/
```

> **Note:** Paths may vary by distribution. On Debian/Ubuntu the plugin directory is typically
> `/usr/lib/x86_64-linux-gnu/lxqt-panel/` and the desktop file goes to
> `/usr/share/lxqt/lxqt-panel/`.

### Method 3: Environment variable

```bash
# Add the plugin directory to LXQT_PANEL_PLUGINS_DIR
export LXQT_PANEL_PLUGINS_DIR="/path/to/plugin/directory"
lxqt-panel
```

### Method 4: User directory

Place the `.desktop` file in:
```
~/.local/share/lxqt/lxqt-panel/nvmonitor.desktop
```

Place the `.so` file in one of the directories listed in `LXQT_PANEL_PLUGINS_DIR`.

## Configuration

After adding the plugin to the panel, right-click on it and select "Configure". The following options are available:

### Metric
- **GPU Utilization** — shows GPU usage percentage
- **VRAM Load** — shows VRAM bandwidth utilization percentage
- **VRAM Usage Percent** — shows VRAM capacity used (used/total)
- **Temperature** — shows GPU temperature in degrees Celsius

### Display
- **Update interval** — data refresh interval (seconds)
- **Minimal size** — minimum widget size (pixels)
- **History length** — number of data points in the graph history
- **Grid lines** — number of horizontal grid lines
- **Show value on graph** — display current value on the graph

### Title
- **Label** — title text (empty = no title)

### Colors
- **Use theme colors** — use theme colors
- **Graph color** — graph line color
- **Grid color** — grid line color
- **Title color** — title text color

## Requirements

- LXQt 2.4+
- Qt 6
- NVIDIA GPU with proprietary drivers installed (for NVML support)
- `libnvidia-ml.so` library (usually installed with NVIDIA drivers)

## Build Dependencies

- CMake 3.16+
- Qt 6 (Widgets)
- lxqt (liblxqt)
- KF6::WindowSystem
- lxqt-panel headers (from source tree or installed)

## Project Structure

```
nvmonitor/
├── CMakeLists.txt                    # Build script
├── README.md                         # This file
├── nvmonitorplugin.h                 # Plugin + library class
├── nvmonitorplugin.cpp               # Plugin implementation
├── nvmonitor.h                       # Graph widget + NVML wrapper
├── nvmonitor.cpp                     # Graph and NVML implementation
├── nvmonitorconfiguration.h          # Configuration dialog
├── nvmonitorconfiguration.cpp        # Configuration dialog implementation
├── nvmonitorconfiguration.ui         # Configuration dialog UI
├── resources/
│   └── nvmonitor.desktop.in          # Plugin description file
├── translations/
│   └── nvmonitor.ts                  # Translation file (optional)
└── tests/
    ├── CMakeLists.txt                # Test build config
    ├── test_nvml.cpp                 # NVML library tests
    ├── test_settings.cpp             # Settings save/load tests
    └── test_widget.cpp               # Widget rendering tests
```

## Testing

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/lxqt -DBUILD_TESTS=ON
make -j$(nproc)
cd tests && ctest --verbose
```

Tests are automatically skipped on systems without NVIDIA GPU (marked as `SKIP`).

## License

GNU Lesser General Public License v2.1 or later (LGPL2+)
