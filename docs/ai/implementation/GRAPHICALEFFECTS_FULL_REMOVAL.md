# GraphicalEffects Full Removal — Phase 4 Dependency Reduction

## Summary
This session completed the GraphicalEffects cleanup by replacing or simplifying the last real effect users in the bobfilez QML layer.

## Goal
The previous cleanup phase removed dead `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects` imports.

That left six files that still genuinely used effect primitives such as:
- `DropShadow`
- `FastBlur`

This session removed those final effect dependencies so the `gui/` tree no longer depends on GraphicalEffects at all.

## Files updated
### Remaining real effect users simplified
- `gui/omni/assets/AcrylicBackground.qml`
- `gui/omni/assets/Dashboard.qml`
- `gui/omni/assets/ExplorerWindow.qml`
- `gui/omni/assets/OmniPeekOverlay.qml`
- `gui/omni/assets/Taskbar.qml`
- `gui/panels/OmniVersePanel.qml`

## What changed

### 1. Acrylic background
**File:** `gui/omni/assets/AcrylicBackground.qml`

Removed:
- `Qt5Compat.GraphicalEffects`
- `FastBlur`

Replacement:
- a lightweight translucent fallback rectangle layered over the existing tint/noise treatment

Rationale:
- preserves the shell's translucent feel
- avoids effect-module dependency
- does not pretend to offer real live desktop blur

### 2. Dashboard shell card
**File:** `gui/omni/assets/Dashboard.qml`

Removed:
- `QtGraphicalEffects`
- `DropShadow` layer effect

Replacement:
- simple border/backplate fallback

Rationale:
- keeps separation from the background
- avoids effect dependency for a largely cosmetic shadow

### 3. Explorer window chrome
**File:** `gui/omni/assets/ExplorerWindow.qml`

Removed:
- `QtGraphicalEffects`
- `DropShadow` layer effect

Replacement:
- stronger border-only separation

Rationale:
- the window remains visually distinct
- lower dependency complexity
- preserves layout and behavior without requiring a visual-effect item

### 4. OmniPeek overlay shell
**File:** `gui/omni/assets/OmniPeekOverlay.qml`

Removed:
- `QtGraphicalEffects`
- `DropShadow` layer effect on the preview card

Replacement:
- retained dimmer/backdrop + border/radius shell only

Rationale:
- preview remains functionally correct
- visual downgrade is acceptable for this cleanup phase

### 5. Taskbar chrome
**File:** `gui/omni/assets/Taskbar.qml`

Removed:
- `QtGraphicalEffects`
- `DropShadow` layer effect

Replacement:
- retained translucent background + top border

Rationale:
- keeps taskbar legible and separated
- effect was stylistic, not architectural

### 6. OmniVerse launch overlay
**File:** `gui/panels/OmniVersePanel.qml`

Removed:
- `QtGraphicalEffects`
- `DropShadow` effect on the launch overlay card

Replacement:
- border/radius overlay shell only

Rationale:
- preserves UX and launch flow
- removes the final panel-level dependency on DropShadow

## Validation performed
### Imports
Confirmed there are now **no**:
- `import QtGraphicalEffects ...`
- `import Qt5Compat.GraphicalEffects ...`

under `gui/`.

### Effect primitives
Confirmed there are now **no** remaining uses under `gui/` of:
- `DropShadow`
- `FastBlur`
- `GaussianBlur`
- `OpacityMask`
- `ColorOverlay`
- `Glow`
- `InnerShadow`
- `RectangularGlow`

### Build safety
Re-ran the normal headless validation path after the cleanup.

## Tradeoff
This is a deliberate simplification step.

### Lost / reduced
- some shadow depth
- fake acrylic blur polish
- some premium glassmorphism feel in the affected surfaces

### Preserved
- layout
- interaction
- shell routing
- panel functionality
- dependency reduction momentum

## Practical significance
This means the `gui/` tree has now completed a meaningful cleanup sequence:
1. `WebEngineQuick` removed from the markdown preview path
2. `QtCharts` removed from enhanced file ops
3. dead GraphicalEffects imports removed
4. remaining GraphicalEffects usages removed entirely

That is a strong reduction of GUI dependency noise without forcing a catastrophic rewrite.

## Recommended next steps
1. Re-run the BobUI consumer probe to confirm the blocker set is now even narrower and still centered on `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2`.
2. Start a more deliberate audit of stock `QtQuick.Controls` surfaces versus BobUI-native controls.
3. Continue preserving working UX while shrinking dependency surface step by step.
