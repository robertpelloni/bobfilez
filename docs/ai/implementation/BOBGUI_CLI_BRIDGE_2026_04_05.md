# BobGUI CLI Bridge Demo Expansion — 2026-04-05

## Summary

This session targeted the weakest frontend lane in the current matrix: `frontends/bobgui_app`.

Unlike the Qt, BobUI, JUCE, and BTK demos, the BobGUI lane is a **Meson-built C application** while the main bobfilez engine is a **CMake-built C++20 library**.
That cross-build-system and cross-language boundary makes a direct `fo_core` link noticeably more expensive than the other frontend integrations.

Instead of pretending that direct native linkage was already cheap and solved, this session implemented a pragmatic intermediate step:

- keep the BobGUI app native
- keep the project CLI-first
- bridge the BobGUI lane to real functionality through `fo_cli`
- do the work asynchronously so the UI is not blocked

## Why a CLI bridge first

A direct BobGUI-to-`fo_core` bridge would require at least one of the following:

1. a dedicated exported C ABI shim around selected `fo_core` workflows
2. Meson/CMake cooperation for consuming that shim reliably on this host
3. careful Windows runtime/toolchain coordination across BobGUI, GLib, and the C++ core build

That is still a valid future direction.
But for the current repo state, it would have been a larger infrastructure project than the rest of the frontend parity work.

A CLI bridge is consistent with the project’s existing priorities:

- **CLI-first**
- preserve feature availability
- avoid overpromising host-native validation where the host is still missing key BobGUI/Meson/pkg-config tooling

## What Changed

### `frontends/bobgui_app/main.c`

The BobGUI app was expanded from a static label-only window into a functional native tool window with:

- a path entry field
- buttons for:
  - **Scan**
  - **Duplicates**
  - **Statistics**
  - **Hash**
  - **Metadata**
- a live status label
- a scrollable text output view

### Async execution model

The app now:

1. locates `fo_cli.exe` from common repo-relative paths or `BOBFILEZ_CLI`
2. reads the requested path from the BobGUI entry widget
3. launches the CLI command in a background GLib thread
4. captures stdout/stderr with `g_spawn_sync(...)`
5. returns results to the UI thread with `g_idle_add(...)`

This keeps the BobGUI window responsive without introducing a larger native bridge layer yet.

## Supported BobGUI actions

The BobGUI lane now issues real CLI-backed requests for:

- `scan --format=json`
- `duplicates --format=json --mode=fast`
- `stats --format=json`
- `hash --format=json --threads=4`
- `metadata --format=json`

The output is currently shown as raw structured text/JSON in the text view.
That is intentionally simple but already useful and honest.

## Host Validation Reality

This host currently lacks the normal BobGUI/Meson toolchain surface needed to validate the BobGUI lane end-to-end:

- `meson` not found on PATH
- `pkg-config` not found on PATH
- `ninja` not found on PATH

So the BobGUI lane is now **substantially more functional in code**, but full local build validation is still blocked by the host environment rather than the app logic itself.

## Why this is still worthwhile

Even without full local validation, this session materially improves the BobGUI lane because it establishes:

- a real user interaction model
- a real backend execution path
- a clear near-term integration strategy
- a cleaner stepping stone toward a future narrow C ABI bridge

In other words, BobGUI is no longer just a decorative placeholder window.
It is now an actual BobGUI-native frontend concept with live operational intent.

## Recommended Next Step

If we want to deepen the BobGUI lane further, the best follow-up is:

1. introduce a **small C ABI shim** around a few `fo_core` workflows
2. consume that shim from the Meson BobGUI app
3. keep the CLI bridge as fallback or comparison mode

That would preserve the current practical win while moving toward true direct BobGUI-to-core integration.
