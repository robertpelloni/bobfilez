# BobGUI Ignore Actions Direct C API Expansion — 2026-04-06

## Summary

The previous session extended the direct `fo_c_api` seam so BobGUI could use native/direct handling for:
- history listing
- ignore-rule listing

The next obvious gap was that **ignore add/remove actions** still depended on CLI routing.

This session closes that gap by:
1. extending the direct C API with ignore add/remove helpers
2. upgrading the BobGUI surface so ignore management is no longer read-only
3. keeping the existing per-operation fallback model intact

## What Changed

### Direct C API expanded again
Updated:
- `core/include/fo/c_api/bobfilez_c_api.h`
- `core/c_api/bobfilez_c_api.cpp`

Added JSON helpers:
- `fo_bobfilez_ignore_add_json(pattern, reason)`
- `fo_bobfilez_ignore_remove_json(pattern)`

Added summary-text helpers:
- `fo_bobfilez_ignore_add_summary_text(pattern, reason)`
- `fo_bobfilez_ignore_remove_summary_text(pattern)`

These functions continue to use the same database-path rule established for the history/ignore listing work:
- `BOBFILEZ_DB_PATH` when set
- otherwise `fo.db`

### BobGUI upgraded from read-only ignore listing to actionable ignore management
Updated:
- `frontends/bobgui_app/main.c`

Added:
- dedicated **Ignore Pattern** input
- dedicated **Reason** input
- **Ignore Add** button
- **Ignore Remove** button

The existing main path entry remains focused on filesystem-oriented actions, while ignore management now has a clearer dedicated input lane.

### CLI fallback still preserved
BobGUI still behaves honestly per operation:
- prefer direct `fo_c_api`
- fall back to `fo_cli` if the direct path is unavailable or fails

This means the direct seam is stronger, but resilience is unchanged.

## Testing
Updated:
- `tests/test_c_api.cpp`
- `tests/c_api_smoke.c`

Added coverage for:
- ignore add JSON
- ignore add summary text
- ignore remove summary text
- post-action database state verification

Validation performed:
- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅

Validation surface is now:
- **73 / 73 tests passed**

## Why this matters

This session is a good example of a healthy workflow-expansion pattern:
- first expose read-only state
- then expose safe write actions
- only then consider broader UI polish

It also improves BobGUI specifically by making ignore management feel more like a real operational surface rather than a passive viewer.

## Recommended Next Step

At this point, the best next move is probably not more raw C API width for ignore rules, but either:
1. additional BobGUI usability polish around the broader operational surface, or
2. selection of the next lightweight backend-real workflow with clear frontend gaps
