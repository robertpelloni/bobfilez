# HANDOFF.md — bobfilez Session 35

## Current Status (2026-04-04)
**Version:** 6.0.20  
**Focus:** QtQuick.Controls main host reduction

---

## What Was Done This Session

### 1. Converted `main.qml` Off `QtQuick.Controls`
- Updated **`gui/omni/assets/main.qml`**.
- The target was the notification-center subsection, which was the last localized stock-controls usage in the shell host itself.

### 2. Replaced the Localized Controls with Plain Qt Quick Primitives
- Replaced `Label` with `Text` in the notification-center overlay.
- Replaced the trivial `Clear All` button with a `Rectangle` + `Text` + `MouseArea` + `HoverHandler` shell.
- Result:
  - `main.qml` no longer imports `QtQuick.Controls`

### 3. Re-Measured the Controls Footprint
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - previous count: **41 QML files**
  - current count: **40 QML files**
- This is another small but real, quantified reduction in the stock-controls surface.

### 4. Documented the Main-Host Reduction Strategy
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_MAIN_REDUCTION.md`**.
- The document records:
  - why `main.qml` was a safe next target
  - what was localized enough to convert without disturbing routed panels
  - why this remains preferable to jumping immediately into richer panel forms

### 5. Validation and Release Alignment
- Re-ran the normal headless validation path so the versioned state remains current.
- Reconciled release/docs metadata to **6.0.20**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BobUI / Omni shell build | 🟡 Still blocked | BobUI integration is now more structurally correct (real omnicore path + real registration call), but the provider surface still lacks `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` for bobfilez's present GUI targets. |
| BobUI-native migration plan | 🟡 In progress | Four dependency-surface reductions plus five stock-controls reduction passes are complete. Removing most QML is still expensive; removing `QtQuick` itself is still incompatible with current BobUI because BobUI widgets are Quick-based. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current Qt/QML/Omni direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BobUI path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Use the improved structural wiring as the new baseline**
   - future BobUI GUI probing should assume the current `omnicore` path and the real `OmniUI::registerQmlTypes()` startup call.

3. **Continue the stock controls reduction incrementally**
   - keep targeting localized shell-host or overlay surfaces before touching rich panel forms with heavy `ComboBox` / `CheckBox` / `GroupBox` / `ProgressBar` / `Menu` usage.

4. **Do not plan around removing `QtQuick` yet**
   - the current BobUI implementation itself is Quick-based, so a true no-Quick target is premature.
