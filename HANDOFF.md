# HANDOFF.md — bobfilez Session 33

## Current Status (2026-04-04)
**Version:** 6.0.18  
**Focus:** QtQuick.Controls explorer reduction

---

## What Was Done This Session

### 1. Converted `ExplorerWindow.qml` Off `QtQuick.Controls`
- Updated **`gui/omni/assets/ExplorerWindow.qml`**.
- This was the next safe escalation after the earlier shell-adjacent control reductions because it is a larger shell surface but still dominated by relatively lightweight control patterns.

### 2. Replaced Stock Controls with Plain Qt Quick Primitives
- Replaced `Label` with `Text` throughout the explorer shell.
- Replaced toolbar navigation `Button`s with `Rectangle` + `Text` + `MouseArea` + `HoverHandler` shells.
- Replaced the command-bar deduplicate button with a custom lightweight rectangle button.
- Replaced sidebar and file-list `ItemDelegate` usage with rectangle-backed rows plus manual hover/click handling.
- Removed the stock `ScrollBar` attachment from the file list.
- Result:
  - `ExplorerWindow.qml` no longer imports `QtQuick.Controls`

### 3. Re-Measured the Controls Footprint
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - previous count: **43 QML files**
  - current count: **42 QML files**
- The shell-adjacent no-Controls set now includes:
  - `Taskbar.qml`
  - `StartMenu.qml`
  - `DesktopIcons.qml`
  - `NexusPulse.qml`
  - `WindowManager.qml`
  - `ExplorerWindow.qml`

### 4. Documented the Explorer Conversion Strategy
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_EXPLORER_REDUCTION.md`**.
- The document records:
  - what was replaced
  - why this was still a safe step
  - what tradeoffs were accepted
  - why this remains preferable to jumping immediately into richer control-dense panels

### 5. Validation and Release Alignment
- Re-ran the normal headless validation path and full test suite so the versioned state remains current.
- Reconciled release/docs metadata to **6.0.18**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BobUI / Omni shell build | 🟡 Still blocked | BobUI integration is now more structurally correct (real omnicore path + real registration call), but the provider surface still lacks `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` for bobfilez's present GUI targets. |
| BobUI-native migration plan | 🟡 In progress | Four dependency-surface reductions plus three stock-controls reduction passes are complete. Removing most QML is still expensive; removing `QtQuick` itself is still incompatible with current BobUI because BobUI widgets are Quick-based. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current Qt/QML/Omni direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BobUI path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Use the improved structural wiring as the new baseline**
   - future BobUI GUI probing should assume the current `omnicore` path and the real `OmniUI::registerQmlTypes()` startup call.

3. **Continue the stock controls reduction incrementally**
   - keep targeting shell-adjacent files with mostly `Label` / trivial `Button` usage before touching richer controls like `ComboBox`, `ProgressBar`, `GroupBox`, `Menu`, or `TextArea`.

4. **Do not plan around removing `QtQuick` yet**
   - the current BobUI implementation itself is Quick-based, so a true no-Quick target is premature.
