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

### Preferred helper script
```powershell
scripts\build_bobui_gui.bat
```

You can also override the default BobUI path:
```powershell
$env:BOBUI_ROOT = "C:\path\to\bobui-or-bobui-install"
scripts\build_bobui_gui.bat build-bobui
```

### Manual PowerShell flow
```powershell
$env:BOBUI_ROOT = "C:\path\to\bobui-or-bobui-install"
cmake -S . -B build-gui -G Ninja -DFO_BUILD_GUI=ON -DFO_BUILD_OMNI=ON
cmake --build build-gui
```

## Current limitation
This repository now prefers BobUI correctly at the CMake level, but the machine still needs a **BobUI-built Qt6 package layout** available for `find_package(Qt6 ...)` to succeed.

Observed probe result with a raw `libs/bobui` checkout:
- BobUI source tree is detected correctly
- configure still fails at `find_package(Qt6 ...)`
- exact blocker: missing `Qt6Config.cmake` / `qt6-config.cmake`

If `libs/bobui` is only a source checkout and has not been configured/built/installed yet, GUI configure will still fail until the corresponding package config files exist.

## In-place BobUI build attempt
An in-place BobUI developer build was attempted using:
- `scripts/build_bobui_inplace.bat`

### Result
The BobUI configure step **did succeed** in `libs/bobui/build-bobui/` after bypassing the repo's MSVC minimum-version gate with:
- `-DQT_NO_MSVC_MIN_VERSION_CHECK=ON`

That configure generated:
- `libs/bobui/build-bobui/lib/cmake/Qt6/Qt6Config.cmake`

### What worked
Bobfilez could then finally resolve the top-level Qt package config from the BobUI build tree.

### What failed next
A follow-up bobfilez consumer probe failed because BobUI's build tree does **not** provide the components bobfilez currently requests:
- `Qt6Qml`
- therefore also no usable `Qt6Quick`
- and by implication no path to `Qt6QuickControls2` / `Qt6WebEngineQuick` from the current BobUI tree

Exact next blocker observed:
- missing `Qt6QmlConfig.cmake`

## Practical conclusion
BobUI can be made to work as a **QtBase-class provider**, but in its current checked-out state it is **not yet a full drop-in replacement for the bobfilez GUI stack**.

Why:
- bobfilez native UI currently depends on `Qml`, `Quick`, `QuickControls2`, and `WebEngineQuick`
- the current BobUI repository appears to provide a QtBase-derived foundation plus OmniUI, but **not** the full declarative/web module set expected by bobfilez's current CMake files

## What would make it work
One of these paths is required:
1. **Expand BobUI** so it also exports the missing Qt declarative/web components.
2. **Refactor bobfilez GUI targets away from QML/Quick/WebEngine** and onto BobUI/OmniUI-only C++ surfaces.
3. **Use BobUI only for the subset it actually provides** while retaining another provider for the missing declarative modules (messier hybrid path).

## Go-port status
As of this session, there is **no active Go port** in the bobfilez repository.

Observed state:
- no top-level Go module for bobfilez
- no `filez-go/` or equivalent tracked implementation tree
- no current handoff/docs thread describing an active Go rewrite
- no alternate port is currently maintained in-tree after removal of the old Java experiment

So the practical answer is: the Go port is not currently part of the maintained codebase, or it was dropped before the present repo state.
