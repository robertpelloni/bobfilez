# BobUI Native Migration Audit — Removing QML / Quick / WebEngine from bobfilez

## Executive summary
The current bobfilez native UI is still overwhelmingly a **QML shell** with thin C++ model bridges.

That means there are really **three different migration questions**, and they should not be conflated:

1. **Can we remove `Qt6WebEngineQuick`?**
   - **Yes, probably with moderate effort.**
2. **Can we remove most QML business/UI composition and move to BobUI-authored native surfaces?**
   - **Yes, but it is a major rewrite.**
3. **Can we remove `QtQuick` itself and still use current BobUI?**
   - **No, not realistically with the current BobUI implementation.**

Why not? Because current BobUI/OmniUI widgets are themselves based on:
- `QQuickItem`
- `QQuickPaintedItem`

So a "BobUI-only" migration may reduce or replace **QML source files**, but it does **not** currently eliminate the **QtQuick runtime dependency**.

---

## What was inspected

### bobfilez native UI structure
- `gui/omni/assets/main.qml`
- `gui/omni/assets/qml.qrc`
- `gui/omni/src/main.cpp`
- `gui/omni/src/FileModel.*`
- `gui/omni/src/TreemapModel.*`
- `gui/CMakeLists.txt`
- `gui/omni/CMakeLists.txt`
- all `gui/**/*.qml`

### BobUI / OmniUI structure
- `libs/bobui/OmniUI/omnicore/include/*`
- `libs/bobui/OmniUI/omnicore/src/OmniQmlRegistration.cpp`
- representative widgets/layouts such as:
  - `OmniWindow.h`
  - `OmniButton.h`
  - `OmniMarkdownView.h`

---

## Current bobfilez UI dependency surface

### Volume of QML
Current tree contains:
- **49 QML files**
- **9,844 QML lines**

Breakdown:
- `gui/omni/assets/` → **10 files**, **1,369 lines**
- `gui/panels/` → **39 files**, **8,475 lines**
- `gui/omni/src/` → **5 C++ files**, **521 lines**

### Shell routing surface
`gui/omni/assets/main.qml` currently hosts **39 route/panel surfaces** via `shell.activePanel`, including:
- explorer
- search
- rename
- convert
- hex
- image
- md
- watcher
- fileops
- visual_dedup
- topology
- pruner
- hierarchy
- achievements
- cloud
- network
- vault
- forensic
- assets
- omnivision
- omniaudio
- omnigraph
- omniflow
- timemachine
- swarm
- recovery
- notary
- photos
- develop
- oracle
- omnimount
- omnisec
- omniclerk
- omnigit
- omniverse
- omnicrypt
- omnicluster
- terminal
- omnishare

### Actual bootstrap pattern
`gui/omni/src/main.cpp` currently does this:
- creates `QGuiApplication`
- registers only:
  - `FileModel`
  - `TreemapModel`
- loads `qrc:/main.qml` through `QQmlApplicationEngine`

So the native bootstrap is still **QML-first**, not BobUI-widget-first.

---

## Module usage in current QML
Across the 49 QML files, imports are:

- `QtQuick` → **49 files**
- `QtQuick.Controls` → **48 files**
- `QtQuick.Layouts` → **48 files**
- `QtGraphicalEffects` → **31 files**
- `Qt5Compat.GraphicalEffects` → **2 files**
- `QtWebEngine` → **1 file**
- `QtCharts` → **1 file**
- `QtQuick.Dialogs` → **1 file**
- `OmniUI` → **2 files**
- `OmniData` → **2 files**
- `OmniLayout` → **1 file**
- `Omni.File` → **2 files**
- `Omni.Viz` → **1 file**

### Important implication
Most of the shell is still built on **stock Qt Quick Controls** and **Graphical Effects**, not on BobUI widgets.

The current product is therefore not merely "using QML" — it is specifically using:
- stock Quick controls
- stock Quick layouts
- stock Graphical Effects
- one WebEngine surface
- one Charts surface

---

## Existing C++ bridge surface is thin
The actual C++ UI-facing layer in bobfilez is small:

### `FileModel`
- `QAbstractListModel`
- exposes filesystem data and actions to QML
- provides current-path, selection state, file operations, duplicate search hooks

### `TreemapModel`
- `QAbstractListModel`
- exposes `TreemapEngine` output to QML

This is not a native widget hierarchy. It is a **data bridge** into QML.

### Practical consequence
Moving away from QML would mean rebuilding:
- shell window management
- navigation surfaces
- toolbar composition
- panel layout
- repeated delegates/lists/grids
- command surfaces
- state propagation
- theming/effects

in another UI composition style.

---

## BobUI reality check

### BobUI widgets are Quick-based
Representative BobUI headers show:
- `OmniWindow : public QQuickPaintedItem`
- `OmniButton : public QQuickPaintedItem`
- `OmniMarkdownView : public QQuickPaintedItem`
- layout primitives like `OmniGridPane`, `OmniFlowPane`, `OmniStackPane`, `OmniTilePane` derive from `QQuickItem`

### Critical conclusion
With the **current BobUI implementation**, migrating to BobUI does **not** remove the need for:
- `QtQuick`
- scenegraph/QQuick runtime concepts

It may reduce dependency on:
- stock `QtQuick.Controls`
- stock Graphical Effects
- stock WebEngineQuick

But it does **not** currently support a complete removal of `QtQuick` itself.

---

## Another important finding: BobUI QML types are not really wired in bobfilez bootstrap yet
BobUI contains a large QML registration surface in:
- `libs/bobui/OmniUI/omnicore/src/OmniQmlRegistration.cpp`

That file registers types such as:
- `OmniUI.Button`
- `OmniUI.Slider`
- `OmniUI.Dial`
- `OmniUI.ListView`
- `OmniLayout.Window`
- `OmniLayout.GridPane`
- `OmniLayout.FlowPane`
- `OmniGfx.DropShadow`
- `OmniData.*`

However, current bobfilez bootstrap does **not** call:
- `OmniUI::registerQmlTypes()`

Current `gui/omni/src/main.cpp` registers only:
- `FileModel`
- `TreemapModel`

### Why this matters
Some panels already import BobUI namespaces such as:
- `OmniUI 1.0`
- `OmniData 1.0`
- `OmniLayout 1.0`

Examples:
- `PhotoDevelopPanel.qml`
- `PhotoLibraryPanel.qml`

Those panels are therefore not evidence of a finished BobUI migration; they are evidence of a **partially attempted / not fully wired** migration.

---

## Dependency-removal difficulty by category

### 1. Remove `Qt6WebEngineQuick`
**Difficulty: Low to Medium**

Why:
- only one obvious current QML consumer: `MarkdownViewerPanel.qml`
- that panel uses `QtWebEngine` for HTML preview (`WebEngineView`)

Possible replacements:
- BobUI's `OmniMarkdownView` (if sufficient for product needs)
- `QTextDocument` / `QTextBrowser` style fallback in a QWidget/QQuickPaintedItem wrapper
- external markdown rendering to static rich text
- reduced-feature preview (no Mermaid/KaTeX/JS runtime)

### 2. Remove `QtCharts`
**Difficulty: Low**

Why:
- only one panel currently imports `QtCharts`
- likely replaceable by BobUI’s `OmniPlot` or a custom chart primitive

### 3. Remove `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects`
**Difficulty: Medium**

Why:
- used in **33 files**
- but effects are mostly stylistic: shadows, glow, acrylic-like polish

Potential replacement path:
- BobUI `OmniGfx.DropShadow`
- BobUI shader/effect components
- simplified native drawing until effect parity returns

### 4. Remove stock `QtQuick.Controls`
**Difficulty: High**

Why:
- used in **48 files**
- shell uses `ApplicationWindow`, `Button`, `Label`, `Menu`, `ScrollView`, `ListView`, `GridView`, `GroupBox`, etc.
- replacing these requires broad migration to BobUI equivalents or custom Quick items

This is likely the **largest practical migration step** short of deleting QML entirely.

### 5. Remove most QML source files while keeping QtQuick runtime
**Difficulty: Very High**

Why:
- current UI composition is encoded directly in ~10k lines of QML
- C++ bridge layer is minimal
- native equivalents would need to be rewritten panel-by-panel

This is possible, but it is effectively a **full frontend rewrite**.

### 6. Remove `QtQuick` itself
**Difficulty: Not realistic with current BobUI**

Why:
- BobUI itself is Quick-based today
- its widgets/layouts are `QQuickItem` / `QQuickPaintedItem`

So unless BobUI grows a non-Quick rendering/runtime surface, "no QtQuick" is the wrong target for the current architecture.

---

## Effort estimate by migration strategy

### Strategy A — Minimal cleanup: make current bobfilez more BobUI-compatible
**Recommended first step**

Goals:
- remove `WebEngineQuick`
- remove `QtCharts`
- reduce `GraphicalEffects`
- start using BobUI widgets where available

Estimated cost:
- **moderate**
- high ROI
- does not require rewriting all 39 panel routes immediately

### Strategy B — Keep QML, but replace stock controls/layouts with BobUI types
**Most realistic BobUI-first path**

Goals:
- preserve QML composition
- replace stock controls with:
  - `OmniUI.Button`
  - `OmniUI.Slider`
  - `OmniUI.ListView`
  - `OmniLayout.*`
  - `OmniGfx.*`
- gradually convert panels

Estimated cost:
- **high**, but incremental and survivable

### Strategy C — Rewrite shell and panels into BobUI-authored C++ scene objects
**Possible, but expensive**

Goals:
- greatly reduce or eliminate large QML asset set
- own shell composition in C++

Estimated cost:
- **very high**
- likely requires re-architecting state propagation and composition from scratch

### Strategy D — Eliminate QtQuick entirely
**Not recommended right now**

Requires:
- BobUI architectural evolution, not just bobfilez changes

---

## Recommended migration plan

### Phase 0 — Fix BobUI wiring truthfully
1. Ensure BobUI QML registration is actually integrated where intended.
2. Audit which `OmniUI` imports currently work versus which are aspirational only.
3. Stop assuming the existing shell is already BobUI-native; document exact hybrid state.

### Phase 1 — Reduce the hardest external Qt modules first
1. Replace `MarkdownViewerPanel.qml` WebEngine dependency.
2. Replace `QtCharts` usage in `EnhancedFileOpsPanel.qml`.
3. Replace `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects` with BobUI effect primitives where possible.

### Phase 2 — Migrate controls and layout primitives, not composition wholesale
1. Keep QML for now.
2. Replace stock controls with BobUI controls panel-by-panel.
3. Introduce BobUI layout primitives into shell surfaces where stable.

### Phase 3 — Revisit whether large QML files should remain
1. After BobUI widget usage is real and stable, decide whether the remaining QML is acceptable.
2. Only then consider moving selected panels or shell frames into heavier C++ composition.

### Phase 4 — Reconsider the true end state
At that point the project can decide whether it wants:
- **QML + BobUI controls**
- **C++-heavy Quick scenegraph app**
- or some future BobUI architecture that no longer depends on Quick at all

---

## Final recommendation
### Recommended answer to “how hard is it?”
- **Remove WebEngineQuick:** feasible now
- **Reduce nonessential stock Qt modules:** feasible in phases
- **Go BobUI-first while keeping QML initially:** the best path
- **Rewrite away from QML immediately:** expensive
- **Remove QtQuick entirely:** not compatible with current BobUI architecture

### Best next move
The best next engineering move is **not** "delete QML".

It is:
1. **reduce the module surface** (`WebEngineQuick`, `QtCharts`, effects)
2. **start real BobUI widget adoption**
3. **keep QML as a composition layer until BobUI-native surfaces are truly ready**

That preserves momentum and avoids a giant rewrite cliff.
