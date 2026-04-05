# Frontend Parity Expansion & Pure Qt Lane Decoupling — 2026-04-05

## Summary

This session continued the multi-frontend strategy after the initial React + JUCE functional milestone.

Two themes drove the work:

1. **Expand functional parity beyond scan + duplicates** so the newer frontends exercise more of `fo_core`.
2. **Make the pure Qt demo lane honest** by separating it from BobUI package discovery, instead of accidentally resolving Qt through BobUI-specific hints.

## What Changed

### 1. Qt demo expanded from 3 tabs to 5 tabs

Updated `frontends/qt/src/main.cpp` so `fo_qt_demo` now includes:

- **Dashboard**
- **Scanner**
- **Duplicates**
- **Statistics**
- **Hasher**

The implementation follows the same practical pattern already proven in JUCE:

- use the real `fo::core::Registry<fo::core::IFileScanner>` for scans and statistics
- use the real `fo::core::Engine` for duplicate discovery
- use the real `fo::core::Registry<fo::core::IHasher>` for hashing
- perform heavy work on background `std::thread`s
- marshal UI updates back to the Qt event loop with `QTimer::singleShot(..., this, ...)`

This keeps the demo useful without trying to prematurely convert it into a larger application framework.

### 2. BobUI demo expanded to the same parity tier

Updated:

- `frontends/bobui/src/QmlEngineWrapper.hpp`
- `frontends/bobui/src/QmlEngineWrapper.cpp`
- `frontends/bobui/assets/Main.qml`

The BobUI/QML lane now exposes four real backend actions through a single QObject bridge:

- `runScan(...)`
- `runDuplicates(...)`
- `runStats(...)`
- `runHash(...)`

And emits corresponding result signals:

- `scanFinished(...)`
- `duplicatesFinished(...)`
- `statsFinished(...)`
- `hashFinished(...)`
- scoped `errorOccurred(scope, message)`

The QML surface now includes:

- **Dashboard**
- **Scanner**
- **Duplicates**
- **Statistics**
- **Hasher**

This is intentionally still a lightweight demo UI, but it now proves that the BobUI lane can drive the same backend workflows as the Qt and JUCE demos once a compatible Qt runtime is available.

### 3. React web UI expanded and normalized

Updated `bobui_web/public/react/app.js` and `bobui_web/server.js`.

#### New/finished web capabilities

The React SPA now exposes:

- **Dashboard**
- **Scanner**
- **Duplicates**
- **Statistics**
- **Hasher**
- **Metadata**

#### Important data-shape fixes

The earlier SPA work had already proven the scanner and duplicate endpoints, but several CLI/JSON shape mismatches were still present:

- `scan` returns `path`, not `filename`
- `hash` returns `hash`, not `fast64`
- `stats.extensions` is an **array of `{ ext, count, size }` objects**, not a keyed object map

The React layer now normalizes these backend shapes explicitly instead of assuming one rigid structure.

#### New API route

Added `POST /api/metadata` in `bobui_web/server.js`, backed by:

- `fo_cli metadata --format=json ...`

This keeps the web lane aligned with the CLI-first project rule.

### 4. Pure Qt lane no longer piggybacks on BobUI package discovery

Updated:

- `CMakeLists.txt`
- `scripts/build_qt_gui.bat`

Previously, `FO_BUILD_QT_DEMO` still flowed through `BobUIQtSetup`, which meant the supposedly plain Qt lane could accidentally resolve Qt packages from BobUI's build tree.

That was misleading.

The new behavior is:

- **BobUI discovery is used only for BobUI-backed GUI/Omni/BobUI demo targets**
- **Pure Qt demo builds are now expected to resolve Qt through external Qt package discovery**

This is architecturally cleaner because it preserves the intended distinction:

- **Qt demo** = plain Qt lane
- **BobUI demo** = BobUI/Omni layer lane

### 5. `build_qt_gui.bat` now reflects host reality more clearly

The script now:

- honors `QT6_ROOT`, `QT_ROOT`, and `QTDIR`
- auto-detects `D:\Qt\6.11.0\mingw_64` when no explicit Qt root is provided
- emits a warning when that detected kit is **MinGW-based** while the helper is launching an **MSVC** environment

This does not "solve" the host toolchain mismatch, but it does make the build helper more transparent and useful.

### 6. BTK demo thread handoff simplified

Updated:

- `frontends/btk/src/DemoWindow.hpp`
- `frontends/btk/src/DemoWindow.cpp`

The BTK demo previously used a rougher lambda-based UI handoff path plus stray dead code/comments from experimentation.

It now uses explicit queued slot delivery via:

- `applyScanResult(const QString &)`
- `applyDuplicatesResult(const QString &)`

with `QMetaObject::invokeMethod(..., "slotName", Qt::QueuedConnection, Q_ARG(QString, ...))`

This is clearer and more aligned with the older Qt/CopperSpice-style runtime model that BTK actually inhabits.

## Validation Notes

### Headless validation still matters most

Because several of these frontend lanes are toolchain-conditional on this host, headless validation remains the most reliable regression guard.

### Pure Qt lane reality on this host

`D:\Qt` does exist, but the visible desktop Qt kit remains:

- `D:\Qt\6.11.0\mingw_64`

The active validated native compiler path in this repo remains MSVC 2019 Build Tools.

So the current honest state is:

- the **pure Qt lane is now configured honestly**
- the **host still lacks the clean MSVC Qt desktop kit** that would make the pure Qt demo straightforward to validate end-to-end

That is an environment boundary, not a reason to keep muddying Qt and BobUI discovery together.

## Why This Direction

This session deliberately favored **clarity of lanes** over pretending all frontends are equally mature on this host.

The important outcome is not that every lane is fully green right now.
The important outcome is that:

- each lane has a clearer identity
- the React/JUCE/Qt/BobUI demos are increasingly exercising the same backend workflows
- the build helpers and docs describe host/toolchain reality more honestly

## Next Recommended Steps

1. **Validate the React lane manually in a browser** against the new stats/hash/metadata routes.
2. **Continue parity work in the remaining native demos** where the host/runtime boundary allows it.
3. **Add a narrow C bridge for BobGUI** if we want true direct `fo_core` integration there rather than keeping it as the least-integrated demo lane.
4. **Install or point at an MSVC-compatible Qt desktop kit** to fully validate the decoupled pure Qt and BobUI QML lanes.
