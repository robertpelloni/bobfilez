# QtQuick.Controls OmniPeek Reduction — Overlay Surface Conversion

## Summary
This session converted `gui/omni/assets/OmniPeekOverlay.qml` away from `QtQuick.Controls`.

## Why OmniPeek was a good target
Although `OmniPeekOverlay.qml` used more controls than the smallest shell files, it was still a practical next step because:
- the UI is self-contained
- the interactions are localized to one overlay surface
- most stock control usage was still convertible to plain `QtQuick` primitives
- the text viewer could fall back from `ScrollView + TextArea` to a lighter `Flickable + TextEdit` path

## What changed
Updated:
- `gui/omni/assets/OmniPeekOverlay.qml`

Removed:
- `import QtQuick.Controls 2.15`

### Replacements performed
- `Label` → `Text`
- header action `Button`s → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`
- model-viewer control `Button`s → lightweight rectangle-button repeater
- media play icon `Label` → `Text`
- text viewer:
  - `ScrollView` → `Flickable`
  - `TextArea` → `TextEdit` (read-only)
- footer `Label`s → `Text`

## What remained intact
- overall overlay structure
- preview type switching logic
- close behavior
- content payload display
- metadata footer

## Measurement
### Before this session
Global `QtQuick.Controls` import count was:
- **42 QML files**

### After this session
Global `QtQuick.Controls` import count is now:
- **41 QML files**

## Practical significance
This proves the controls-reduction strategy can safely extend beyond simple shell chrome into a more featureful overlay, as long as the control usage is still locally understandable and replaceable.

## Tradeoffs
This conversion replaces stock convenience controls with simpler primitives, which means:
- less built-in behavior from Qt Quick Controls
- more manual ownership of hover/click shell behavior
- text viewing now uses a lighter primitive stack (`Flickable` + `TextEdit`) instead of Controls wrappers

These tradeoffs are acceptable because the purpose of the migration step is to shrink dependency surface while keeping the overlay behaviorally intact.

## Recommended next steps
1. Continue reducing shell-adjacent / overlay surfaces before heavy panel forms.
2. Keep measuring the import count after each pass.
3. Defer rich control-heavy panels until a replacement approach exists for the larger control families.
