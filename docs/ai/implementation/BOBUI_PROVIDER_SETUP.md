# BobUI Provider Setup — v6.0.6

## Summary
bobfilez should prefer **BobUI** (`github.com/robertpelloni/bobui`) as its native Qt provider rather than assuming a separately installed stock Qt distribution.

## Important architectural note
BobUI is a **Qt fork**, not a replacement UI toolkit with completely different target names.

That means the bobfilez GUI targets still correctly consume standard CMake package names such as:
- `Qt6::Core`
- `Qt6::Gui`
- `Qt6::Qml`
- `Qt6::Quick`
- `Qt6::QuickControls2`
- `Qt6::WebEngineQuick`

What changes is **where those Qt6 packages come from**.

## What was changed

### CMake bootstrap
A new helper was added:
- `cmake/BobUIQtSetup.cmake`

The root `CMakeLists.txt` now:
- adds `cmake/` to `CMAKE_MODULE_PATH`
- prefers BobUI when `FO_BUILD_GUI` or `FO_BUILD_OMNI` is enabled
- looks for BobUI through:
  - `BOBUI_ROOT`
  - `FO_BOBUI_ROOT`
  - local `libs/bobui`
- appends likely BobUI build/install prefixes to `CMAKE_PREFIX_PATH`

### GUI targets
- `gui/CMakeLists.txt`
- `gui/omni/CMakeLists.txt`

These still call `find_package(Qt6 ...)`, but now do so in a build where BobUI is the preferred provider path.

## Why this approach is correct
Trying to literally rename all `Qt6::*` references to `BobUI::*` would be incorrect unless BobUI exposes an entirely different exported target scheme.

At the moment, the right integration strategy is:
1. build/install BobUI (the Qt fork)
2. expose its generated Qt6 package config path
3. let bobfilez resolve normal `Qt6::*` targets from BobUI

## Expected usage

### Example PowerShell flow
```powershell
$env:BOBUI_ROOT = "C:\path\to\bobui-or-bobui-install"
cmake -S . -B build-gui -G Ninja -DFO_BUILD_GUI=ON -DFO_BUILD_OMNI=ON
cmake --build build-gui
```

## Current limitation
This repository now prefers BobUI correctly at the CMake level, but the machine still needs a **BobUI-built Qt6 package layout** available for `find_package(Qt6 ...)` to succeed.

If `libs/bobui` is only a source checkout and has not been configured/built/installed yet, GUI configure may still fail until the corresponding package config files exist.

## Go-port status
As of this session, there is **no active Go port** in the bobfilez repository.

Observed state:
- no top-level Go module for bobfilez
- no `filez-go/` or equivalent tracked implementation tree
- no current handoff/docs thread describing an active Go rewrite
- the maintained alternate implementation present in-tree is `filez-java/`

So the practical answer is: the Go port is not currently part of the maintained codebase, or it was dropped before the present repo state.
