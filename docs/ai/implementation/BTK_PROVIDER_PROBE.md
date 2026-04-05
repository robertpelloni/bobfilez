# BTK Provider Probe — First Real Native Consumer Validation

## Summary
This session moved beyond the naming/provider switch and performed the first real BTK-native validation work against the bobfilez repository.

The result is useful and specific:
- the `libs/btk` submodule is now properly registered in the superproject
- BTK can be configured in place on this machine
- but the current BTK build is **not yet consumable by bobfilez's GUI targets** through its generated package exports

That means the active blocker has moved from "wrong provider model" to "incomplete or not-yet-consumer-ready BTK build exports."

## What was validated
### 1. `libs/btk` submodule registration
The earlier add attempt timed out mid-clone and left a partial nested repo without a tracked gitlink.

This session repaired that by:
- checking out BTK's default branch (`master`) inside `libs/btk`
- re-running `git submodule add -f https://github.com/robertpelloni/btk.git libs/btk`

Result:
- `libs/btk` is now tracked as a real git submodule entry
- current observed gitlink:
  - `5604225074e6e11688e4c9f63996b179b5303ed8`

## 2. BTK in-place build attempt
Ran:
- `scripts/build_btk_inplace.bat`

### Result
BTK **configures successfully** on this machine.

Notable findings from the configure phase:
- BTK identifies as a BTK/CopperSpice-line framework
- it enables a broad component set (Core, Xml, Gui, Multimedia, Network, OpenGL, Sql, Svg, WebKit, XmlPatterns)
- Vulkan was not found, so CsVulkan was skipped

### Current BTK build blocker
The actual compile then fails during BTK's own build with errors centered on:
- `libs/btk/src/core/kernel/btkinputowner.h`
- `libs/btk/src/core/kernel/btkinputowner.cpp`

Observed failure shape:
- `Capabilities` type parsing failures
- `Q_DECLARE_FLAGS`-related syntax fallout
- multiple cascading MSVC compile errors in `BtkInputOwner`

This means:
- BTK is not yet building cleanly on this host/configuration
- bobfilez cannot rely on a finished installed/exported BTK package from this tree yet

## 3. BTK-backed bobfilez GUI probe
Ran:
- `scripts/build_btk_gui.bat`

### First issue found and fixed
The first GUI probe incorrectly picked up the **source-tree** `cmake/BTKConfig.cmake` template rather than the generated build-tree package config.

Why that happened:
- `BTKFrameworkSetup.cmake` originally included source-tree hints too early
- the raw source tree contains a template config file that is not suitable for downstream consumption

Fix applied:
- adjusted `cmake/BTKFrameworkSetup.cmake` to prefer:
  - `libs/btk/build-btk`
  - build/install-style prefixes
- removed the earlier dependence on source-tree `cmake/` hints as the preferred lookup path

### Second issue: real BTK consumer blocker
After fixing hint ordering, the BTK-backed GUI probe advanced to the next, more meaningful failure:

BTK's generated build-tree `BTKConfig.cmake` tries to include companion files that are **not present where that config expects them**, including:
- `CopperSpiceLibraryTargets.cmake`
- `CopperSpiceBinaryTargets.cmake`
- `BTKMacros.cmake`
- `BTKDeploy.cmake`

As a result:
- `find_package(BTK CONFIG)` does locate `libs/btk/build-btk/BTKConfig.cmake`
- but that config does not currently form a usable downstream package entry point for bobfilez
- target resolution then fails, including:
  - inability to resolve a usable BTK/CopperSpice target for `Core`

## Interpretation
This is a much better failure than before.

Previously, bobfilez still depended on:
- BobUI-specific provider assumptions
- BobUI-specific bootstrap code
- BobUI-specific source-tree layout (`OmniUI/omnicore`)

Now, the failure is **inside the BTK provider/export path itself**, which means:
- the bobfilez-side provider retargeting is doing its job
- the next work belongs to BTK package/export readiness and compatibility, not to legacy BobUI cleanup in active wiring

## Recommended next steps
1. Fix or work around BTK's build failure in `BtkInputOwner` on this MSVC environment.
2. Once BTK builds, produce a real install/export prefix and probe bobfilez against that install instead of the raw build tree.
3. If BTK build-tree exports are meant to be consumable directly, repair the generated `BTKConfig.cmake` companion-file paths.
4. Keep active bobfilez native integration pointed at BTK; do not regress to the older BobUI bootstrap/provider model.
