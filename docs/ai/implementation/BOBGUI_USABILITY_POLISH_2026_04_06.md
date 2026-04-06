# BobGUI Usability Polish — 2026-04-06

## Summary

After extending BobGUI with direct support for history, ignore listing, and ignore add/remove actions, the next bottleneck was no longer backend reachability.

It was **ergonomics**.

The lane had become substantially more capable, but the UI still treated every action as if it shared the same input model. That was increasingly awkward now that BobGUI supported both:
- filesystem path actions
- ignore-management actions
- path-free operational actions

This session improves that by refining the BobGUI control surface without changing the underlying direct/fallback architecture.

## What Changed

Updated:
- `frontends/bobgui_app/main.c`

### 1. Clearer operational guidance while requests are running
Added a small pending-output helper so BobGUI now writes a more descriptive in-progress message before the background worker finishes.

The output now shows:
- operation name
- effective target
- ignore reason when relevant
- an explicit note that the UI remains responsive because the request runs off the main thread

### 2. Dedicated utility actions for the broader operational surface
Added:
- **Reset Ignore Fields** button
- **Clear Output** button

These are simple, low-risk ergonomic helpers that matter more once the BobGUI surface stops being a single-shot demo and starts behaving like an operational panel.

### 3. Better separation between filesystem actions and ignore-management inputs
The lane already had dedicated ignore inputs from the previous session. This session builds on that by making it easier to:
- restore known-good example ignore values
- clear noisy output without destroying the rest of the state

### 4. Improved initial guidance text
Refined the startup output so users are told more clearly which controls are meant for:
- path-based operations
- ignore management
- path-free listing operations

## Why this matters

This is a small but important product step.

Once a frontend supports more than a handful of actions, usability starts to matter almost as much as raw feature count. In BobGUI specifically, the direct C API expansion had already made the lane more capable; this session helps make that capability easier to operate.

## Validation

This session is primarily UI-surface refinement in the BobGUI lane. On this host, full BobGUI runtime validation is still constrained by missing convenience tooling for that framework.

Repo-wide safety validation was still re-run:
- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅

Validation surface remains:
- **73 / 73 tests passed**

## Recommended Next Step

Continue favoring this pattern when a lane becomes more capable:
1. expand real backend reach first
2. then improve the control surface so the new capability is actually pleasant to use
