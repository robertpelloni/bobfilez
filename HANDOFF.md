# HANDOFF.md — bobfilez Session 36

## Current Status (2026-04-04)
**Version:** 6.0.21  
**Focus:** QtQuick.Controls dashboard reduction

---

## What Was Done This Session

### 1. Converted `Dashboard.qml` Off `QtQuick.Controls`
- Updated **`gui/omni/assets/Dashboard.qml`**.
- This was the next safe escalation after the shell-host and overlay reductions because it is still shell-adjacent and self-contained, even though it used a slightly richer control set.

### 2. Replaced the Dashboard Controls with Plain Qt Quick Primitives
- Replaced `Label` with `Text` throughout the dashboard.
- Replaced the settings `Button` with a `Rectangle` + `Text` + `MouseArea` + `HoverHandler` shell.
- Replaced both `GroupBox` sections with explicit titled `Text` + `Rectangle` section containers.
- Replaced `ProgressBar` usage with a local inline `SlimProgressBar` helper built from plain rectangles.
- Replaced quick action `Button`s with a local inline `QuickActionChip` helper built from `Rectangle`, `Text`, `Row`, `MouseArea`, and `HoverHandler`.
- Result:
  - `Dashboard.qml` no longer imports `QtQuick.Controls`

### 3. Re-Measured the Controls Footprint
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - previous count: **40 QML files**
  - current count: **39 QML files**
- Also confirmed a larger milestone:
  - **`gui/omni/assets/` now contains zero QML files importing `QtQuick.Controls`**

### 4. Documented the Dashboard Reduction Strategy
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_DASHBOARD_REDUCTION.md`**.
- The document records:
  - why `Dashboard.qml` was a safe richer target
  - how the local helper-component approach replaced controls without a large abstraction jump
  - why the shell-assets layer can now be treated as a completed controls-removal milestone

### 5. Validation and Release Alignment
- Re-ran the normal headless validation path so the versioned state remains current.
- Reconciled release/docs metadata to **6.0.21**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BobUI / Omni shell build | 🟡 Still blocked | BobUI integration is now more structurally correct (real omnicore path + real registration call), but the provider surface still lacks `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` for bobfilez's present GUI targets. |
| BobUI-native migration plan | 🟡 In progress | Four dependency-surface reductions plus six stock-controls reduction passes are complete. The full `gui/omni/assets/` shell-assets layer is now off `QtQuick.Controls`; the remaining imports live in routed panel files. Removing `QtQuick` itself is still incompatible with current BobUI because BobUI widgets are Quick-based. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current Qt/QML/Omni direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BobUI path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Use the improved structural wiring as the new baseline**
   - future BobUI GUI probing should assume the current `omnicore` path and the real `OmniUI::registerQmlTypes()` startup call.

3. **Shift the controls-reduction campaign into routed panels**
   - the shell-assets layer is now complete, so the next step is to select the simplest routed panel candidates rather than the heaviest feature panels.

4. **Do not plan around removing `QtQuick` yet**
   - the current BobUI implementation itself is Quick-based, so a true no-Quick target is premature.
