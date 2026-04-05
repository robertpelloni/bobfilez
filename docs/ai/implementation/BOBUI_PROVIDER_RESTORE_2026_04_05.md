# BobUI Provider Restore — 2026-04-05

## Summary
This session restored **BobUI/Qt6** as the active native provider path for bobfilez while preserving the BTK investigation as documented research.

The project had been actively wired for BTK/CopperSpice package discovery, but the user requested a return to BobUI for the active path.
The restoration was done honestly:
- active GUI/Omni build wiring now prefers BobUI again
- BobUI helper scripts were restored
- the current machine's real blocker is still documented clearly

## What changed

### 1. Restored a BobUI-specific CMake discovery helper
Added:
- `cmake/BobUIQtSetup.cmake`

Purpose:
- prefer `BOBUI_ROOT` / `FO_BOBUI_ROOT` / local `libs/bobui`
- append likely BobUI build/install prefixes to `CMAKE_PREFIX_PATH`
- treat BobUI as a **Qt6 package provider** rather than a separate target namespace

### 2. Switched the root build back to BobUI discovery for active GUI/Omni builds
Updated:
- `CMakeLists.txt`

Change:
- `FO_BUILD_GUI` / `FO_BUILD_OMNI` now include `BobUIQtSetup` instead of `BTKFrameworkSetup`
- option wording updated from BTK-backed GUI / Omni to BobUI-backed GUI / Omni

### 3. Restored active GUI target wiring to Qt6/BobUI model
Updated:
- `gui/CMakeLists.txt`
- `gui/omni/CMakeLists.txt`

Changes:
- restored `find_package(Qt6 REQUIRED COMPONENTS ...)`
- active GUI/Omni targets now link Qt6-style targets again
- BobUI `OmniUI/omnicore` source-tree inclusion was restored into the native GUI targets
- include paths for BobUI omnicore and its local deps were restored

### 4. Restored BobUI QML registration into the active bootstrap path
Updated:
- `gui/omni/src/OmniQmlRegistration.cpp`

Change:
- active registration now calls:
  - `OmniUI::registerQmlTypes();`
- then registers bobfilez-local bridge namespaces afterward:
  - `Omni.File`
  - `Omni.Viz`
  - `Omni.Native`

This makes the active bootstrap structurally BobUI-aware again.

### 5. Restored BobUI helper scripts
Added:
- `scripts/build_bobui_gui.bat`
- `scripts/build_bobui_inplace.bat`

Purpose:
- provide a repeatable BobUI-backed GUI probe path
- provide a repeatable BobUI in-place build path

## Validation performed

### Headless build
- `scripts/build_headless.bat`
- remains successful

### BobUI-backed GUI probe
Ran:
- `scripts/build_bobui_gui.bat`

Result:
- bobfilez correctly discovers BobUI's top-level `Qt6Config.cmake`
- configure then fails at the real current BobUI provider boundary:
  - missing `Qt6Qml`

Observed failure shape:
- found package config:
  - `libs/bobui/build-bobui/lib/cmake/Qt6/Qt6Config.cmake`
- but package `Qt6` reports failure because required component `Qml` is missing
- missing expected config file:
  - `libs/bobui/build-bobui/lib/cmake/Qt6Qml/Qt6QmlConfig.cmake`

### BobUI in-place build script
Ran:
- `scripts/build_bobui_inplace.bat`

Observation:
- the existing BobUI tree on this host is still sensitive to its own feature/configuration expectations
- the script now provides a repeatable in-place developer build probe instead of assuming an already-good export tree
- the current in-place build does **not** finish successfully on this machine; it now stops in BobUI's own corelib during `qlocale.cpp` compilation with a `qtmochelpers.h` failure (`'result': undeclared identifier`)
- this means the present BobUI state on this host has **two honest boundaries**:
  1. the reusable build tree currently exposed to bobfilez still lacks `Qt6Qml`
  2. a fresh BobUI in-place build currently fails upstream in BobUI corelib before becoming a fully rebuilt provider surface

## Practical conclusion
The active native provider path is now honestly BobUI/Qt6 again.

However, the machine-specific blockers are now:
- **the currently reusable BobUI build tree lacks `Qt6Qml`**
- **a fresh BobUI in-place build currently fails upstream in `qtmochelpers.h` / `qlocale.cpp`**

So the current project state is:
- BobUI is the active native provider strategy again
- BTK remains documented research and fallback investigation history
- the next BobUI-side work would be either:
  - fix BobUI's current in-place corelib build failure on this host
  - expand BobUI's module surface to include `Qt6Qml` / `Qt6Quick`
  - or point bobfilez at a BobUI/Qt6 prefix that already exports those components
