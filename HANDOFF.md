# HANDOFF.md — bobfilez Session 52

## Current Status (2026-04-05)
**Version:** 6.0.37  
**Focus:** Native UI launch profiles and runtime bundles — fourth concrete Option C step to move from raw config callbacks to named launch policy units

---

## What Was Done This Session

### 1. Executed the Fourth Real Option C Refactor in Code
- Continued the provider-coupling reduction by replacing the raw launch-config callback bag with clearer named policy units.
- Added:
  - **`gui/omni/src/NativeUiRuntimeBundle.hpp`**
  - **`gui/omni/src/NativeUiRuntimeBundle.cpp`**
  - **`gui/omni/src/NativeUiLaunchProfile.hpp`**
  - **`gui/omni/src/NativeUiLaunchProfile.cpp`**
- Removed the active launch-config-based bootstrap path from the build in favor of the new profile/bundle model.

### 2. Split Launch Policy into Two Named Concepts
- Introduced **`NativeUiRuntimeBundle`** to own:
  - bundle name
  - registration routine
  - runtime factory
- Introduced **`NativeUiLaunchProfile`** to own:
  - profile name
  - main QML URL
  - chosen runtime bundle
  - root-object failure handler
- The bootstrap now selects a default launch profile rather than assembling raw callback fields directly.

### 3. Kept Active Behavior the Same While Improving Policy Structure
- The active shell still launches the same way.
- The internal QQml-based runtime remains unchanged.
- This refactor does **not** add a second runtime provider.
- It does **not** change the BTK diagnosis.
- It only makes runtime/registration policy selection more explicit and more extensible for future Option C work.

### 4. Updated GUI Build Wiring for the New Policy Model
- Updated:
  - **`gui/CMakeLists.txt`**
  - **`gui/omni/CMakeLists.txt`**
- Replaced the launch-config files in the GUI source lists with the new launch-profile and runtime-bundle files.

### 5. Documented the Extraction as the Next Option C Milestone
- Added **`docs/ai/implementation/NATIVE_UI_LAUNCH_PROFILES_AND_BUNDLES.md`**.
- The document records:
  - the shift from generic config to named policy units
  - the distinction between runtime bundles and launch profiles
  - the updated bootstrap flow
  - the preserved behavior and unchanged limitations

### 6. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.37**.
- Updated:
  - **`VERSION.md`**
  - **`core/include/fo/core/version.hpp`**
  - **`CHANGELOG.md`**
  - **`HANDOFF.md`**

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BTK-backed GUI / Omni build | 🔴 Still blocked at a strategic compatibility boundary | BTK now builds successfully for its normal enabled module set, but bobfilez's active native GUI bootstrap still cannot configure because `Declarative` is not exported and the framework path no longer looks like a same-generation QML provider. |
| BTK Declarative/QML support | 🔴 Deeper upstream/provider readiness gap confirmed | Round 4 proved the issue is broader than a missing component-list entry: when `Declarative` is experimentally re-enabled, the module immediately hits stale declarative-specific CMake integration, obsolete metatype declarations, and fatal missing QtScript-era headers such as `QtScript/qscriptvalue.h`. |
| BTK vs bobfilez declarative API generation | 🔴 Direct mismatch confirmed | Round 5 proved BTK's current declarative surface is `QDeclarative*`-era and exposes no discovered `QQml*` / `QQuick*` provider surface, while bobfilez's active bootstrap is explicitly `QQmlApplicationEngine`-based. |
| Recommended project strategy | 🟢 Decision documented | Round 6 formalized the least-destructive path: keep bobfilez on a modern QQml-style shell path, reduce provider coupling, and treat BTK modernization as a separate upstream R&D effort rather than the immediate app runtime target. |
| Option C execution status | 🟡 In progress | Round 7 extracted the active shell runtime bootstrap out of `main.cpp`, Round 8 separated bootstrap/runtime/registration responsibilities, Round 9 turned launch policy into explicit configuration, and Round 10 refined that into named launch profiles and runtime bundles. |
| BTK native migration plan | 🟡 In progress | Active BobUI-specific provider/bootstrap assumptions remain removed from bobfilez, but the remaining blocker is now whether BTK can provide the QML/Declarative layer bobfilez still depends on. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Treat BTK MSVC build correctness as validated for the currently enabled BTK modules**
   - the old `btkinputowner.*` blocker is resolved
   - `CsCore2.1.lib` and related outputs now exist

2. **Continue executing Option C incrementally in code**
   - keep bobfilez aligned to a modern QQml-style shell path
   - continue reducing provider assumptions where practical
   - next likely target: introduce a small runtime/registration bundle registry or profile-selection helper so additional named launch packages can be added without growing bootstrap logic again

3. **Treat BTK modernization as a separate upstream R&D lane, not the near-term app path**
   - the missing top-level component entry is only the first symptom
   - the current `src/declarative` path also needs declarative-specific CMake modernization and a real QtScript/CsScript-era dependency story before it can honestly satisfy bobfilez's QML requirements
   - even after that, a forward-compatibility story from `QDeclarative*` toward bobfilez's current `QQml*` bootstrap would still need to be solved explicitly

4. **Continue provider-neutral GUI work**
   - keep prioritizing the work that remains valuable independent of BTK readiness, such as incremental dependency reduction, asset cleanup, and provider-boundary cleanup
