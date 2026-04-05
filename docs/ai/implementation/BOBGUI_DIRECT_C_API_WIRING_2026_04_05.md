# BobGUI Direct C API Wiring — 2026-04-05

## Summary

After adding the `fo_c_api` shim in the previous session, this session moved the BobGUI lane one step further:

- `frontends/bobgui_app` can now **prefer direct `fo_c_api` integration when available**
- and still **fall back to the CLI bridge** when direct linkage is not present

That makes the BobGUI lane more resilient and more architecturally mature.

## What Changed

### 1. BobGUI app now supports two backend modes
Updated:
- `frontends/bobgui_app/main.c`

The BobGUI app now has a two-tier backend strategy:

1. **Preferred mode:** direct `fo_c_api`
2. **Fallback mode:** `fo_cli`

This is selected at build/runtime availability boundaries instead of hardcoding only one path.

### 2. Conditional direct C API support in Meson
Updated:
- `frontends/bobgui_app/meson.build`
- added `frontends/bobgui_app/meson_options.txt`

The Meson app now attempts to discover `fo_c_api` in common repo-relative build directories and, when found, defines:
- `BOBFILEZ_HAVE_C_API=1`

This allows the BobGUI app to compile in direct native support without removing the CLI fallback.

### 3. Direct C API use remains async
Even in direct-library mode, the BobGUI app still runs work in a background GLib thread and applies results back on the UI loop with `g_idle_add(...)`.

That means the move from CLI-bridge mode to direct-C-API mode does **not** regress responsiveness.

### 4. Added a true C consumer smoke test
Added:
- `tests/c_api_smoke.c`

Updated:
- `tests/CMakeLists.txt`

The new smoke executable links to `fo_c_api` as a real C consumer and validates that the C ABI is not only callable from C++ tests but also consumable from a C-compiled client.

## Why this matters

The BobGUI lane now has a genuine progressive integration story:

- placeholder UI
- CLI bridge
- direct C ABI shim
- BobGUI app preferring direct C ABI when available

That is a much healthier architecture than a brittle one-shot jump from placeholder to fully native direct integration.

## Validation

Validated through the normal MSVC headless path:

- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅

This confirmed:
- the new C smoke consumer builds and runs
- the existing GTest suite still passes
- the total discovered validation surface increased again

## Remaining Host Boundary

The host still lacks the BobGUI/Meson toolchain on PATH:
- `meson`
- `pkg-config`
- `ninja`

So full end-to-end BobGUI app validation on this machine is still an environment issue.

But the source-side wiring is now significantly stronger and much closer to real direct BobGUI integration.

## Recommended Next Step

Once the BobGUI/Meson toolchain is available on the host, validate the BobGUI app in both modes:

1. direct `fo_c_api` mode
2. CLI fallback mode

and keep both paths until the direct mode is fully trusted.
