# HANDOFF.md — bobfilez Session 48

## Current Status (2026-04-05)
**Version:** 6.0.33  
**Focus:** BTK framework decision matrix — converted probe history into a recommended app strategy and upstream R&D split

---

## What Was Done This Session

### 1. Converted the BTK Probe Series into a Real Architecture Decision Artifact
- Added **`docs/ai/design/BTK_FRAMEWORK_DECISION_MATRIX.md`**.
- This document turns the prior BTK probe series into a decision-ready framework strategy matrix rather than leaving the project with only accumulating negative findings.

### 2. Compared the Four Real Strategic Paths
- The matrix evaluates four practical options:
  1. modernize BTK upstream toward a `QQml*` / `QQuick*`-class provider
  2. port bobfilez backward toward BTK's `QDeclarative*` generation
  3. keep bobfilez on a modern QQml-style shell path while decoupling provider assumptions
  4. replace the QML shell architecture entirely
- Scored each option against:
  - preservation of current shell work
  - near-term delivery risk
  - long-term coherence
  - upstream engineering burden
  - app-side regression risk
  - strategic optionality
  - fit with the actual BTK evidence gathered so far

### 3. Made the Recommendation Explicit
- The new documented recommendation is:
  - **primary app strategy:** keep bobfilez on its modern QQml-style shell path and decouple provider assumptions
  - **optional parallel R&D strategy:** treat BTK modernization as a separate upstream framework effort rather than as the immediate bobfilez GUI runtime target
- Also documented explicit non-recommendations:
  - do not port bobfilez backward to `QDeclarative*`
  - do not replace the QML shell architecture unless forced much later

### 4. Added a Phased Plan and Mermaid Decision Flow
- The design document now includes:
  - a phased action plan
  - an explicit mermaid decision flow
  - a clear separation between app strategy and upstream BTK R&D
- This gives future agents a concrete path instead of restarting the same provider debate from scratch.

### 5. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.33**.
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
| Recommended project strategy | 🟢 Decision documented | Round 6 formalizes the least-destructive path: keep bobfilez on a modern QQml-style shell path, reduce provider coupling, and treat BTK modernization as a separate upstream R&D effort rather than the immediate app runtime target. |
| BTK native migration plan | 🟡 In progress | Active BobUI-specific provider/bootstrap assumptions remain removed from bobfilez, but the remaining blocker is now whether BTK can provide the QML/Declarative layer bobfilez still depends on. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Treat BTK MSVC build correctness as validated for the currently enabled BTK modules**
   - the old `btkinputowner.*` blocker is resolved
   - `CsCore2.1.lib` and related outputs now exist

2. **Adopt Option C from the framework decision matrix as the primary app strategy**
   - keep bobfilez aligned to a modern QQml-style shell path
   - reduce provider assumptions where practical
   - stop treating BTK as the immediate native runtime target for the active shell bootstrap

3. **Treat BTK modernization as a separate upstream R&D lane, not the near-term app path**
   - the missing top-level component entry is only the first symptom
   - the current `src/declarative` path also needs declarative-specific CMake modernization and a real QtScript/CsScript-era dependency story before it can honestly satisfy bobfilez's QML requirements
   - even after that, a forward-compatibility story from `QDeclarative*` toward bobfilez's current `QQml*` bootstrap would still need to be solved explicitly

4. **Resume additional provider-neutral GUI work**
   - continue only the work that remains valuable independent of BTK readiness, such as incremental dependency reduction, asset cleanup, and provider-boundary cleanup
