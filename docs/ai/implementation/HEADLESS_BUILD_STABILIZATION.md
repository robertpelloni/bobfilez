# Headless Build Stabilization — v6.0.5

## Summary
This session converted bobfilez from a dependency-blocked Windows build into a verified **headless MSVC/Ninja build** that compiles the core library, CLI, tests, and benchmarks successfully.

## Why this was needed
The default Windows configure/build path was blocked by two major issues:

1. **Heavy optional media dependencies were effectively mandatory in practice**
   - `ffmpeg:x64-windows` repeatedly failed during vcpkg build/link on this host.
   - `chromaprint` was coupled to the same media-analysis path.
   - These providers are already guarded in code and do not need to be part of the default non-GUI build.

2. **Full GUI validation requires Qt6/Omni dependencies not available on this machine**
   - The repo could not complete a full `FO_BUILD_OMNI=ON` configure because `Qt6Config.cmake` / Qt6 package discovery was unavailable.

## Build strategy adopted
A pragmatic stabilization path was used instead of waiting on the full GUI environment:

- keep the default repository build meaningful on Windows
- make native FFmpeg/Chromaprint opt-in instead of mandatory
- provide a repeatable **headless** build entrypoint for MSVC users
- fix the C++ compile errors that were hidden behind the earlier dependency failures

## Changes made

### 1. Optionalized native media-analysis dependencies
**File:** `vcpkg.json`

- Removed `ffmpeg` and `chromaprint` from the default dependency list.
- Added a new feature:
  - `media-analysis`
- This keeps the native in-process video/audio providers available for environments that explicitly enable them, while allowing the core/CLI/test build to proceed without forcing FFmpeg compilation on every Windows machine.

## 2. Fixed CMake target ordering for optional provider wiring
**File:** `core/CMakeLists.txt`

- Moved FFmpeg/Chromaprint `target_compile_definitions()` / `target_link_libraries()` usage so it occurs only after `add_library(fo_core ...)` exists.
- Kept feature-guarded provider enablement intact.
- Wired the vendored `md4c` sources/include path into `fo_core` so the markdown backend can compile without relying on an external package.

## 3. Added a repeatable Windows headless build entrypoint
**File:** `scripts/build_headless.bat`

The script:
- locates a usable `vcvars64.bat`
- initializes the MSVC environment
- configures `build-msvc` with:
  - `-DFO_BUILD_GUI=OFF`
  - `-DFO_BUILD_OMNI=OFF`
  - `-DFO_BUILD_TESTS=ON`
  - `-DFO_BUILD_BENCH=ON`
- builds with conservative parallelism (`--parallel 2`) to avoid MSVC heap pressure seen with more aggressive settings

## 4. Resolved latent compile errors in the newly expanded core/CLI surface
Representative fixes included:

- `core/include/fo/core/batch_rename_interface.hpp`
  - added `<memory>`
  - replaced invalid `virtual bool enabled = true;` with `bool enabled = true;`
- `core/src/batch_rename_engine.cpp`
  - corrected enum usage to `Mode::SnakeCase`
- `core/include/fo/core/conversion_interface.hpp`
  - added `<memory>`
  - added `ui_hint` field to match existing initializer usage
- `core/src/advanced_archive_manager.cpp`
  - disambiguated archive engine calls
  - added a local UUID helper for Windows-safe compilation
- `core/src/conversion_engine.cpp`
  - added missing `available_options()` overrides for several converter backends
- `core/src/enhanced_fileops.cpp`
  - removed stray non-portable UUID header include
  - corrected renamed/invalid option enum fields
- `core/include/fo/core/enhanced_fileops_interface.hpp`
  - reordered/fixed `TransferJob` to use `EnhancedCopyOptions`
- `core/src/hex_editor.cpp`
  - added `NOMINMAX` before `windows.h`
- `core/src/markdown_viewer.cpp`
  - aligned feature flags with the vendored `md4c` version
- `core/include/fo/core/omnivision_engine_interface.hpp`
  - added missing `<map>`
- `core/src/omniverse_engine.cpp`
  - replaced `M_PI` usage with a local constant
- `core/include/fo/core/search_interface.hpp`
  - corrected `ReplaceOptions::match_mode` typing
- `core/src/wasm_bridge.cpp`
  - restricted implementation to `__EMSCRIPTEN__`
- `cli/fo_cli.cpp`
  - added missing `<regex>` include
  - restored compatibility aliases for later-added command blocks using `args` / `paths`
  - removed the remaining `output_path` shadowing warning in the `convert` command

## Verification performed

### Build
Executed:

```powershell
scripts\build_headless.bat
```

### Successful artifacts
- `build-msvc/core/fo_core.lib`
- `build-msvc/cli/fo_cli.exe`
- `build-msvc/tests/fo_tests.exe`
- `build-msvc/benchmarks/fo_benchmarks.exe`
- `build-msvc/benchmarks/fo_bench_dhash.exe`

### CLI smoke test
Executed:

```powershell
.\build-msvc\cli\fo_cli.exe --help
```

Result:
- CLI launched successfully and reported the expected command surface.

### Test suite
Executed:

```powershell
.\build-msvc\tests\fo_tests.exe
```

Result:
- **63 / 63 tests passed**

## Remaining gap
The repository is now validated for the **headless** Windows/MSVC profile, but the full GUI/shell build remains environment-blocked on this machine.

Outstanding requirement:
- install/configure Qt6 so CMake can resolve `Qt6Config.cmake`
- then re-run a full build with GUI/Omni targets enabled

## Recommended next steps
1. Preserve `scripts/build_headless.bat` as the standard Windows fallback build path.
2. Keep native FFmpeg/Chromaprint under the explicit `media-analysis` feature unless a more reliable default FFmpeg story is established.
3. Next stabilization pass should target:
   - Qt6/Omni environment validation
   - full `FO_BUILD_OMNI=ON` configure/build
   - focused hardening of the still-scaffolded v3+ / v4+ / v5+ subsystems
