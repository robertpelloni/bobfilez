# Frontend Lint Parity Expansion — 2026-04-05

## Summary

After metadata parity work, another practical cross-lane gap remained obvious in the native demos:

- the BobGUI lane had direct/fallback lint support
- the web/CLI layers already exposed lint
- the Qt, BobUI, and JUCE demo lanes still lacked a lint workflow

This session reduces that gap by expanding those native demo frontends with real lint support backed by the registered `std` linter.

## What Changed

### Qt demo
Updated:
- `frontends/qt/src/main.cpp`

Added a new **Lint** tab that:
- selects a directory
- invokes `Registry<ILinter>::instance().create("std")`
- renders issue counts and sample issue details
- reports when no issues are found

### BobUI demo
Updated:
- `frontends/bobui/src/QmlEngineWrapper.hpp`
- `frontends/bobui/src/QmlEngineWrapper.cpp`
- `frontends/bobui/assets/Main.qml`

Added:
- `runLint(...)`
- `lintFinished(...)`
- a full QML **Lint** tab

This keeps the BobUI lane closer to the same practical workflow family already available in the CLI/web/BobGUI layers.

### JUCE demo
Updated:
- `frontends/juce/src/main.cpp`

Added a new **Lint** tab using the same native JUCE execution pattern already established elsewhere:
- `juce::FileChooser`
- background work through `juce::Thread::launch`
- UI updates through `juce::MessageManager::callAsync`

## Why this matters

Lint is a high-value, low-ceremony filesystem hygiene workflow. It is exactly the kind of practical feature that should not remain stranded in only one or two lanes if the project is serious about the multi-frontend story.

This session improves that story by making the native demo family less uneven.

## Validation

Validated where the host/toolchain allowed it:
- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅
- `scripts/build_juce_gui.bat` ✅

Validation surface remains:
- **71 / 71 tests passed**

## Recommended Next Step

Continue selecting the next best practical workflow gap in the matrix, especially where:
- the backend seam already exists
- the user-facing interaction is simple enough to add without framework-specific overengineering
