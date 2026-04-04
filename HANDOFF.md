# HANDOFF.md — bobfilez Session 28

## Current Status (2026-04-04)
**Version:** 6.0.13  
**Focus:** GraphicalEffects import cleanup (phase 3 dependency reduction)

---

## What Was Done This Session

### 1. Audited Real Versus Dead GraphicalEffects Usage
- Scanned all QML files for:
  - `import QtGraphicalEffects 1.15`
  - `import Qt5Compat.GraphicalEffects 1.15`
  - actual effect item usage such as `DropShadow` / `FastBlur`
- Important finding:
  - the raw import count overstated the true dependency footprint
  - only a small set of files still actually instantiate effect items

### 2. Removed Dead Effect Imports Safely
- Removed unused effect imports from 27 QML files that did **not** actually instantiate effect primitives.
- Left the real effect-using files untouched for now, including:
  - `AcrylicBackground.qml`
  - `Dashboard.qml`
  - `ExplorerWindow.qml`
  - `OmniPeekOverlay.qml`
  - `Taskbar.qml`
  - `OmniVersePanel.qml`
- This preserves visual behavior while shrinking the dependency surface and making the next replacement phase cleaner.

### 3. Verified the Remaining Surface Is More Accurate
- Confirmed the remaining GraphicalEffects imports now correspond to actual effect usage rather than stale imports.
- This is the third completed dependency-surface reduction after:
  1. markdown `WebEngineQuick` removal
  2. `QtCharts` removal

### 4. Documentation and Release Alignment
- Added **`docs/ai/implementation/GRAPHICALEFFECTS_IMPORT_CLEANUP.md`** documenting the audit, exact cleaned file set, and the remaining real effect users.
- Reconciled release/docs metadata to **6.0.13**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BobUI / Omni shell build | 🟡 Still blocked | BobUI exports a top-level `Qt6Config.cmake` in its in-place build tree, but the current BobUI repo still lacks `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` for bobfilez's present GUI targets. `WebEngineQuick`, `QtCharts`, and many stale GraphicalEffects imports are no longer part of the immediate blocker/noise set. |
| BobUI-native migration plan | 🟡 In progress | Three dependency-surface reductions are now complete: `WebEngineQuick`, `QtCharts`, and dead GraphicalEffects imports. Removing most QML is still expensive; removing `QtQuick` itself is still incompatible with current BobUI because BobUI widgets are Quick-based. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current Qt/QML/Omni direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BobUI path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Replace the remaining real effect users next**
   - the surviving GraphicalEffects set is now small and well-defined:
     - `AcrylicBackground.qml`
     - `Dashboard.qml`
     - `ExplorerWindow.qml`
     - `OmniPeekOverlay.qml`
     - `Taskbar.qml`
     - `OmniVersePanel.qml`

3. **Do not plan around removing `QtQuick` yet**
   - the current BobUI implementation itself is Quick-based, so a true no-Quick target is premature.

4. **Use the BobUI scripts as current probes**
   - `scripts/build_bobui_inplace.bat`
   - `scripts/build_bobui_gui.bat`
