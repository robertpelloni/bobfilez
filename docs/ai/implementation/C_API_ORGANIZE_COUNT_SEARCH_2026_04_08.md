# C API Organize + Count + SearchEngine Integration

**Date:** 2026-04-08
**Version:** 6.0.73
**Scope:** Expand the C API seam with organize dry-run and count operations, integrate SearchEngine into Engine, add comprehensive SearchEngine tests.

## What Changed

### C API — Organize Dry-Run

Two new C API functions expose safe organization previews:

- `fo_bobfilez_organize_dry_run_json(root_path, destination_template)` — returns a JSON object listing all planned moves without touching the filesystem.
- `fo_bobfilez_organize_dry_run_summary_text(root_path, destination_template)` — returns a human-readable text summary of the same preview.

Both use the `RuleEngine` internally with a single rule derived from the `destination_template` parameter. They validate that neither `root_path` nor `destination_template` is empty, and they follow the established error-reporting pattern through `fo_bobfilez_last_error()`.

### C API — Count

Two new C API functions expose combined file/duplicate counting:

- `fo_bobfilez_count_json(root_path)` — returns JSON with `files`, `directories`, `total_size`, `duplicate_groups`, `duplicate_files`, `wasted_size`, and human-readable size fields.
- `fo_bobfilez_count_summary_text(root_path)` — returns a readable summary of the same data.

These are thin wrappers over the scan+duplicate pipeline and format the output using the same pattern as the existing stats/count C API helpers.

### SearchEngine Integration

`SearchEngine` is now accessible via `Engine::search_engine()`. The integration required:

- Moving `Engine`'s constructor and destructor out-of-line into `engine.cpp`.
- Forward-declaring `SearchEngine` in `engine.hpp` and managing the raw pointer manually.
- Moving `SearchEngine`'s constructor and destructor out-of-line into `search_engine.cpp`.

These structural changes were necessary because MSVC 2019 (14.29) aggressively instantiates `unique_ptr` deleters at class-definition time, causing incomplete-type errors when the pimpl `Impl` struct is not visible.

### Tests — 21 New Tests

| Suite | Tests | Coverage |
|-------|-------|----------|
| `SearchEngineTest` | 15 | Literal, wildcard, regex filename search; content literal/regex; extension filter; size filter; max results; empty query; recursive; invert match; engine integration; cancel |
| `CApiTest` (new tests) | 5 | Organize dry-run JSON, summary, empty-template rejection; count JSON, summary |
| `fo_c_api_smoke` (updated) | — | Now exercises organize and count functions |

Test count went from **74 → 95** (all passing).

## Files Modified

- `core/include/fo/c_api/bobfilez_c_api.h` — added 4 new function declarations
- `core/c_api/bobfilez_c_api.cpp` — added organize dry-run and count implementations
- `core/include/fo/core/engine.hpp` — added `SearchEngine*` member, moved constructor/destructor out-of-line
- `core/src/engine.cpp` — added constructor, destructor, `search_engine()` definitions
- `core/include/fo/core/search_interface.hpp` — moved constructor/destructor out-of-line
- `core/src/search_engine.cpp` — added out-of-line constructor/destructor
- `tests/test_c_api.cpp` — added 5 new C API tests
- `tests/test_search_engine.cpp` — new file with 15 SearchEngine tests
- `tests/c_api_smoke.c` — added organize and count smoke tests
- `tests/CMakeLists.txt` — added `test_search_engine.cpp`
- `VERSION.md` — 6.0.73
- `core/include/fo/core/version.hpp` — 6.0.73
- `CHANGELOG.md` — entry for 6.0.73

## Validation

- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅ — **95 / 95 tests passed**
