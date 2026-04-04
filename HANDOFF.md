# HANDOFF.md — bobfilez Session 26

## Current Status (2026-04-04)
**Version:** 6.0.11  
**Focus:** Markdown WebEngine removal (phase 1 dependency reduction)

---

## What Was Done This Session

### 1. Removed the Explicit WebEngine Path from Markdown Preview
- Added a native preview item:
  - `gui/omni/src/NativeMarkdownView.h`
  - `gui/omni/src/NativeMarkdownView.cpp`
- The new preview uses:
  - `QQuickPaintedItem`
  - `QTextDocument`
  - `fo::core::MarkdownRenderer`
- This replaces the old browser-style preview path in the markdown panel.

### 2. Reworked the Markdown Panel
- Updated **`gui/panels/MarkdownViewerPanel.qml`** to remove:
  - `import QtWebEngine 1.9`
  - `WebEngineView`
- Added:
  - `import Omni.Native 1.0`
  - native `MarkdownView` usage
- Preserved basic panel behavior by keeping:
  - source editor
  - word count / reading time / title wiring
  - simple TOC generation
- Accepted that this is a simplification step rather than full feature parity with JS-driven browser rendering.

### 3. Removed `WebEngineQuick` from the Omni Target
- Updated **`gui/omni/CMakeLists.txt`** to remove:
  - `WebEngineQuick` from `find_package(Qt6 ...)`
  - `Qt6::WebEngineQuick` from linking
- Updated **`gui/omni/src/main.cpp`** to register the new native markdown preview QML type.
- Updated **`gui/CMakeLists.txt`** so the shared GUI target also includes the new source file.

### 4. Validated the Intended Architectural Result
- Confirmed there are no remaining `QtWebEngine`, `WebEngineView`, or `WebEngineQuick` references under `gui/`.
- Re-ran the BobUI consumer probe against the in-place BobUI build tree.
- Result:
  - the remaining blocker is still `Qt6Qml`
  - **not** `WebEngineQuick`
- This proves the dependency reduction worked as intended and did not change the deeper BobUI declarative-stack blocker already identified.

### 5. Updated Interface and Implementation Documentation
- Updated **`core/include/fo/core/markdown_viewer_interface.hpp`** comments to describe browser/WebEngine rendering as optional rather than mandatory.
- Added **`docs/ai/implementation/MARKDOWN_WEBENGINE_REMOVAL.md`** documenting the implementation, feature tradeoffs, and why this was the right phase-one cut.

### 6. Documentation and Release Alignment
- Reconciled release/docs metadata to **6.0.11**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BobUI / Omni shell build | 🟡 Still blocked | BobUI exports a top-level `Qt6Config.cmake` in its in-place build tree, but the current BobUI repo still lacks `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` for bobfilez's present GUI targets. `WebEngineQuick` is no longer part of the immediate blocker set after this session's markdown change. |
| BobUI-native migration plan | 🟡 In progress | The first recommended cut (`WebEngineQuick`) is now demonstrated. Removing most QML is still expensive; removing `QtQuick` itself is still incompatible with current BobUI because BobUI widgets are Quick-based. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current Qt/QML/Omni direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BobUI path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Take the next smallest dependency cut**
   - after `WebEngineQuick`, audit and replace `QtCharts` next
   - then attack `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects`

3. **Do not plan around removing `QtQuick` yet**
   - the current BobUI implementation itself is Quick-based, so a true no-Quick target is premature.

4. **Use the BobUI scripts as current probes**
   - `scripts/build_bobui_inplace.bat`
   - `scripts/build_bobui_gui.bat`
