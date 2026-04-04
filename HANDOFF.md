# HANDOFF.md — bobfilez Session 31

## Current Status (2026-04-04)
**Version:** 6.0.16  
**Focus:** QtQuick.Controls initial reduction

---

## What Was Done This Session

### 1. Started the Stock Controls Reduction with Two Shell-Critical Files
- Updated **`gui/omni/assets/Taskbar.qml`**.
- Updated **`gui/omni/assets/StartMenu.qml`**.
- These were chosen because they are high-visibility shell surfaces but only used relatively lightweight control abstractions.

### 2. Removed `QtQuick.Controls` from Both Surfaces
- Replaced `Label` usage with `Text`.
- Replaced trivial text-only `Button` uses with small `Rectangle` + `Text` + `MouseArea` shells.
- Removed the leftover `ToolTip` usage in `Taskbar.qml`.
- Result:
  - both files no longer import `QtQuick.Controls`

### 3. Measured the Remaining Controls Footprint
- Re-ran the QML import audit for `QtQuick.Controls`.
- Result:
  - previous count: **48 QML files**
  - current count: **46 QML files**
- This is a modest but real first cut in the next dependency cluster.

### 4. Documented the Initial Controls Strategy
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_INITIAL_REDUCTION.md`**.
- The document records:
  - why these two files were chosen first
  - what kinds of stock control usage are easy to replace
  - which richer control categories were intentionally deferred

### 5. Validation and Release Alignment
- Re-ran the normal headless validation path and full test suite so the versioned state remains current.
- Reconciled release/docs metadata to **6.0.16**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BobUI / Omni shell build | 🟡 Still blocked | BobUI integration is now more structurally correct (real omnicore path + real registration call), but the provider surface still lacks `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` for bobfilez's present GUI targets. |
| BobUI-native migration plan | 🟡 In progress | Four dependency-surface reductions plus a first stock-controls reduction are complete. Removing most QML is still expensive; removing `QtQuick` itself is still incompatible with current BobUI because BobUI widgets are Quick-based. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current Qt/QML/Omni direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BobUI path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Use the improved structural wiring as the new baseline**
   - future BobUI GUI probing should assume the current `omnicore` path and the real `OmniUI::registerQmlTypes()` startup call.

3. **Continue the stock controls reduction incrementally**
   - next target files should be shell-adjacent surfaces with mostly `Label` / trivial `Button` usage before tackling heavier control sets.

4. **Do not plan around removing `QtQuick` yet**
   - the current BobUI implementation itself is Quick-based, so a true no-Quick target is premature.
