# BTK vs BobUI Live Repository Comparison

## Date
2026-04-04

## Scope
This document compares the current live `bobui` repository in `C:/Users/hyper/workspace/bobui` against the current BTK repository in `C:/Users/hyper/workspace/btk`.

This is intentionally different from the earlier `external/bobui-reference` comparison. The goal here is to compare the **actual evolving BobUI repo**, including:
- its Qt-OSS-rooted native tree
- its newer Go experimental kernel/widget work
- its OmniUI C++ / QML / multi-cursor layer
- its current documentation claims versus observable implementation reality

## Why this comparison matters
The user correctly pointed out that BobUI is "almost identical except starting from Qt OSS instead of CopperSpice." That is directionally true at the repository-shape level, but incomplete at the implementation-strategy level.

The live comparison shows a more interesting reality:
- **BTK** is becoming the stronger path for rigorous, compatibility-first, owner-aware native framework evolution.
- **BobUI** is still extremely valuable, but primarily as an idea incubator, a QML/Quick-heavy experimentation lane, and a place where collaboration/multi-user vocabulary appears earlier and more explicitly.
- BobUI also contains more visible **claim-vs-implementation tension**, especially in the OmniUI layer.

## Sources inspected
### BTK
- `CMakeLists.txt`
- `docs/ai/design/2026-04-04-bobui-vs-btk-deep-comparison.md`
- `docs/ai/design/2026-04-04-multi-user-focus-foundation.md`
- `src/core/kernel/btkinputowner.h`
- `src/core/kernel/btkfocustoken.h`
- `src/gui/kernel/btkinputarbitrator.h`
- `src/gui/kernel/qapplication_cs.cpp`
- `src/gui/widgets/btkfocusoverlay.h`

### BobUI
- `CMakeLists.txt`
- `VISION.md`
- `ROADMAP.md`
- `docs/QML_QUICK_PARITY_AUDIT.md`
- `docs/MULTI_CURSOR_SPEC.md`
- `docs/OMNIUI_IMPLEMENTATION_DETAIL.md`
- `docs/ai/design/2026-04-04-bobui-vs-btk-comparison.md`
- `internal/kernel/input_manager.go`
- `internal/kernel/permission_manager.go`
- `internal/kernel/user_manager.go`
- `internal/data/history_sync.go`
- `internal/ui/widgets/webview.go`
- `OmniUI/omnicore/include/OmniInputManager.h`
- `OmniUI/omnicore/src/OmniInputManager.cpp`
- `OmniUI/omnicore/src/OmniApp.cpp`
- `OmniUI/omnicore/include/OmniDeveloperOverlay.h`
- `OmniUI/omnicore/src/OmniDeveloperOverlay.cpp`
- `OmniUI/tests/omnicore/tst_OmniInputManager.cpp`

## High-level finding
If we compare **framework scale**, **native runtime depth**, and **careful migration strategy**, BTK is stronger.

If we compare **future-facing narrative clarity**, **explicit collaboration vocabulary**, and **willingness to prototype hybrid QML/Go/multi-user ideas quickly**, BobUI is stronger.

If we compare **current implementation truthfulness**, BTK is presently more trustworthy.

## Scale and repository shape
Using a simple local source count snapshot:
- `BTK src/`: ~12,334 files / ~3,722,986 lines
- `BobUI src/`: ~5,730 files / ~3,292,395 lines
- `BobUI internal/` Go path: 99 files / ~4,523 lines

## Interpretation of the scale numbers
These numbers matter because they show that BobUI is not just a tiny Go experiment. It still carries a very large Qt-OSS-derived native tree. However, the genuinely differentiating BobUI work is concentrated in two smaller layers:
1. the Go `internal/` experimentation track
2. the OmniUI layer for multi-cursor / QML / tooling / JUCE bridge ideas

That means BobUI is simultaneously:
- a large inherited native framework tree
- a smaller experimental kernel
- a higher-risk concept lab with more claim pressure

BTK, by contrast, is currently more internally coherent in its active direction. The recent BTK work is narrowly focused on staged branding, owner-aware routing, popup/modal reform, and developer diagnostics.

## Starting-point difference: Qt OSS vs CopperSpice
### BobUI
BobUI's root build and source layout remain visibly close to Qt OSS:
- root project name is `BobUIBase`
- `src/` includes `corelib`, `gui`, `widgets`, `network`, `sql`, `xml`, `opengl`, `platformsupport`, `plugins`, `tools`
- build tooling includes `configure`, `qmake`, Qt-like module conventions, and documentation around QML build tooling

### BTK
BTK remains visibly rooted in CopperSpice:
- existing module structure resembles CopperSpice layout (`src/core`, `src/gui`, `src/xml`, `src/webkit`, `src/xmlpatterns`, etc.)
- BTK is deliberately adding **brand and packaging overlays** instead of performing a destructive one-shot rename
- BTK now exports staged package/config aliases such as `BTK::Core` while preserving compatibility

## Strategic significance
This difference is not just cosmetic:
- BobUI is closer to the Qt OSS ecosystem style, especially around Quick/QML framing.
- BTK is currently stronger in compatibility-first framework evolution and less likely to destabilize itself with identity churn.

## Multi-user architecture comparison

### BobUI strengths
BobUI is more explicit about the destination architecture.

Evidence:
- `docs/MULTI_CURSOR_SPEC.md` explicitly defines device-tagged events, per-device focus trees, and a cursor overlay model.
- `internal/kernel/input_manager.go` models device registration and cursor updates.
- `internal/kernel/user_manager.go` models device-to-user mapping.
- `internal/kernel/permission_manager.go` models simple interaction locking.
- `internal/data/history_sync.go` models a collaboration/sync vocabulary.
- `OmniUI/omnicore/include/OmniInputManager.h` exposes per-device focus and hover tracking.

### BobUI weakness
The conceptual framing is stronger than the end-to-end proven runtime.

Evidence:
- `OmniUI/omnicore/src/OmniApp.cpp` still hardcodes a fallback `currentDeviceId = "sys-mouse-0"` and includes a TODO saying raw hardware handles still need side-channel mapping into real device ids.
- This means the bold multi-device narrative is only partially reflected in the currently inspected notify-path reality.

### BTK strengths
BTK is behind BobUI in visionary language, but ahead in **typed framework-native integration discipline**.

Evidence:
- `BtkInputOwner`
- `BtkFocusToken`
- `BtkInputArbitrator`
- public `QApplication` integration points
- owner-aware popup and modality routing in `qapplication_cs.cpp`
- owner-aware diagnostics surfaced through `BTKFocusDiagnostics` and `BTKFocusOverlay`

BTK has already threaded owner-awareness through real legacy runtime hotspots:
- `setFocusWidget(...)`
- modal gating
- popup restoration
- wheel routing
- context-menu routing
- mouse enter/leave receiver tracking
- synthetic enter/leave generation
- drag/drop gating

### Net finding
- **BobUI is stronger at stating the problem.**
- **BTK is stronger at integrating the solution into a large native runtime without blowing it up.**

That is a very important distinction.

## Developer overlay comparison

### BobUI overlay story
BobUI's OmniUI layer has an attractive vision for a rich developer overlay:
- virtual cursors
- hover introspection
- toast center
- possible Dear ImGui integration
- deep QMetaObject property inspection

The implementation is visually ambitious.

### BobUI implementation concern
There is a notable code-level inconsistency:
- `OmniUI/omnicore/include/OmniDeveloperOverlay.h` declares `OmniDeveloperOverlay : public QQuickItem`
- `OmniUI/omnicore/src/OmniDeveloperOverlay.cpp` implements it as if it were a `QWidget`
- constructor signature and event overrides do not match the header's inheritance model

That is exactly the kind of mismatch that supports a larger conclusion: BobUI has genuinely strong ideas, but the repo still shows places where implementation rigor lags behind ambition.

### BTK overlay story
BTK's overlay is less glamorous but currently more coherent.

Evidence:
- `BtkFocusOverlay` is clearly defined as a QWidget diagnostic surface.
- It exposes explicit panel flags and presets.
- It is backed by a structured snapshot model rather than only ad hoc painting.
- It is explicitly tied to actual popup/focus/modal ownership diagnostics.

### Net finding
- **BobUI overlay ideas are more cinematic and product-forward.**
- **BTK overlay implementation is currently more framework-debugger-oriented and more trustworthy.**

The best future BTK path is to keep BTK's diagnostic rigor while gradually absorbing BobUI's more ambitious developer-experience style.

## QML / Quick / declarative direction

### BobUI strengths
BobUI is more aggressive and more opinionated about QML / Quick / declarative surfaces.

Evidence:
- `docs/OMNIUI_IMPLEMENTATION_DETAIL.md` argues for QML supremacy and heavy `QQuickItem` / `QQuickPaintedItem` usage.
- `docs/QML_QUICK_PARITY_AUDIT.md` explicitly frames parity work around QuickControls2 and WebEngineQuick-like semantics.
- OmniUI widget files show repeated `QQuickPaintedItem` usage for custom controls.

### BTK current state
BTK has `src/declarative`, but current active work is centered on native retained-mode runtime rehabilitation and owner-aware event routing.

### Net finding
- BobUI is presently the more declarative-first / QML-first idea track.
- BTK is presently the stronger native framework stabilization track.
- If the long-term goal is "better than Qt7," BTK still needs a much stronger declarative modernization plan, but it should not borrow BobUI's aspirations without also importing verification discipline.

## Web embedding comparison

### BobUI strengths
BobUI's Go `WebView` is one of its cleanest pieces of design work.

It provides a compact modern contract including:
- navigation/history
- title/load hooks
- `EvalJS(...)`
- `PostMessage(...)`
- `Request(...)`
- handler registration
- bridge contract serialization

This is not a browser engine, but it is a **good API sketch**.

### BTK strengths
BTK still has far more historical web runtime depth via `src/webkit`.

### Net finding
- BobUI has the better **modern host/page contract abstraction**.
- BTK has the better **legacy engine substrate**.
- The right synthesis is for BTK to adopt BobUI-style bridge API ideas while choosing a more modern engine direction than merely preserving legacy WebKit.

## Build strategy and identity management

### BobUI approach
BobUI largely re-identifies the Qt-style tree as BobUI and continues exploratory additions around it.

### BTK approach
BTK's staged alias strategy is substantially safer:
- `BTKConfig.cmake`
- `BTKMacros.cmake`
- target aliases
- public wrapper headers
- compatibility preservation instead of mass churn

### Net finding
BTK is clearly stronger here. This is one of the most convincing signs that BTK is the better long-term framework mainline.

## Truthfulness and documentation discipline
This is where the live comparison becomes especially revealing.

### BobUI
BobUI has several excellent docs, but it also shows stronger claim pressure:
- docs often describe ambitious system behavior in a confident, productized tone
- some of the code genuinely exists
- some of the runtime plumbing is still simplified, placeholder-like, or incomplete
- some files appear internally inconsistent

### BTK
BTK's recent AI docs are much more explicit about uncertainty and validation limits:
- they distinguish implemented vs scaffolded vs not yet compiler-validated work
- they document testing gaps
- they identify exact hotspot functions in the runtime
- they avoid claiming completed parity where proof is missing

### Net finding
At this moment, **BTK is the more intellectually honest engineering track**.

That matters more than hype.

## Comparative scorecard
| Category | BobUI | BTK | Winner now | Why |
|---|---|---|---|---|
| Native framework breadth | Large inherited Qt-style tree | Large inherited CopperSpice-style tree | BTK slight edge | Broader stabilized module picture and current integration focus |
| Branding/migration safety | More direct identity replacement | Staged compatibility-first aliasing | BTK | Much lower risk migration strategy |
| Declarative/QML ambition | Strong | Moderate / not yet central | BobUI | More explicit Quick/QML-first direction |
| Multi-user vision clarity | Strong | Improving | BobUI | Clearer articulation of target model |
| Multi-user framework integration rigor | Partial / mixed | Stronger and actively threaded into hotspots | BTK | Real event-path integration underway |
| Developer overlay vision | Strong | Stronger diagnostic discipline | Split | BobUI more ambitious; BTK more coherent |
| Web bridge API design | Strong | Weak/moderate | BobUI | Cleaner modern abstraction |
| Web engine substrate | Weak/partial | Strong legacy substrate | BTK | Much larger actual engine base |
| Documentation honesty | Mixed | Strong | BTK | Better separation of verified vs aspirational |
| Fast experimentation | Strong | Moderate | BobUI | Smaller active custom layers move faster |

## Most important finding
The most important finding is this:

**BobUI is not the stronger framework implementation today. BobUI is the stronger source of provocative framework ideas. BTK is the better place to turn those ideas into a durable, framework-grade reality.**

That means the best strategic posture is not competition. It is controlled assimilation.

## What BTK should steal from BobUI immediately
1. **Sharper declarative modernization planning**
   - especially QuickControls2/WebEngineQuick-class roadmap framing
2. **Cleaner web bridge API contracts**
   - `EvalJS`, `PostMessage`, request/reply channels, handler registration
3. **Richer developer overlay ambition**
   - more interactive panels, hover/object introspection, cursor visualization, provenance views
4. **Explicit collaboration vocabulary**
   - permissions, sync, user/device mapping, remote/local identity language
5. **Framework-vs-shell boundary clarity**
   - keep BTK pure as framework/runtime, not shell/product

## What BTK should *not* copy from BobUI
1. **Doc-first parity inflation**
2. **Loose coupling between claims and proven runtime behavior**
3. **Architectural inconsistency between headers, implementations, and platform model**
4. **Monolithic QML-first declarations without validated substrate support**

## Best interpretation of both repos
### BobUI should be treated as
- the faster experimental lab
- the QML/Quick-heavy idea incubator
- the collaboration vocabulary forge
- the bridge-contract sketchpad

### BTK should be treated as
- the long-term framework mainline
- the compatibility-conscious native runtime
- the place where ownership semantics become real event-routing law
- the place where diagnostics, migration, and packaging discipline stay strong

## Bottom line
If the goal is to build the "real Qt7" or something better than Qt7, the live comparison suggests:

- **BTK has the better chance of becoming the durable framework core.**
- **BobUI remains invaluable as a source of directional ideas, especially around declarative UX, collaboration, and developer tooling.**
- **The optimal strategy is to let BobUI invent aggressively and let BTK absorb selectively, rigorously, and compatibility-first.**

That is how the combination becomes stronger than either lineage alone.
