# GraphicalEffects Import Cleanup — Phase 3 Dependency Surface Reduction

## Summary
This session cleaned up the large amount of stale `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects` import noise in the bobfilez QML layer without attempting a risky visual rewrite.

## Why this was the right move
The prior migration audit recommended attacking `QtGraphicalEffects` next, but that category needed to be split into:
1. **dead imports that can be removed immediately**, and
2. **real effect usage that still needs a replacement strategy**

This session handled the low-risk half first.

## Audit result
A targeted scan showed that after earlier cleanup work, GraphicalEffects imports were present in many files, but only a small subset still actually instantiated effect items such as:
- `DropShadow`
- `FastBlur`

### Files that still genuinely use effect items
These were intentionally left unchanged for now:
- `gui/omni/assets/AcrylicBackground.qml`
- `gui/omni/assets/Dashboard.qml`
- `gui/omni/assets/ExplorerWindow.qml`
- `gui/omni/assets/OmniPeekOverlay.qml`
- `gui/omni/assets/Taskbar.qml`
- `gui/panels/OmniVersePanel.qml`

### Files that had dead imports only
These were cleaned:
- `gui/omni/assets/main.qml`
- `gui/omni/assets/StartMenu.qml`
- `gui/panels/AssetManagerPanel.qml`
- `gui/panels/CloudPanel.qml`
- `gui/panels/DigitalRotPanel.qml`
- `gui/panels/ForensicPanel.qml`
- `gui/panels/GamificationPanel.qml`
- `gui/panels/HierarchyPanel.qml`
- `gui/panels/NetworkPanel.qml`
- `gui/panels/NotaryPanel.qml`
- `gui/panels/OmniAudioPanel.qml`
- `gui/panels/OmniClerkPanel.qml`
- `gui/panels/OmniClusterPanel.qml`
- `gui/panels/OmniCryptPanel.qml`
- `gui/panels/OmniFlowPanel.qml`
- `gui/panels/OmniGitPanel.qml`
- `gui/panels/OmniGraphPanel.qml`
- `gui/panels/OmniMountPanel.qml`
- `gui/panels/OmniSecPanel.qml`
- `gui/panels/OmniVisionPanel.qml`
- `gui/panels/OraclePanel.qml`
- `gui/panels/RecoveryPanel.qml`
- `gui/panels/SwarmPanel.qml`
- `gui/panels/TimeMachinePanel.qml`
- `gui/panels/TopologyPanel.qml`
- `gui/panels/VaultPanel.qml`
- `gui/panels/VisualDedupPanel.qml`

## What changed
Removed only unused imports:
- `import QtGraphicalEffects 1.15`
- `import Qt5Compat.GraphicalEffects 1.15`

No actual effect expressions or visual behavior were touched in files that still use them.

## Practical outcome
### Before
GraphicalEffects imports were spread widely across the QML layer, making the dependency look larger and more entangled than it really was.

### After
The remaining imports now more accurately represent the true effect-dependent surfaces.

This makes the next phase much cleaner:
- replace or simplify the **actual six effect-using files**
- without wasting effort on files that were never truly dependent

## Validation performed
- Confirmed the remaining GraphicalEffects imports now exist only in files that still instantiate effect items.
- Confirmed dead imports were removed from the rest of the QML tree.

## Architectural significance
This is not a full GraphicalEffects removal yet.
It is a **dependency-surface cleanup** that improves signal-to-noise and reduces the apparent migration burden.

That is valuable because it:
- simplifies future auditing
- reduces accidental module coupling
- clarifies which visual surfaces need real replacement work

## Recommended next steps
1. Replace or simplify the remaining six real effect users.
2. Consider BobUI/OmniGfx replacements where actually available and properly wired.
3. Prefer pragmatic visual simplification over framework gymnastics if the effect is purely cosmetic.
