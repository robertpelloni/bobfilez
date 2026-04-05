# HANDOFF.md — bobfilez Session 49

## Current Status (2026-04-05)
**Version:** 6.0.34  
**Focus:** Native UI bootstrap seam — first concrete Option C implementation step to reduce provider coupling

---

## What Was Done This Session

### 1. Executed the First Real Option C Refactor in Code
- Added a new bootstrap seam for the native shell entrypoint:
  - **`gui/omni/src/NativeUiBootstrap.hpp`**
  - **`gui/omni/src/NativeUiBootstrap.cpp`**
- Refactored **`gui/omni/src/main.cpp`** into a minimal launcher which now delegates to:
  - `fo::gui::run_omni_shell(argc, argv)`
- This moves the active runtime-specific wiring out of `main.cpp` and into a narrower bootstrap layer.

### 2. Reduced Direct Provider Coupling in the Entrypoint
- The active runtime is still the same for now, but the direct dependency surface is reduced.
- The bootstrap layer now owns:
  - `QGuiApplication` creation
  - declarative runtime creation/loading
  - QML type registration for:
    - `FileModel`
    - `TreemapModel`
    - `NativeMarkdownView`
  - root-object failure handling for `qrc:/main.qml`
- This preserves behavior while isolating provider/runtime assumptions behind a smaller seam.

### 3. Aligned GUI Build Wiring with the New Seam
- Updated:
  - **`gui/CMakeLists.txt`**
  - **`gui/omni/CMakeLists.txt`**
- Added the new bootstrap files to the GUI source lists.
- Also aligned the GUI source list to include `TreemapModel.cpp/.h`, matching the shell's QML registration path more honestly.

### 4. Documented the Refactor as the First Option C Execution Milestone
- Added **`docs/ai/implementation/NATIVE_UI_BOOTSTRAP_SEAM.md`**.
- The document records:
  - the original coupling problem
  - what moved out of `main.cpp`
  - the internal runtime seam introduced
  - what behavior remains unchanged
  - what this change does and does not solve

### 5. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.34**.
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
| Option C execution status | 🟡 Started | Round 7 implemented the first code-level provider-coupling reduction by extracting the active shell runtime bootstrap out of `main.cpp` and into a narrower bootstrap seam. |
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
   - next likely target: isolate QML type-registration policy and runtime selection from the bootstrap implementation further

3. **Treat BTK modernization as a separate upstream R&D lane, not the near-term app path**
   - the missing top-level component entry is only the first symptom
   - the current `src/declarative` path also needs declarative-specific CMake modernization and a real QtScript/CsScript-era dependency story before it can honestly satisfy bobfilez's QML requirements
   - even after that, a forward-compatibility story from `QDeclarative*` toward bobfilez's current `QQml*` bootstrap would still need to be solved explicitly

4. **Continue provider-neutral GUI work**
   - keep prioritizing the work that remains valuable independent of BTK readiness, such as incremental dependency reduction, asset cleanup, and provider-boundary cleanup
