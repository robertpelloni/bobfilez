# filez Deployment Instructions

## 1. CLI Deployment
To build the feature-complete `fo_cli` for Windows, Linux, or macOS:

```bash
# Using CMake + Ninja
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target fo_cli
```

The resulting `fo_cli` (or `fo_cli.exe`) will be located in the `build/cli/` directory.

---

## 2. Web GUI Deployment (BobUI Web)
The web-based GUI is built with Node.js and Express.js.

```bash
cd bobui_web
npm install --silent
npm start
```
By default, the UI is served at `http://localhost:3131`.

- **Primary page (open this)**: `http://localhost:3131/unified.html` — every
  workflow on one value-ordered page (A Essentials → E Automate), each card
  with label + ⓘ tooltip, one shared activity log. Classic shell demo (`/`)
  and legacy React tabs (`/react/`) are linked from its footer only.
- **One-command start**: `bobui_web\start-server.bat` (defaults `PORT=3131`).
- **System tray**: `bobui_web\tray.bat` (or `python bobui_web\tray.py`) puts
  bobfilez in the Windows notification area — Open unified dashboard, Server
  status, Restart server, Open logs folder, **Quit server + tray** (posts
  `/api/quit`; the dashboard header has the same Quit). Needs `pystray` +
  `Pillow`; without them it falls back to a small tkinter window with the
  same actions.
- **Full dev loop**: `scripts\dev_all.bat` — headless build → `fo_tests.exe`
  → CLI smoke → dashboard wiring tests (`node --test test\` in `bobui_web`)
  → server. Run heavy builds BelowNormal on shared machines.
- **Honest-off endpoints**: `/api/vault/*` and `/api/watch/status` return
  501 by design until the `vault-cli` / queryable-watcher work lands
  (see `TODO.md`); the UI renders these as errors, never as success.

---

## 3. Native GUI (BobUI / Qt6 Framework)
*(In Progress)*
To build the premium native GUI using **BobUI** as the active Omni/UI layer plus a compatible Qt6 QML runtime:

```bash
# Preferred helper
scripts/build_bobui_gui.bat

# Example environment:
#   QT6_ROOT=/path/to/qt-msvc-kit
#   BOBUI_ROOT=/path/to/bobui-source-or-build
cmake -S . -B build -G Ninja -DFO_BUILD_GUI=ON -DFO_BUILD_OMNI=ON
cmake --build build --target fo_gui fo_omni
```

Current honest blocker on this machine:
- the local BobUI build tree still lacks `Qt6Qml`
- the discovered external desktop Qt6 QML kit under `D:\Qt` is currently `mingw_64`, not an MSVC desktop kit
- BobUI in-place MSVC builds now get much farther, but still eventually reach a later MSVC compiler-ICE boundary in snippet-related targets rather than the earlier `qlocale.cpp` stop

---

## 4. MSI Installer (Windows Only)
Requires the **WiX Toolset v3.11** or higher.

```bash
.\scripts\package_msi.bat
```
This generates an `.msi` in the root directory using settings from `wix/`.

---

## 5. AppImage (Linux) & DMG (macOS)
Scripts are provided for Unix-like systems:

```bash
# Linux
./scripts/package_appimage.sh

# macOS
./scripts/package_macos.sh
```
Ensure all dependencies are met before running these scripts.
