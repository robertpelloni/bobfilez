# HANDOFF.md — bobfilez Session 38

## Current Status (2026-04-04)
**Version:** 6.0.23  
**Focus:** QtQuick.Controls Swarm panel reduction

---

## What Was Done This Session

### 1. Converted `SwarmPanel.qml` Off `QtQuick.Controls`
- Updated **`gui/panels/SwarmPanel.qml`**.
- This was selected as the next routed-panel target because it is especially dashboard-like and avoids richer control families such as `Popup`, `Menu`, `ComboBox`, `TextField`, `ToolTip`, and `Slider`.

### 2. Replaced the Panel Controls with Plain Qt Quick Primitives
- Replaced `Label` with `Text` throughout the panel.
- Replaced action `Button`s with a local `PanelButton` helper built from `Rectangle`, `Text`, `HoverHandler`, and `MouseArea`.
- Replaced the `GroupBox` detected-nodes section with explicit titled `Text` + `Rectangle` section containers.
- Replaced the `ProgressBar` throughput indicator with a local `SlimProgressBar` helper.
- Added a small local `statusColor(status)` helper to keep delegate styling readable without scattering inline conditionals.
- Result:
  - `SwarmPanel.qml` no longer imports `QtQuick.Controls`

### 3. Re-Measured the Controls Footprint
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - previous count: **38 QML files**
  - current count: **37 QML files**
- This is the second successful quantified reduction inside the routed-panel layer.

### 4. Documented the Routed-Panel Pattern Reinforcement
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_SWARM_REDUCTION.md`**.
- The document records:
  - why `SwarmPanel.qml` was a cleaner next target than richer panels
  - how the local helper strategy preserved readability
  - why this strengthens the pattern of targeting dashboard-style panels first

### 5. Validation and Release Alignment
- Re-ran the normal headless validation path so the versioned state remains current.
- Reconciled release/docs metadata to **6.0.23**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BobUI / Omni shell build | 🟡 Still blocked | BobUI integration is now more structurally correct (real omnicore path + real registration call), but the provider surface still lacks `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` for bobfilez's present GUI targets. |
| BobUI-native migration plan | 🟡 In progress | Four dependency-surface reductions plus eight stock-controls reduction passes are complete. The shell-assets layer is complete, and two routed-panel conversions (`DigitalRotPanel.qml`, `SwarmPanel.qml`) are now done. Remaining imports are concentrated in other routed panels, many of which are denser and will require continued selective targeting. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current Qt/QML/Omni direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BobUI path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Use the improved structural wiring as the new baseline**
   - future BobUI GUI probing should assume the current `omnicore` path and the real `OmniUI::registerQmlTypes()` startup call.

3. **Continue routed-panel reductions from simplest to richest**
   - `DigitalRotPanel.qml` and `SwarmPanel.qml` now prove the panel layer is approachable, but future work should still prioritize the most self-contained, dashboard-like panels before menu-heavy or form-heavy surfaces.

4. **Do not plan around removing `QtQuick` yet**
   - the current BobUI implementation itself is Quick-based, so a true no-Quick target is premature.
