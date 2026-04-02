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

---

## 3. Native GUI (BobUI Qt Fork)
*(In Progress)*
To build the new premium native GUI (links against `libs/bobui`):

```bash
# Requires Qt6 and MSVC (Windows) or Clang (Linux/macOS)
cmake -S . -B build -G Ninja -DFO_BUILD_GUI=ON
cmake --build build --target fo_gui
```

---

## 4. Java CLI/GUI Deployment
To build the standalone cross-platform Java application:

```bash
cd filez-java
./gradlew :cli:fatJar
```
Then run with:
```bash
java -jar cli/build/libs/filez-all.jar
```

---

## 5. MSI Installer (Windows Only)
Requires the **WiX Toolset v3.11** or higher.

```bash
.\scripts\package_msi.bat
```
This generates an `.msi` in the root directory using settings from `wix/`.

---

## 6. AppImage (Linux) & DMG (macOS)
Scripts are provided for Unix-like systems:

```bash
# Linux
./scripts/package_appimage.sh

# macOS
./scripts/package_macos.sh
```
Ensure all dependencies are met before running these scripts.
