# HANDOFF.md — bobfilez Session 51

## Current Status (2026-04-05)
**Version:** 6.0.36  
**Focus:** Native UI launch config — third concrete Option C step to turn launch policy into explicit configuration

---

## What Was Done This Session

### 1. Executed the Third Real Option C Refactor in Code
- Continued the provider-coupling reduction by turning active shell launch policy into explicit configuration data.
- Added:
  - **`gui/omni/src/NativeUiLaunchConfig.hpp`**
  - **`gui/omni/src/NativeUiLaunchConfig.cpp`**
- Refactored **`gui/omni/src/NativeUiBootstrap.cpp`** so it now consumes a launch configuration object instead of hardcoding launch policy inline.

### 2. Made Launch Policy Explicit Instead of Implicit
- Introduced `NativeUiLaunchConfig` with explicit fields for:
  - `main_qml`
  - `register_types`
  - `runtime_factory`
  - `object_created_handler`
- Added `create_default_omni_ui_launch_config()` to centralize the current default shell launch policy.
- The bootstrap now:
  1. builds a launch config
  2. validates it
  3. runs registration policy from config
  4. creates the runtime from config
  5. applies the configured root-object failure handler
  6. loads the configured main QML URL

### 3. Kept Active Behavior the Same While Narrowing the Seams Again
- The active shell still launches the same way for users.
- The internal QQml-based runtime remains unchanged.
- This change does **not** introduce a second runtime, and does **not** change the BTK diagnosis.
- It only moves one more layer of behavior from implicit bootstrap code into explicit launch policy data.

### 4. Updated GUI Build Wiring for the Extraction
- Updated:
  - **`gui/CMakeLists.txt`**
  - **`gui/omni/CMakeLists.txt`**
- Added the new launch-config files to the GUI source lists so the extraction is represented honestly in both native GUI targets.

### 5. Documented the Extraction as the Next Option C Milestone
- Added **`docs/ai/implementation/NATIVE_UI_LAUNCH_CONFIG.md`**.
- The document records:
  - the prior implicit launch-policy problem
  - the new config object
  - the bootstrap flow after the refactor
  - the behavior preserved
  - the limitations that remain unchanged

### 6. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.36**.
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
| Option C execution status | 🟡 In progress | Round 7 extracted the active shell runtime bootstrap out of `main.cpp`, Round 8 separated bootstrap/runtime/registration responsibilities, and Round 9 turned launch policy into an explicit configuration object. |
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
   - next likely target: introduce named launch profiles / runtime bundles so the bootstrap can choose a provider-neutral shell launch package rather than individual config callbacks

3. **Treat BTK modernization as a separate upstream R&D lane, not the near-term app path**
   - the missing top-level component entry is only the first symptom
   - the current `src/declarative` path also needs declarative-specific CMake modernization and a real QtScript/CsScript-era dependency story before it can honestly satisfy bobfilez's QML requirements
   - even after that, a forward-compatibility story from `QDeclarative*` toward bobfilez's current `QQml*` bootstrap would still need to be solved explicitly

4. **Continue provider-neutral GUI work**
   - keep prioritizing the work that remains valuable independent of BTK readiness, such as incremental dependency reduction, asset cleanup, and provider-boundary cleanup
