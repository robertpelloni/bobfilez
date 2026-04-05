# HANDOFF.md — bobfilez Session 47

## Current Status (2026-04-05)
**Version:** 6.0.32  
**Focus:** BTK native provider probe round 5 — confirmed QDeclarative-era BTK surface vs QQml-era bobfilez bootstrap mismatch

---

## What Was Done This Session

### 1. Verified the API Generation Used by bobfilez's Active Native GUI Path
- Re-inspected **`gui/omni/src/main.cpp`**.
- Confirmed the current bobfilez GUI bootstrap is explicitly written against the newer QQml-style stack:
  - `QGuiApplication`
  - `QQmlApplicationEngine`
  - `QQmlContext`
  - `qmlRegisterType(...)`
- This means the BTK-provider question is not merely whether some declarative layer exists, but whether the provider exposes the right generation of declarative API.

### 2. Verified BTK's Declarative Surface is QDeclarative-Era, Not QQml/QQuick-Era
- Inspected BTK declarative headers such as:
  - **`libs/btk/src/declarative/qml/qdeclarativeengine.h`**
- Confirmed BTK exposes a `QDeclarative*` family centered on:
  - `QDeclarativeEngine`
  - `QDeclarativeComponent`
  - `QDeclarativeView`
  - `QDeclarativeItem`
- Searched the BTK tree for modern `QQml*` / `QQuick*` surface and found no actual provider classes or headers, including no discovered:
  - `QQmlApplicationEngine`
  - `QQmlEngine`
  - `QQmlContext`
  - `QQuickItem`
  - `QQuickView`
  - `qqml*.h`
  - `qquick*.h`

### 3. Quantified the Scale of the BTK Declarative Revival Gap
- Ran a structural scan over **`libs/btk/src/declarative`** and measured:
  - **61 files** with direct QtScript-related usage
  - **24 files** referencing `QScriptDeclarativeClass`
  - **23 files** still using `Q_DECLARE_METATYPE(...)`
- Also confirmed missing expected internal-script surface signs, including no discovered files such as:
  - `qscriptdeclarativeclass_p.h`
  - `qscriptengine.h`
- This provides a more honest scale estimate for any upstream BTK declarative-revival effort.

### 4. Sharpened the Strategic Conclusion
- The BTK provider gap is now established as three stacked issues:
  1. `Declarative` is not exported by the active BTK package surface
  2. when experimentally re-enabled, the declarative module is stale/incomplete and tied to missing QtScript-era infrastructure
  3. even beyond that, BTK's declarative model is still `QDeclarative*`-era while bobfilez's active GUI bootstrap is `QQml*`-era
- This means the next step is no longer just another build fix. It is now a framework-direction decision.

### 5. Documented the Probe Round 5 Findings
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND5.md`**.
- The document records:
  - the `QQml*` vs `QDeclarative*` mismatch
  - the absence of `QQml*` / `QQuick*` provider surface in BTK
  - the quantified declarative/QtScript dependency footprint
  - the conclusion that a revived BTK declarative module would still not directly satisfy bobfilez's current GUI bootstrap

### 6. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.32**.
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
   - even after that, a forward-compatibility story from `QDeclarative*` toward bobfilez's current `QQml*` bootstrap would still need to be solved explicitly

3. **Keep the bobfilez BTK consumer path honest**
   - do not pretend BTK is a full QML-capable provider until `Declarative` actually exists in the built package surface
   - and do not assume that a revived `QDeclarative*` surface automatically satisfies a `QQmlApplicationEngine`-based application architecture

4. **Resume additional GUI dependency-reduction only if BTK investigation pauses**
   - if the framework-path investigation stalls, the next productive lane is still incremental `QtQuick.Controls` reduction in richer routed panels
