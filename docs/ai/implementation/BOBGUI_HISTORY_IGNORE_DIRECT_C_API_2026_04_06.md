# BobGUI History + Ignore Direct C API Expansion — 2026-04-06

## Summary

The previous session added **History** and **Ignore Rules** to the React web UI and to BobGUI through BobGUI's per-operation `fo_cli` fallback path.

That was useful, but another obvious refinement remained:

- BobGUI already had a direct `fo_c_api` seam for scan / duplicates / stats / hash / metadata / lint
- history and ignore listing were still CLI-only in BobGUI

This session closes that gap by extending the direct C ABI so BobGUI can now use the **same preferred-direct / fallback-CLI model** for history and ignore-rule listing too.

## What Changed

### Direct C API expanded
Updated:
- `core/include/fo/c_api/bobfilez_c_api.h`
- `core/c_api/bobfilez_c_api.cpp`

Added JSON functions:
- `fo_bobfilez_history_json(...)`
- `fo_bobfilez_ignore_json(...)`

Added summary-text functions:
- `fo_bobfilez_history_summary_text(...)`
- `fo_bobfilez_ignore_summary_text(...)`

### New C API behavior for stateful requests
History and ignore rules are database-backed rather than path-backed workflows.

To keep the C ABI narrow while still making these operations practical, the C API now resolves its database path as:
1. `BOBFILEZ_DB_PATH` environment variable, when set
2. otherwise `fo.db`

That keeps the ABI small, makes tests deterministic, and avoids forcing an early larger structured configuration ABI.

### BobGUI direct mapping updated
Updated:
- `frontends/bobgui_app/main.c`

BobGUI now maps these operations through the direct seam when available:
- `history` → `fo_bobfilez_history_summary_text(...)`
- `ignore` → `fo_bobfilez_ignore_summary_text(...)`

The existing fallback model remains intact:
- prefer direct `fo_c_api`
- fall back to `fo_cli` when direct support is unavailable or fails

## Testing
Updated:
- `tests/test_c_api.cpp`
- `tests/c_api_smoke.c`

Added coverage for:
- history JSON
- history summary text
- ignore JSON
- ignore summary text

The C++ test uses a temporary DB path via `BOBFILEZ_DB_PATH` so database-backed state can be exercised without mutating the repo-local `fo.db`.

## Validation
Validated on this host:
- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅

Validation surface is now:
- **72 / 72 tests passed**

## Why this matters

This session strengthens the architectural story in two ways:

1. BobGUI's direct seam is continuing to scale workflow-by-workflow rather than stalling after the first few operations.
2. The project now has a cleaner example of how database-backed operational workflows can fit into the narrow C ABI without prematurely freezing a large configuration interface.

## Recommended Next Step

Continue using the same rule for direct C ABI growth:
- only add operations once the workflow is already real in CLI/core
- prefer narrow JSON + summary-text seams
- use environment/config indirection sparingly and only where it keeps the ABI materially simpler
