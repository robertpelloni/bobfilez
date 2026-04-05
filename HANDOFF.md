# HANDOFF.md — bobfilez Session 46

## Current Status (2026-04-05)
**Version:** 6.0.31  
**Focus:** BTK native provider probe round 4 — experimental Declarative enablement confirms stale module and missing QtScript dependency

---

## What Was Done This Session

### 1. Probed BTK Declarative Enablement Directly
- Verified again that bobfilez's active native entrypoint still depends on a real QML stack:
  - **`gui/omni/src/main.cpp`** uses `QGuiApplication`, `QQmlApplicationEngine`, and `qmlRegisterType(...)`
- Inspected the BTK source tree and confirmed:
  - **`libs/btk/src/declarative/`** exists
  - the top-level BTK component list still does **not** include `Declarative`
- To determine whether that omission was superficial or structural, temporarily re-enabled `Declarative` in BTK's top-level component list as a probe.

### 2. Confirmed the BTK Declarative Module is Stale Even Before Deep Compilation
- The first declarative-enable configure attempt failed immediately on stale declarative-specific CMake usage:
  - obsolete `target_add_definitions(...)`
- Applied the smallest temporary modernization necessary to continue probing:
  - switched to `target_compile_definitions(...)`
  - fixed the incorrect `function_generate_resources("${DECLARATIVE_SOURCES}")` usage by modernizing it toward the current target-based module pattern
- This was deliberately treated as an experiment only, not as a claimed production-ready upstream declarative revival.

### 3. Reached the Deeper Compile-Time Boundary
- After the minimal declarative CMake modernization, BTK configured successfully with `Declarative` enabled and began compiling declarative sources.
- The experiment then failed on stronger evidence that the module is not currently viable in this BTK branch:
  - obsolete declarative metatype usage requiring `CS_DECLARE_METATYPE(TYPE)` instead of `Q_DECLARE_METATYPE(TYPE)`
  - fatal missing QtScript-era headers, especially:
    - **`QtScript/qscriptvalue.h`**
- This confirmed the blocker is broader than a missing top-level component toggle.

### 4. Preserved a Buildable BTK Submodule State After the Experiment
- While working against the newer upstream BTK tip, restored the normal BTK MSVC build by fixing a regressed property-name path in:
  - **`libs/btk/src/gui/kernel/qapplication_cs.cpp`**
- Reverted the temporary declarative-enable experiment changes so the normal BTK component set remains buildable.
- Re-ran **`scripts/build_btk_inplace.bat`** and confirmed the normal BTK module set returns to a successful MSVC build.
- Pushed the corrected BTK commit to the existing dedicated branch:
  - **`pi/msvc-focus-fixes-20260405`**

### 5. Documented the Probe Round 4 Findings
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND4.md`**.
- The document records:
  - the temporary declarative-enable experiment
  - the stale declarative-specific CMake findings
  - the deeper compile-time QtScript dependency gap
  - the conclusion that BTK's current Declarative/QML path is structurally incomplete in this branch

### 6. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.31**.
- Updated:
  - **`VERSION.md`**
  - **`core/include/fo/core/version.hpp`**
  - **`CHANGELOG.md`**
  - **`HANDOFF.md`**

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BTK-backed GUI / Omni build | 🟡 Still blocked | BTK now builds successfully for its normal enabled module set, but bobfilez's QML-based GUI still cannot configure because `Declarative` is not exported by the active BTK package surface. |
| BTK Declarative/QML support | 🔴 Deeper upstream/provider readiness gap confirmed | Round 4 proved the issue is broader than a missing component-list entry: when `Declarative` is experimentally re-enabled, the module immediately hits stale declarative-specific CMake integration, obsolete metatype declarations, and fatal missing QtScript-era headers such as `QtScript/qscriptvalue.h`. |
| BTK native migration plan | 🟡 In progress | Active BobUI-specific provider/bootstrap assumptions remain removed from bobfilez, but the remaining blocker is now whether BTK can provide the QML/Declarative layer bobfilez still depends on. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Treat BTK MSVC build correctness as validated for the currently enabled BTK modules**
   - the old `btkinputowner.*` blocker is resolved
   - `CsCore2.1.lib` and related outputs now exist

2. **Treat BTK Declarative revival as a substantial upstream task, not a trivial toggle**
   - the missing top-level component entry is only the first symptom
   - the current `src/declarative` path also needs declarative-specific CMake modernization and a real QtScript/CsScript-era dependency story before it can honestly satisfy bobfilez's QML requirements

3. **Keep the bobfilez BTK consumer path honest**
   - do not pretend BTK is a full QML-capable provider until `Declarative` actually exists in the built package surface

4. **Resume additional GUI dependency-reduction only if BTK investigation pauses**
   - if the framework-path investigation stalls, the next productive lane is still incremental `QtQuick.Controls` reduction in richer routed panels
