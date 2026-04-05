# BobUI Provider Restore — 2026-04-05

## Summary
This session restored **BobUI/Qt6** as the active native path for bobfilez while preserving the BTK investigation as documented research.

> Follow-up note: the later reality check in `BOBUI_QT_RUNTIME_REALITY_CHECK_2026_04_05.md` refines one important nuance: BobUI remains the active Omni/UI direction, but the local BobUI tree should not be over-assumed to be a complete self-contained Qt6 QML runtime provider on this host.

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
- restore BobUI-oriented Qt discovery rather than the BTK/CopperSpice path
- keep bobfilez consuming standard `Qt6::*` targets

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
The active native direction is BobUI again, but the later follow-up work refined the blocker more precisely:
- the local BobUI build tree still lacks `Qt6Qml`
- the old `qtmochelpers.h` / `qlocale.cpp` stop was real, but it is no longer the leading blocker after the later fix
- the next meaningful host-side requirement is a compatible Qt6 QML runtime/toolchain pairing for bobfilez's MSVC-native lane

So this restore document remains the right description of the app-side switch back to BobUI, while the newer runtime-reality document captures the more precise provider/toolchain situation discovered afterward.
