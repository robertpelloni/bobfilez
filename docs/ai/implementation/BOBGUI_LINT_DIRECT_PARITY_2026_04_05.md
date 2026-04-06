# BobGUI Lint Direct Parity — 2026-04-05

## Summary

The BobGUI lane had become substantially more capable through:
- CLI bridge mode
- direct `fo_c_api` mode
- summary-text direct mode
- resilient per-operation fallback

One practical workflow was still missing from the direct BobGUI surface:
- **lint**

This session closes that gap by extending both the C API seam and the BobGUI app so lint behaves like the other major BobGUI workflows.

## What Changed

### Direct C API
Updated:
- `core/include/fo/c_api/bobfilez_c_api.h`
- `core/c_api/bobfilez_c_api.cpp`

Added:
- `fo_bobfilez_lint_json(...)`
- `fo_bobfilez_lint_summary_text(...)`

Implementation details:
- the C API now consumes the registered `std` linter through `Registry<ILinter>`
- lint JSON returns machine-readable issue records
- lint summary returns a display-friendly summary with counts and sample issues

### BobGUI app
Updated:
- `frontends/bobgui_app/main.c`

Added:
- a **Lint** button to the BobGUI action row
- direct-mode lint mapping through the new C API summary helper
- continued participation in the resilient per-operation fallback model

So the BobGUI lane can now expose lint through:
- direct `fo_c_api` summary mode
- `fo_cli` fallback mode

## Why this matters

This continues the same pattern used across the rest of the frontend matrix:
- avoid leaving obvious high-value workflows stranded in only one or two lanes
- keep alternate frontends honest and practically useful
- preserve fallback behavior so one backend problem does not make the UI useless

## Validation

Updated tests:
- `tests/test_c_api.cpp`
- `tests/c_api_smoke.c`

Validated:
- lint JSON through the direct C API
- lint summary through the direct C API
- continued C consumer smoke behavior

Full validation run:
- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅
- result: **71 / 71 tests passed**

## Recommended Next Step

Continue choosing the next highest-value practical workflow gap in the multi-frontend matrix, especially where a direct seam and a fallback seam can coexist cleanly.
