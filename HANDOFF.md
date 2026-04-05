# HANDOFF.md — bobfilez Session 50

## Current Status (2026-04-05)
**Version:** 6.0.35  
**Focus:** Native UI runtime policy extraction — second concrete Option C step to separate bootstrap, runtime, and registration policy

---

## What Was Done This Session

### 1. Executed the Second Real Option C Refactor in Code
- Continued the provider-coupling reduction started by the first bootstrap seam.
- Added:
  - **`gui/omni/src/NativeUiRuntime.hpp`**
  - **`gui/omni/src/NativeUiRuntime.cpp`**
  - **`gui/omni/src/OmniQmlRegistration.hpp`**
  - **`gui/omni/src/OmniQmlRegistration.cpp`**
- Refactored **`gui/omni/src/NativeUiBootstrap.cpp`** so it no longer owns the active runtime implementation class and QML registration policy directly.

### 2. Separated Three Previously Mixed Concerns
- The active shell launch path is now split more cleanly into:
  1. **bootstrap orchestration** (`NativeUiBootstrap.cpp`)
  2. **declarative runtime adapter/factory** (`NativeUiRuntime.cpp/.hpp`)
  3. **QML registration policy** (`OmniQmlRegistration.cpp/.hpp`)
- The active behavior is preserved, but the seams are now narrower and easier to evolve independently.

### 3. Kept the Runtime Problem Honest While Improving Structure
- The active runtime is still the same QQml-based path internally.
- This refactor does **not** claim a second provider implementation.
- It only reduces coupling so future runtime/provider work can be targeted at smaller layers rather than a monolithic bootstrap file.

### 4. Updated GUI Build Wiring for the Extraction
- Updated:
  - **`gui/CMakeLists.txt`**
  - **`gui/omni/CMakeLists.txt`**
- Added the new runtime and registration files to the GUI source lists so the separation is represented honestly in both native GUI targets.

### 5. Documented the Extraction as the Next Option C Milestone
- Added **`docs/ai/implementation/NATIVE_UI_RUNTIME_POLICY_EXTRACTION.md`**.
- The document records:
  - the residual coupling after the first seam
  - the new responsibility split
  - the behavior preserved
  - the limitations that remain unchanged

### 6. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.35**.
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
| Option C execution status | 🟡 In progress | Round 7 extracted the active shell runtime bootstrap out of `main.cpp`, and Round 8 further separated bootstrap orchestration, runtime adapter selection, and QML registration policy into narrower seams. |
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
   - next likely target: introduce a small shell-launch configuration object so main QML URL, failure policy, registration bundle, and runtime factory selection become explicit data rather than implicit bootstrap behavior

3. **Treat BTK modernization as a separate upstream R&D lane, not the near-term app path**
   - the missing top-level component entry is only the first symptom
   - the current `src/declarative` path also needs declarative-specific CMake modernization and a real QtScript/CsScript-era dependency story before it can honestly satisfy bobfilez's QML requirements
   - even after that, a forward-compatibility story from `QDeclarative*` toward bobfilez's current `QQml*` bootstrap would still need to be solved explicitly

4. **Continue provider-neutral GUI work**
   - keep prioritizing the work that remains valuable independent of BTK readiness, such as incremental dependency reduction, asset cleanup, and provider-boundary cleanup
