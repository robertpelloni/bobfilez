# BobGUI C API Shim — 2026-04-05

## Summary

After establishing the BobGUI CLI bridge as the first practical integration step, this session added the next architectural layer:

- a small **direct C ABI shim** around selected bobfilez core workflows
- compiled as a new library target: `fo_c_api`
- designed specifically to be friendly to C-native consumers such as the BobGUI demo lane

This does **not** replace the CLI bridge yet.
Instead, it gives the project a real second integration tier.

## Why a C shim was the correct next step

The BobGUI lane is harder than the other frontends because it crosses several boundaries at once:

- **BobGUI app:** C / Meson / GLib-oriented world
- **bobfilez core:** C++20 / CMake / STL-oriented world

Trying to jump directly from a placeholder BobGUI app to deep direct `fo_core` usage would have forced too much at once:

- C++ API exposure decisions
- ABI stability decisions
- Meson/CMake interop work
- Windows runtime coordination

A narrow C shim solves the right problem first:

- present a small surface area
- hide the C++ types
- preserve future implementation freedom behind the ABI boundary

## What was added

### New public header
- `core/include/fo/c_api/bobfilez_c_api.h`

### New implementation
- `core/c_api/bobfilez_c_api.cpp`

### New CMake target
- `fo_c_api`

Added in:
- `core/CMakeLists.txt`

## Exposed functions

The new C API currently exposes JSON-returning entrypoints:

- `fo_bobfilez_scan_json(const char* root_path)`
- `fo_bobfilez_duplicates_json(const char* root_path)`
- `fo_bobfilez_stats_json(const char* root_path)`
- `fo_bobfilez_hash_json(const char* root_path)`
- `fo_bobfilez_metadata_json(const char* root_path)`
- `fo_bobfilez_last_error(void)`
- `fo_bobfilez_free_string(char* value)`

## Why JSON was chosen for the C boundary

It would have been possible to define multiple C structs for:

- file lists
- duplicate groups
- stats summaries
- metadata records
- error payloads

That would also have been more expensive and more rigid.

Returning JSON was the better first direct ABI choice because it:

- keeps the ABI narrow and simple
- matches the already-proven CLI/web data shape strategy
- avoids prematurely freezing many C structs
- works well for UI layers that already need display-friendly payloads
- lets us refine the internal C++ implementation without immediately breaking the C-facing contract

## Important behavior decision

The shim intentionally improves one area over the CLI-only bridge path:

### Single-file hashing support
The shim recognizes a regular file path directly and can hash it without forcing the caller through directory-style scanner traversal.

That matters for future BobGUI/native-C consumers because it creates a more ergonomic direct library surface than blindly reproducing every current CLI quirk.

## Tests added

Added:
- `tests/test_c_api.cpp`

Updated:
- `tests/CMakeLists.txt`

Validated behaviors include:
- scan JSON includes created files
- duplicates JSON includes a real duplicate group
- stats JSON contains counts/extensions
- hash JSON works for a single file input
- null input reports an error cleanly

## Validation result

Validated in the normal headless MSVC path:

- `scripts/build_headless.bat` ✅
- `build-msvc/tests/fo_tests.exe` ✅
- total suite now: **68 / 68 passed**

## Architectural Result

The BobGUI lane now has a cleaner maturity ladder:

1. **BobGUI placeholder** → historical starting point
2. **BobGUI CLI bridge** → practical live functionality now
3. **BobGUI direct C ABI seam** → now real and tested
4. **Future optional BobGUI direct linking** → next stage when host tooling is ready

This is a substantial improvement because BobGUI direct integration is no longer just a theoretical next step.

## Next Recommended Step

When Meson/pkg-config/ninja are available on the host, update `frontends/bobgui_app` to optionally consume `fo_c_api` directly, while retaining the current CLI bridge as fallback behavior.
