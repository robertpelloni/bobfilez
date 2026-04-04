# HANDOFF.md — bobfilez Session 37

## Current Status (2026-04-04)
**Version:** 6.0.22  
**Focus:** QtQuick.Controls Digital Rot panel reduction

---

## What Was Done This Session

### 1. Converted `DigitalRotPanel.qml` Off `QtQuick.Controls`
- Updated **`gui/panels/DigitalRotPanel.qml`**.
- This was selected as the first routed-panel target because it is visually self-contained, interaction-light, and does not depend on menus, popups, text-entry widgets, or other denser control families.

### 2. Replaced the Panel Controls with Plain Qt Quick Primitives
- Replaced `Label` with `Text` throughout the panel.
- Replaced action `Button`s with a local `PanelButton` helper built from `Rectangle`, `Text`, `HoverHandler`, and `MouseArea`.
- Replaced the `GroupBox` suggestions section with explicit titled `Text` + `Rectangle` section containers.
- Added a local `StatCard` helper for the repeated dashboard-style metric cards.
- Simplified the helper implementation after the first rewrite by exposing a `clicked` signal from `PanelButton` rather than stacking extra mouse handlers in call sites.
- Result:
  - `DigitalRotPanel.qml` no longer imports `QtQuick.Controls`

### 3. Re-Measured the Controls Footprint
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - previous count: **39 QML files**
  - current count: **38 QML files**
- This is the first quantified reduction after moving from shell assets into the routed panel layer.

### 4. Documented the First Routed-Panel Reduction Strategy
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_DIGITAL_ROT_REDUCTION.md`**.
- The document records:
  - why `DigitalRotPanel.qml` was the correct first panel target
  - how the local helper strategy kept the rewrite readable and incremental
  - why this marks a migration-phase shift from shell assets into routed panels

### 5. Validation and Release Alignment
- Re-ran the normal headless validation path so the versioned state remains current.
- Reconciled release/docs metadata to **6.0.22**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BobUI / Omni shell build | 🟡 Still blocked | BobUI integration is now more structurally correct (real omnicore path + real registration call), but the provider surface still lacks `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` for bobfilez's present GUI targets. |
| BobUI-native migration plan | 🟡 In progress | Four dependency-surface reductions plus seven stock-controls reduction passes are complete. The shell-assets layer is complete, and the first routed-panel conversion (`DigitalRotPanel.qml`) is now done. Remaining imports are concentrated in other routed panels, many of which are denser and will require more selective targeting. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current Qt/QML/Omni direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BobUI path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Use the improved structural wiring as the new baseline**
   - future BobUI GUI probing should assume the current `omnicore` path and the real `OmniUI::registerQmlTypes()` startup call.

3. **Continue routed-panel reductions from simplest to richest**
   - `DigitalRotPanel.qml` proves the panel layer is approachable, but future work should still prioritize the most self-contained, interaction-light panels before menu-heavy or form-heavy surfaces.

4. **Do not plan around removing `QtQuick` yet**
   - the current BobUI implementation itself is Quick-based, so a true no-Quick target is premature.
