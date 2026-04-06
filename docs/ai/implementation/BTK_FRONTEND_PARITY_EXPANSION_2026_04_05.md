# BTK Frontend Parity Expansion — 2026-04-05

## Summary

The BTK/CopperSpice demo lane was still lagging behind the rest of the newer frontend matrix.

Before this session, `frontends/btk` only exposed:
- Scanner
- Duplicates

That left it noticeably behind:
- Qt
- BobUI
- JUCE
- React/web
- BobGUI

This session closed part of that gap by expanding the BTK demo to the same broader practical workflow family.

## What Changed

### `frontends/btk/src/DemoWindow.hpp`
Added new slots and result-application helpers for:
- statistics
- hashing
- metadata

### `frontends/btk/src/DemoWindow.cpp`
Expanded the BTK demo window to include new tabs for:
- **Statistics**
- **Hasher**
- **Metadata**

These use the same general strategy already proven in the other native lanes:
- path entry
- action button
- background thread for work
- queued result handoff back to the UI thread

## Implemented BTK workflows

The BTK demo now supports:
- **Scanner**
- **Duplicates**
- **Statistics**
- **Hasher**
- **Metadata**

### Backing core seams
The new BTK tabs use the existing bobfilez backend seams directly:
- `Registry<IFileScanner>` for scan/stat/metadata discovery
- `Engine` for duplicate grouping
- `Registry<IHasher>` for hashing
- `Registry<IMetadataProvider>` for metadata reads

## Why this matters

BTK remains a research/native-experiment lane, but it should still be a credible member of the frontend matrix rather than a permanently underpowered side demo.

This expansion helps preserve that credibility without pretending BTK is the active Omni runtime direction.

## Validation Reality

This host still does **not** provide straightforward end-to-end validation for the BTK lane in the same way it does for the headless and JUCE paths.

So the important validation this session focused on was:
- keep headless green
- keep the broader repo test surface green
- make the BTK source-side parity stronger and more internally consistent

Validated:
- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅
- full validation surface now: **70 / 70 passed**

## Recommended Next Step

Continue broadening parity in the remaining lower-maturity frontend lanes while keeping BTK clearly documented as the research lane rather than the primary Omni runtime path.
