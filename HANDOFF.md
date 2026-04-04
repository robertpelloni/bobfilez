# HANDOFF.md — bobfilez Session 34

## Current Status (2026-04-04)
**Version:** 6.0.19  
**Focus:** QtQuick.Controls OmniPeek reduction

---

## What Was Done This Session

### 1. Converted `OmniPeekOverlay.qml` Off `QtQuick.Controls`
- Updated **`gui/omni/assets/OmniPeekOverlay.qml`**.
- This was chosen as the next safe target because it is self-contained, visually important, and its stock controls were still replaceable with local `QtQuick` primitives.

### 2. Replaced Stock Controls with Plain Qt Quick Primitives
- Replaced `Label` with `Text` throughout the overlay.
- Replaced header/action `Button`s with `Rectangle` + `Text` + `MouseArea` + `HoverHandler` shells.
- Replaced the model-viewer control buttons with a small lightweight repeater of custom button shells.
- Replaced the text viewer stack:
  - `ScrollView` → `Flickable`
  - `TextArea` → read-only `TextEdit`
- Result:
  - `OmniPeekOverlay.qml` no longer imports `QtQuick.Controls`

### 3. Re-Measured the Controls Footprint
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - previous count: **42 QML files**
  - current count: **41 QML files**
- This proves the reduction strategy can now handle a richer overlay surface, not just shell chrome.

### 4. Documented the OmniPeek Conversion Strategy
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_OMNIPEEK_REDUCTION.md`**.
- The document records:
  - the replacement set
  - why OmniPeek was still a safe incremental target
  - what manual behavior is now owned directly
  - what heavier panel categories remain intentionally deferred

### 5. Validation and Release Alignment
- Re-ran the normal headless validation path so the versioned state remains current.
- Reconciled release/docs metadata to **6.0.19**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BobUI / Omni shell build | 🟡 Still blocked | BobUI integration is now more structurally correct (real omnicore path + real registration call), but the provider surface still lacks `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` for bobfilez's present GUI targets. |
| BobUI-native migration plan | 🟡 In progress | Four dependency-surface reductions plus four stock-controls reduction passes are complete. Removing most QML is still expensive; removing `QtQuick` itself is still incompatible with current BobUI because BobUI widgets are Quick-based. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current Qt/QML/Omni direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BobUI path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Use the improved structural wiring as the new baseline**
   - future BobUI GUI probing should assume the current `omnicore` path and the real `OmniUI::registerQmlTypes()` startup call.

3. **Continue the stock controls reduction incrementally**
   - keep targeting self-contained shell and overlay surfaces before touching richer panel forms with heavy `ComboBox` / `CheckBox` / `GroupBox` / `ProgressBar` / `Menu` usage.

4. **Do not plan around removing `QtQuick` yet**
   - the current BobUI implementation itself is Quick-based, so a true no-Quick target is premature.
