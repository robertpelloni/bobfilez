# Frontend Metadata Parity Expansion — 2026-04-05

## Summary

This session continued the multi-frontend parity push by filling one of the most obvious remaining gaps:

- the web lane already had a metadata surface
- the Qt, BobUI, and JUCE demo lanes did not

That gap is now reduced by extending those native demo frontends with a real metadata workflow backed by the existing metadata provider registry.

## What Changed

### Qt demo
Updated:
- `frontends/qt/src/main.cpp`

Added a new **Metadata** tab that:
- selects a directory
- scans files through `IFileScanner`
- reads metadata through `IMetadataProvider` (`tinyexif`)
- formats readable Taken/GPS summaries
- reports when no metadata records are available

This means the plain Qt lane now exposes:
- Dashboard
- Scanner
- Duplicates
- Statistics
- Hasher
- Metadata

### BobUI demo
Updated:
- `frontends/bobui/src/QmlEngineWrapper.hpp`
- `frontends/bobui/src/QmlEngineWrapper.cpp`
- `frontends/bobui/assets/Main.qml`

Added:
- `runMetadata(...)`
- `metadataFinished(...)`
- a full QML **Metadata** tab

The BobUI/QML lane now reaches parity with the same workflow family and no longer stops short of metadata while the web lane continues ahead.

### JUCE demo
Updated:
- `frontends/juce/src/main.cpp`

Added a new **Metadata** tab using the same native pattern already established in JUCE:
- file chooser → directory selection
- background work via `juce::Thread::launch`
- UI update via `juce::MessageManager::callAsync`

This keeps the JUCE lane aligned with the rest of the native demo matrix.

## Why this matters

The project now has a more believable frontend story:

- web is not the only lane exposing metadata
- the native demos continue converging around a shared practical capability set
- parity work is being driven by real `fo_core` seams rather than mock data

## Validation

Validated where the host/toolchain allowed it:

- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅
- `scripts/build_juce_gui.bat` ✅

The host still lacks the clean Qt/BobUI runtime/toolchain combination needed for full end-to-end validation of every QML/Qt lane, but the source-side parity work is now substantially better aligned.

## Recommended Next Step

Continue parity work by choosing the next best cross-lane user-facing workflow that already exists in either web or CLI form but is still missing from one or more native demo lanes.
