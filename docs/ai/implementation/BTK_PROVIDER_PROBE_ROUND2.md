# BTK Provider Probe Round 2 — Export Surface Repaired, Library Build Still Blocking

## Summary
This session refined the BTK consumer probe so it could move beyond the earlier package-layout failure.

The result is a sharper and more valuable conclusion:
- bobfilez can now consume a locally prepared BTK build-tree package surface
- the remaining BTK-backed GUI configure failure is now due to **missing BTK libraries in `build-btk/lib` and `build-btk/bin`**
- those libraries are missing because BTK's own build still fails earlier in `btkinputowner.*`

This means the active blocker has been narrowed again. The problem is no longer package-layout breakage; it is now firmly BTK's incomplete build output on this host.

## What changed
### 1. Added a dedicated BTK build-tree package-prep script
Added:
- `scripts/prepare_btk_buildtree_package.py`
- `scripts/prepare_btk_buildtree_package.bat`

Purpose:
- locate the generated BTK export directory under `libs/btk/build-btk/CMakeFiles/Export/...`
- copy the BTK/CopperSpice macro/deploy helpers into `libs/btk/build-btk`
- prepare consumer-facing package files in the BTK build root

### 2. Updated the BTK GUI probe script
Updated:
- `scripts/build_btk_gui.bat`

Change:
- it now invokes the BTK build-tree package-prep step before running the bobfilez GUI configure

### 3. Fixed the target-export prefix issue in the prepared package files
The first package-layout workaround still produced a bad `_IMPORT_PREFIX` for imported targets.

Specifically:
- simply copying the generated export files into the build root caused their relative import-prefix logic to resolve incorrectly

The new prep step now patches the prepared export files so `_IMPORT_PREFIX` points explicitly at:
- `libs/btk/build-btk`

This makes the prepared BTK package surface much closer to what a downstream consumer actually needs.

## Probe result after the improved prep step
After the package-prep improvements, the BTK-backed bobfilez GUI configure fails with:
- imported target `CopperSpice::CsCore` referencing:
  - `libs/btk/build-btk/lib/CsCore2.1.lib`
- and that file does not exist

This is the correct and expected downstream failure now.

## Interpretation
This is a better result than the earlier probe because:
- the missing-companion-file problem has been bypassed
- the bad-import-prefix problem has been corrected
- the next blocker is now simply that BTK has not successfully produced its expected output libraries yet

That ties the remaining consumer failure directly back to BTK's own build failure in:
- `src/core/kernel/btkinputowner.h`
- `src/core/kernel/btkinputowner.cpp`

## Practical conclusion
The next required work is now unambiguous:
1. Fix BTK's own MSVC compile failure in `btkinputowner.*`
2. Let BTK actually produce `CsCore2.1.lib`, `CsGui2.1.lib`, and related binaries
3. Re-run the bobfilez BTK-backed GUI probe

Until that happens, further bobfilez-side GUI retargeting will not change the fundamental outcome.

## Recommended next steps
1. Investigate the `Capabilities` / `Q_DECLARE_FLAGS` failure pattern in BTK's `btkinputowner.*`.
2. If an upstream or local BTK fix is possible, apply it in the BTK repo itself rather than in bobfilez.
3. Re-run `scripts/build_btk_inplace.bat` until BTK produces the expected libraries in `build-btk/lib` and `build-btk/bin`.
4. Re-run `scripts/build_btk_gui.bat` once those artifacts exist.
