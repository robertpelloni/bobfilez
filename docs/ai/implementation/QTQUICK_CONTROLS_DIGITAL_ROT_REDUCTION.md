# QtQuick.Controls Digital Rot Reduction — First Routed Panel Milestone

## Summary
This session removed `QtQuick.Controls` usage from `gui/panels/DigitalRotPanel.qml`.

This is significant not just because one more file was converted, but because it marks the first successful controls-removal pass inside the routed panel layer after the shell-assets layer was completed.

## Why `DigitalRotPanel.qml` was the right first routed-panel target
After finishing `gui/omni/assets/`, the next question was not "which panel is most important," but rather "which panel is safest to convert first?"

`DigitalRotPanel.qml` was the best candidate because:
- it is visually self-contained
- its interactive behavior is shallow
- it does not rely on popups, menus, text-entry widgets, or editor-like controls
- its stock-controls usage was limited to `Label`, `Button`, and one `GroupBox`
- the panel already behaves more like a static dashboard than a dense form surface

This made it the ideal bridge between shell-surface reductions and more complex routed-panel work.

## What changed
Updated:
- `gui/panels/DigitalRotPanel.qml`

Removed:
- `import QtQuick.Controls 2.15`

### Replacements performed
- `Label` → `Text`
- action `Button`s → local `PanelButton` helper component built from `Rectangle`, `Text`, `HoverHandler`, and `MouseArea`
- `GroupBox` suggestions section → explicit titled `Text` + `Rectangle` section container
- stats cards → local `StatCard` helper component for repeated card presentation

## Simplification pass
A small refinement was applied immediately after the first rewrite:
- `PanelButton` now exposes a `clicked` signal
- callers connect with `onClicked` directly
- this avoids stacking extra `MouseArea` handlers in button instances

This keeps the local helper cleaner and more reusable while preserving behavior.

## What remained intact
- scanning toggle behavior (`isScanning`)
- reclaimable-space display
- risk-file count display
- system-health card
- suggestions list layout
- footer action surface

## Measurement
### Before this session
Global `QtQuick.Controls` import count was:
- **39 QML files**

### After this session
Global `QtQuick.Controls` import count is now:
- **38 QML files**

## Practical significance
This conversion establishes a new migration phase:
- the shell-assets layer is already complete
- the routed-panel layer is now proven to be reducible as well
- future work should continue by selecting the simplest panel candidates first rather than jumping into menu-heavy or form-heavy panels

## Recommended next steps
1. Continue with the next simplest routed panels that mostly use labels, buttons, and static section shells.
2. Defer panels with `Popup`, `Menu`, `ComboBox`, `TextField`, `SpinBox`, or dense editor behaviors until later.
3. Keep documenting each pass with the import-count delta so progress remains quantitative and comparable.
