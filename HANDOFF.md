# HANDOFF.md — bobfilez Session 25

## Current Status (2026-04-04)
**Version:** 6.0.10  
**Focus:** BobUI native migration cost audit

---

## What Was Done This Session

### 1. Audited the Real Native UI Surface
- Measured the current bobfilez native shell/UI layer and confirmed it is still overwhelmingly QML-driven.
- Quantified the current surface as:
  - **49 QML files**
  - **9,844 QML lines**
  - **39 route-driven shell/panel surfaces** hosted by `gui/omni/assets/main.qml`
- Confirmed the C++ bridge layer is still thin and mostly limited to model exposure (`FileModel`, `TreemapModel`).

### 2. Separated the Migration Questions Properly
- Audited the cost of removing each dependency category separately instead of treating them as one problem:
  - `QtWebEngine`
  - `QtCharts`
  - `QtGraphicalEffects`
  - `QtQuick.Controls`
  - QML composition itself
  - QtQuick runtime itself
- This revealed that these have very different costs and should not be planned as one giant rewrite blob.

### 3. Confirmed the Most Important Architectural Constraint
- Inspected BobUI/OmniUI widget classes and confirmed current BobUI primitives are based on:
  - `QQuickItem`
  - `QQuickPaintedItem`
- Therefore:
  - a BobUI-first migration does **not** currently remove the `QtQuick` dependency
  - it can reduce dependence on stock controls/effects/web modules
  - but it does not support a true "no QtQuick" end state with the current BobUI architecture

### 4. Confirmed bobfilez Is Not Yet Truly BobUI-Driven
- Verified that current bobfilez bootstrap does **not** call BobUI's `OmniUI::registerQmlTypes()`.
- Current shell startup registers only:
  - `FileModel`
  - `TreemapModel`
- This means bobfilez is still primarily a **stock Qt Quick shell** with selective/partial BobUI intent, not a finished BobUI-native shell.

### 5. Produced the Migration Recommendation
- Added **`docs/ai/implementation/BOBUI_NATIVE_MIGRATION_AUDIT.md`**.
- Recommendation captured there:
  1. remove `WebEngineQuick` first
  2. reduce nonessential stock Qt modules next
  3. adopt BobUI widgets/layouts incrementally while keeping QML as a composition layer for now
  4. do **not** try to remove QtQuick entirely unless BobUI itself evolves beyond its current Quick-based architecture

### 6. Documentation and Release Alignment
- Reconciled release/docs metadata to **6.0.10**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BobUI / Omni shell build | 🟡 Still blocked | BobUI exports a top-level `Qt6Config.cmake` in its in-place build tree, but the current BobUI repo still lacks `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` / `Qt6WebEngineQuick` for bobfilez's present GUI targets. |
| BobUI-native migration plan | 🟡 Now clarified | Removing `WebEngineQuick` is realistic; removing most QML is expensive; removing `QtQuick` itself is not compatible with current BobUI because BobUI widgets are Quick-based. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current Qt/QML/Omni direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BobUI path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Target the dependency cuts in the right order**
   - first `WebEngineQuick`
   - then `QtCharts` / effects / stock controls
   - only later revisit wholesale QML reduction

3. **Do not plan around removing `QtQuick` yet**
   - the current BobUI implementation itself is Quick-based, so a true no-Quick target is premature.

4. **Use the BobUI scripts as current probes**
   - `scripts/build_bobui_inplace.bat`
   - `scripts/build_bobui_gui.bat`
