# Markdown WebEngine Removal — Phase 1 Native UI Simplification

## Summary
This session removed the explicit `QtWebEngine` / `WebEngineQuick` dependency from bobfilez's markdown preview path and replaced it with a native preview item.

## Why this was the right first cut
The earlier BobUI migration audit recommended removing the smallest high-value blocker first.

Among the current GUI dependencies, `QtWebEngineQuick` was the cleanest candidate because:
- it had a narrow visible usage footprint
- it introduced a heavyweight browser runtime for one panel
- it was one of the clearest mismatches with the current BobUI-in-place build tree

## What changed

### 1. New native preview item
Added:
- `gui/omni/src/NativeMarkdownView.h`
- `gui/omni/src/NativeMarkdownView.cpp`

This preview item:
- derives from `QQuickPaintedItem`
- accepts raw markdown text from QML
- uses the existing core `fo::core::MarkdownRenderer`
- renders into a native `QTextDocument`
- exposes basic document stats back to QML:
  - word count
  - reading time
  - document title
- supports wheel scrolling
- preserves a no-op `scrollToAnchor()` API so the surrounding panel structure does not break

## 2. Markdown panel no longer imports QtWebEngine
Updated:
- `gui/panels/MarkdownViewerPanel.qml`

Changes:
- removed:
  - `import QtWebEngine 1.9`
  - `WebEngineView`
- added:
  - `import Omni.Native 1.0`
  - `MarkdownView`
- replaced browser-based preview loading with native preview binding
- added lightweight heading parsing in QML to keep TOC generation alive in a simple form

## 3. Omni target no longer links WebEngineQuick
Updated:
- `gui/omni/CMakeLists.txt`

Changes:
- removed `WebEngineQuick` from required Qt components
- removed `Qt6::WebEngineQuick` from target link libraries
- added `src/NativeMarkdownView.cpp`

## 4. Shared GUI target adjusted too
Updated:
- `gui/CMakeLists.txt`

Because `fo_gui` also reuses `gui/omni/src/main.cpp`, it now includes:
- `omni/src/NativeMarkdownView.cpp`
- `omni/src/NativeMarkdownView.h`

## 5. Bootstrap registration
Updated:
- `gui/omni/src/main.cpp`

Registered:
- `qmlRegisterType<fo::gui::NativeMarkdownView>("Omni.Native", 1, 0, "MarkdownView")`

## 6. Interface comments corrected
Updated:
- `core/include/fo/core/markdown_viewer_interface.hpp`

The interface comments now describe browser/WebEngine rendering as an optional frontend path rather than the only rendering model.

---

## Validation performed

### Code-path validation
- Confirmed there are now **no `QtWebEngine`, `WebEngineView`, or `WebEngineQuick` references** remaining under `gui/`.

### Headless safety
- Re-ran:
  - `scripts/build_headless.bat`
- Headless build remains unaffected.

### BobUI consumer probe
- Re-ran the BobUI-backed GUI consumer probe against the in-place BobUI build tree.
- Result:
  - the failure remains at `Qt6Qml`
  - **not** at `WebEngineQuick`

That is the expected and desired outcome for this phase.

---

## Tradeoffs / regressions
This is a deliberate simplification step, so the new native preview is not feature-identical to the old browser-oriented design.

### Likely reduced or absent in the new preview
- Mermaid JS execution
- KaTeX runtime math rendering
- highlight.js browser syntax highlighting
- browser DOM anchor scrolling
- arbitrary embedded HTML/JS behavior

### Preserved or improved
- native preview path
- lower dependency weight
- simpler compatibility story with a BobUI-first future
- markdown preview no longer depends on a browser engine module

## Practical architectural impact
This change proves that at least one of the recommended migration steps from the audit is viable:
- **remove `QtWebEngineQuick` first**

It also sharpens the remaining blocker:
- bobfilez is now less dependent on heavy optional GUI modules
- but the current BobUI tree still cannot satisfy the required declarative stack because `Qt6Qml` is missing

So the next migration questions are now cleaner.

## Recommended next steps
1. Re-run the same simplification approach for:
   - `QtCharts`
   - `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects`
2. Decide whether the markdown preview needs richer native features later, or whether this lighter preview is sufficient.
3. Continue moving the shell toward BobUI widgets while avoiding massive rewrites until the remaining module surface is better understood.
