# Frontend Lint Matrix Completion — 2026-04-06

## Summary

After native lint parity landed for the Qt, BobUI, and JUCE demo lanes, two obvious frontend-matrix gaps still remained:

- the **BTK** research-native demo still stopped at metadata
- the **React/Express** web lane still exposed scan, duplicates, statistics, hash, and metadata, but not lint in the actual SPA surface

This session closes those gaps by extending both remaining lanes with practical lint workflows.

## What Changed

### BTK demo
Updated:
- `frontends/btk/src/DemoWindow.hpp`
- `frontends/btk/src/DemoWindow.cpp`

Added a full **Lint** tab that:
- accepts a directory path
- invokes `Registry<ILinter>::instance().create("std")`
- renders issue counts by lint type
- shows the first detected issues with path and details
- preserves the same queued cross-thread result handoff pattern already used in the BTK demo

### React web UI
Updated:
- `bobui_web/public/react/app.js`

Added:
- lint state handling
- lint result normalization
- a new **Lint** navigation tab
- a lint results table showing path, type, and details
- updated dashboard copy so the web/BTK lane descriptions now match their real capabilities

The existing Express backend already exposed `POST /api/lint`, so this session focused on surfacing that capability honestly in the SPA.

## Why this matters

The multi-frontend matrix only remains credible if practical workflows do not stay stranded in just one or two lanes.

With this session:
- Qt has lint
- BobUI has lint
- JUCE has lint
- BTK has lint
- BobGUI has lint
- React web has lint
- CLI already had lint

That makes lint one of the clearer examples of a workflow now spanning the project’s major user-facing surfaces.

## Validation

Validated where practical on this host:
- `node --check bobui_web/server.js` ✅
- `node --check bobui_web/public/react/app.js` ✅
- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅

Validation surface remains:
- **71 / 71 tests passed**

## Recommended Next Step

Keep using the same selection rule for future parity work:
1. choose a workflow that already exists in CLI/core
2. identify which frontend lanes still lack it
3. expand the lightest missing lanes first
4. validate with the strongest available host-safe baseline
