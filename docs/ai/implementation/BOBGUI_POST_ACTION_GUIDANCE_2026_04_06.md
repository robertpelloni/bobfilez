# BobGUI Post-Action Guidance Polish — 2026-04-06

## Summary

After improving BobGUI's action grouping and control layout, the next friction point was more subtle:

- actions completed successfully
- outputs were technically correct
- but the UI still did relatively little to help the user decide what to do next

This session improves that by making the BobGUI output panel more instructional after actions complete.

## What Changed

Updated:
- `frontends/bobgui_app/main.c`

### 1. Added contextual post-action guidance
Both direct and CLI-backed result rendering now append a small **next helpful action** section to the output.

This guidance varies by workflow. For example:
- ignore add/remove now suggest listing ignore rules to verify state
- history suggests re-running after future file operations
- directory-oriented workflows suggest changing the Path field when exploring another location

### 2. Improved pending-state messaging further
The in-progress output now not only explains that work is happening on a background thread, but also tells the user that the result will remain in the output panel for comparison against the next action.

That small statement matters because BobGUI is increasingly behaving like an operational workbench rather than a fire-and-forget demo.

## Why this matters

This is a product-clarity improvement rather than a backend expansion.

Once a tool exposes many workflows, success is not only about whether the action runs. It is also about whether the interface helps the user maintain momentum.

The BobGUI lane now does a better job of answering:
- what just happened
- what control produced it
- what the user should probably try next

## Validation

Repo-wide safety validation was re-run:
- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅

Validation surface remains:
- **73 / 73 tests passed**

## Recommended Next Step

If BobGUI remains the focus, the next highest-value polish step is likely some form of lightweight success-state refinement, such as:
1. contextual field updates after ignore actions
2. clearer output-panel preservation behavior
3. eventual host-run validation once the BobGUI toolchain boundary is resolved
