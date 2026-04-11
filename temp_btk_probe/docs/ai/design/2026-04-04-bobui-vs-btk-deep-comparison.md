# BobUI vs BTK Deep Comparison

## Scope
This document compares the newly added `external/bobui-reference` submodule against the current BTK repository with emphasis on architecture, features, gaps, and what should be adopted versus reinterpreted.

## Executive Summary
BobUI is not stronger than BTK in terms of mature, verified C++ framework depth today. BTK already has a broader and more battle-tested Qt/CopperSpice-style C++ substrate. However, BobUI is strategically valuable because it centers several ideas BTK currently lacks or only implies:

1. explicit multi-user ownership and collaboration semantics
2. framework-vs-shell boundary discipline
3. experimentation with WebView bridge contracts
4. explicit planning language around parity audits rather than only raw implementation breadth

The right move is **not** to copy BobUI wholesale. The right move is to extract its strongest strategic ideas and rebuild them on top of BTK's stronger systems foundation.

## 1. Repository Shape Comparison

### BTK
Observed major source areas:
- `src/core`
- `src/gui`
- `src/network`
- `src/xml`
- `src/sql`
- `src/svg`
- `src/opengl`
- `src/vulkan`
- `src/webkit`
- `src/xmlpatterns`
- `src/declarative`
- `src/plugins`
- `src/tools`
- `src/annex`

### BobUI reference
Observed internal Go packages:
- `internal/audio`
- `internal/data`
- `internal/kernel`
- `internal/net`
- `internal/state`
- `internal/ui`
- `internal/vm`

### Interpretation
- BTK is presently **broader and deeper** as a traditional application framework runtime.
- BobUI is **narrower but more explicitly future-facing** in collaboration, ownership, and framework identity.

## 2. Architectural Philosophy

### BTK today
- inherited from the CopperSpice/Qt lineage
- broad cross-platform framework surface
- classic retained-mode widget + application framework thinking
- some declarative infrastructure exists, but it is not yet the center of the current active roadmap

### BobUI
- aggressively positions itself as a framework/kernel, not the shell product
- puts multiplayer/collaboration and distributed interaction at the center of the story
- mixes runtime ideas, product vision, and partial implementations more freely
- often documents aspirational parity work before runtime-complete implementations exist

### Resulting recommendation
BTK should adopt BobUI's **strategic framing** while preserving BTK's stronger implementation discipline.

## 3. Feature-by-Feature Comparison

### 3.1 Core runtime and application framework
**BTK advantage**
- mature C++ class library base
- richer native module set
- more complete traditional framework surface

**BobUI advantage**
- clearer framing for permissions, ownership, synchronization, self-healing, and session-aware state

**Action**
- keep BTK runtime core
- add BobUI-inspired ownership/session concepts in BTK core/gui

### 3.2 Declarative / Quick direction
**BTK current state**
- `src/declarative` exists
- not integrated in the current root component build
- modern Quick/QuickControls2/WebEngineQuick parity is not yet verified

**BobUI current state**
- lightweight Go-native control baselines: popup, dialog, drawer, tooltip, menu, scroll view, scrollbar, webview
- these are useful prototypes, but not complete parity implementations

**Action**
- treat BobUI controls as conceptual reference only
- define a real BTK declarative roadmap around build-verified runtime semantics

### 3.3 Web runtime
**BTK current state**
- legacy WebKit-era stack exists
- no verified modern WebEngineQuick equivalent yet

**BobUI current state**
- lightweight `WebView` navigation/history model
- bridge contract: `EvalJS`, `PostMessage`, `Request`, handler registration
- still not a full embedded browser runtime

**Action**
- adopt the bridge contract ideas
- build a modern BTK web embedding strategy beyond legacy WebKit
- preserve explicit permission/interception points

### 3.4 Audio and media
**BTK current state**
- multimedia module exists
- not yet JUCE-grade in audio/plugin story

**BobUI current state**
- interesting audio graph / voice / MIDI mesh concepts
- appears more experimental than production-complete

**Action**
- BobUI contributes distributed-audio/collab ideas
- JUCE remains the primary reference for actual audio/DSP/plugin architecture

### 3.5 Collaboration and multiplayer
**BTK current state**
- not yet built around multiple simultaneous focus owners
- classic single-focus assumptions likely still dominate

**BobUI current state**
- this is the strongest distinguishing area
- explicit concern for ownership, permissions, sync, user/session distinctions, and collaborative action histories

**Action**
- this is the single most important BobUI concept BTK should assimilate first
- rebuild it rigorously in C++ inside core/gui/event routing

### 3.6 Tooling and overlays
**BTK current state**
- classic tools and framework tooling exist
- no strong immediate-mode or multiplayer-aware debug overlay layer yet

**BobUI current state**
- documentation repeatedly points toward developer overlays, diagnostics, node surfaces, and inspectable runtime concepts

**Action**
- combine BobUI's inspection ambitions with Dear ImGui-style tooling ergonomics on top of BTK

## 4. BobUI Strengths Worth Keeping
- multi-user ownership as a first-class architecture primitive
- framework/kernel vs shell/product separation
- permission and synchronization vocabulary
- WebView bridge abstraction
- willingness to think about remote/local collaborative state as framework concerns

## 5. BobUI Weaknesses to Avoid
- equating scaffolding with parity
- claiming runtime breadth without verification
- allowing documentation and code reality to drift apart
- pursuing too many surfaces simultaneously before stabilizing the substrate

## 6. Adoption Matrix
| BobUI concept | Adopt? | How |
|---|---|---|
| Multi-user ownership model | Yes | Rebuild in BTK core/gui event routing |
| Owner-scoped modality | Yes | Introduce focus/ownership tokens and modal policy |
| WebView bridge contract | Yes | Use as design input for BTK web runtime |
| Framework vs shell separation | Yes | Preserve BTK as framework/kernel only |
| Go control baselines | Partially | Use as requirements sketches, not direct ports |
| Broad parity claims | No | Replace with testable BTK feature checklists |
| Placeholder-heavy implementations | No | Require build/runtime verification |

## 7. Concrete BTK Improvements Inspired by BobUI
1. `InputOwner` and `FocusToken` core types
2. owner-scoped modal policies
3. shared-state and collaboration permission interfaces
4. browser bridge contracts with request/reply channels
5. multiplayer-aware undo and action provenance
6. remote/local cursor identity in GUI event dispatch

## 8. Bottom Line
BobUI should be treated as a **vision-rich experimental reference**.

BTK should become:
- more rigorous than BobUI
- more implementation-complete than BobUI
- more modular than BobUI
- more verifiable than BobUI
- and eventually broader than BobUI by combining BobUI's collaboration vision with BTK's stronger native framework substrate and the proven strengths of JUCE, Qt, Ultimate++, JavaFX, and Dear ImGui.
