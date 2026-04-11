# BTK Assimilation Requirements

## Problem Statement
BTK currently provides a substantial CopperSpice-derived C++ framework surface, but it does not yet satisfy the user's larger strategic goals:

1. BTK branding must replace CopperSpice branding.
2. External frameworks must be brought into the repo as reference and assimilation inputs.
3. BobUI must be analyzed as a direct comparative reference because it is another Qt-port/rethink effort.
4. BTK must be redesigned for large-scale foundational development, including true multi-user local and networked collaboration.
5. Missing capability areas must be identified across Qt 5/6, JUCE, Ultimate++, JavaFX, and Dear ImGui.

## Session-0 Requirements
### Completed in this session
- Add `JUCE`, `ultimatepp`, and `bobui` as git submodules.
- Establish BTK-branded package/config entry points in CMake.
- Create initial AI-devkit phase docs capturing requirements, design, planning, implementation notes, and testing.
- Produce an initial comparative analysis of BTK vs BobUI/JUCE/U++ and adjacent toolkits.

### Requirements still open
- Full repository-wide identifier migration from `CopperSpice`/`Cs*` to `BTK`/`Btk*`.
- Real QML / Quick / QuickControls2 / WebEngineQuick implementation strategy.
- Systematic parity verification against modern Qt 6 APIs.
- Audio/plugin/media assimilation comparable to JUCE.
- Productivity, IDE, data/grid/reporting, and RAD features comparable to Ultimate++.
- Multi-user ownership/focus/input semantics across the event and windowing stack.

## Functional Requirements
- Support multiple concurrent focus owners instead of a single global focus assumption.
- Track input ownership by device/user/session.
- Support collaborative modal semantics where a modal surface is scoped to an owner, not necessarily the whole process.
- Expose a declarative/UI runtime roadmap that can absorb QML/Quick-like concepts.
- Preserve compatibility while introducing BTK naming.

## Non-Functional Requirements
- Avoid a reckless global rename that destroys buildability.
- Prefer staged compatibility aliases before deep ABI/API breaks.
- Keep upstream reference repos intact in submodules.
- Document every major decision before broad invasive changes.

## Immediate Constraints
- The requested end-state is far beyond a single session.
- Current BTK contains mature subsystems but not a verified modern Qt Quick/WebEngine stack.
- BobUI includes many interesting ideas but also contains partial, aspirational, or runtime-incomplete implementations; it should not be copied blindly.
