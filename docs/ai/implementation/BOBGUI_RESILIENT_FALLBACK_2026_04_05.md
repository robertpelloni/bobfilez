# BobGUI Resilient Fallback Refinement — 2026-04-05

## Summary

The BobGUI lane already had two backend modes:

1. preferred direct `fo_c_api`
2. fallback `fo_cli`

But the earlier implementation still had a brittle edge:
- if the direct path was compiled in but failed for a specific operation, the app would stop there even if the CLI fallback was available

This session fixed that by making the BobGUI backend selection more resilient at the **operation** level, not just at startup.

## What Changed

### `frontends/bobgui_app/main.c`
Added a shared helper:
- `run_cli_request(...)`

Then updated the BobGUI worker path so that:
- if direct `fo_c_api` mode is available and succeeds, it is used normally
- if direct mode is unavailable for a specific operation, and `fo_cli` exists, the app automatically falls back
- if direct mode returns an error, and `fo_cli` exists, the app automatically falls back
- the output clearly explains that a fallback occurred and why

## Why this matters

This change makes the BobGUI lane more robust in realistic mixed environments:
- partial direct support
- temporary direct failures
- different build outputs available at different times

Instead of forcing the user to manually reason about backend availability, the app now behaves more like a resilient product:
- prefer the best path
- degrade gracefully
- explain what happened

## User-facing effect

When fallback happens, the BobGUI output now explicitly notes:
- that direct `fo_c_api` was unavailable or failed
- the reason for fallback
- the resulting `fo_cli` output

That preserves transparency without sacrificing usability.

## Validation

This change did not alter the headless functional seams directly, but the repo-wide baseline was revalidated:

- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅
- result: **70 / 70 tests passed**

## Recommended Next Step

Once the BobGUI/Meson toolchain is available on-host, validate the BobGUI app under scenarios where:
- direct mode succeeds
- direct mode is absent
- direct mode fails and CLI fallback is used

That will verify the new resilience behavior end-to-end.
