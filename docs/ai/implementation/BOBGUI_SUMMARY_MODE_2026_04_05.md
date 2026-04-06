# BobGUI Summary Mode — 2026-04-05

## Summary

The BobGUI lane already had two backend modes:

1. direct `fo_c_api`
2. fallback `fo_cli`

This session improved the *direct* mode experience by adding a second output tier inside the C API itself:

- machine-friendly JSON helpers remain available
- BobGUI-facing human-readable summary helpers were added

That means the BobGUI app no longer needs to dump raw JSON in direct mode just because the transport is easy.

## What Changed

### New direct C API summary functions
Added to `core/include/fo/c_api/bobfilez_c_api.h` and implemented in `core/c_api/bobfilez_c_api.cpp`:

- `fo_bobfilez_scan_summary_text(...)`
- `fo_bobfilez_duplicates_summary_text(...)`
- `fo_bobfilez_stats_summary_text(...)`
- `fo_bobfilez_hash_summary_text(...)`
- `fo_bobfilez_metadata_summary_text(...)`

These complement the existing JSON-oriented functions rather than replacing them.

## Why this was the right design

### Keep JSON
JSON is still valuable because it is:
- easy to debug
- easy to transport
- easy to expose to web/CLI/native adapters
- a good low-friction ABI boundary for C consumers

### Add summary text
Summary text is better for the BobGUI app because it:
- is immediately readable by humans
- avoids pushing parsing complexity into the BobGUI C code
- improves the direct-mode UX without forcing a large structured C ABI too early

So the project now has a useful split:
- **JSON** = transport / debug / machine boundary
- **summary text** = display-friendly direct UI boundary

## BobGUI app impact

Updated `frontends/bobgui_app/main.c` so that:
- direct `fo_c_api` mode now uses the summary-text functions
- `fo_cli` fallback mode remains as-is and still shows raw CLI output

This is actually a healthy distinction:
- direct mode feels more native and product-like
- fallback mode stays transparent and useful for debugging/compatibility

## Validation

Updated tests:
- `tests/test_c_api.cpp`
- `tests/c_api_smoke.c`

Validated:
- JSON access still works
- summary-text access works
- the real C consumer smoke test now checks both surfaces

Full validation run:
- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅
- result: **69 / 69 tests passed**

## Architectural Result

The BobGUI direct path is now more credible as a product lane because it has:
- direct native access
- useful human-readable output
- retained JSON transport/debug support
- validated C-consumer coverage

This is a stronger position than either extreme:
- raw JSON only
- or prematurely designing a large rigid C struct ABI

## Recommended Next Step

Once the BobGUI/Meson toolchain is available on-host, validate the BobGUI app in both modes and then consider small operation-specific UI refinements inside the BobGUI view itself while keeping the current dual-backend model.
