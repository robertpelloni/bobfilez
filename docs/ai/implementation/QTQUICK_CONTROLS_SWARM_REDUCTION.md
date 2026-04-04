# QtQuick.Controls Swarm Reduction — Second Routed Panel Milestone

## Summary
This session removed `QtQuick.Controls` usage from `gui/panels/SwarmPanel.qml`.

This is the second successful conversion inside the routed panel layer, following `DigitalRotPanel.qml`, and it further validates the current migration strategy: dashboard-like panels with shallow interaction patterns are the right next targets after completing the shell-assets layer.

## Why `SwarmPanel.qml` was the right next target
Among the remaining panel files, `SwarmPanel.qml` stood out as especially suitable because:
- it is self-contained
- it behaves like a dashboard rather than a form editor
- it does not use popups, menus, text entry, or other richer control families
- its stock-controls usage was limited to `Label`, `Button`, one `GroupBox`, and one `ProgressBar`

That made it even cleaner than panels involving `Slider`, `ToolTip`, `Popup`, `Menu`, `ComboBox`, or text-entry widgets.

## What changed
Updated:
- `gui/panels/SwarmPanel.qml`

Removed:
- `import QtQuick.Controls 2.15`

### Replacements performed
- `Label` → `Text`
- action `Button`s → local `PanelButton` helper component built from `Rectangle`, `Text`, `HoverHandler`, and `MouseArea`
- `GroupBox` detected-nodes section → explicit titled `Text` + `Rectangle` section container
- `ProgressBar` → local `SlimProgressBar` helper component built from plain rectangles

### Small structural refinement
A local `statusColor(status)` helper function was added to keep node-status text styling readable and avoid repeating inline conditional logic throughout the delegate.

## What remained intact
- peer refresh action wiring (`isDiscovering` toggle)
- world-map visualization shell
- detected-nodes list layout
- sync/settings row actions
- swarm throughput summary row

## Measurement
### Before this session
Global `QtQuick.Controls` import count was:
- **38 QML files**

### After this session
Global `QtQuick.Controls` import count is now:
- **37 QML files**

## Practical significance
With both `DigitalRotPanel.qml` and `SwarmPanel.qml` converted successfully, the project now has a stronger proven pattern for routed-panel work:
- prioritize self-contained dashboard-style panels
- use small local helper components rather than broad shared abstractions too early
- defer widget-dense panels until the simpler panel inventory is exhausted

## Recommended next steps
1. Continue targeting dashboard-like routed panels with minimal control diversity.
2. Keep postponing panels that depend on `Popup`, `Menu`, `ComboBox`, `TextField`, `ToolTip`, `Slider`, and similar richer controls.
3. Preserve the import-count delta in every pass so the reduction campaign stays measurable.
