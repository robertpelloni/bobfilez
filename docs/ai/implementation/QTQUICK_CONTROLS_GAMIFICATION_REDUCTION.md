# QtQuick.Controls Gamification Reduction — Fifth Routed Panel Milestone

## Summary
This session removed `QtQuick.Controls` usage from `gui/panels/GamificationPanel.qml`.

This is the fifth successful routed-panel conversion after `DigitalRotPanel.qml`, `SwarmPanel.qml`, `RecoveryPanel.qml`, and `ForensicPanel.qml`. It is also notable because it expands the proven migration pattern slightly: beyond simple dashboard sections, it also handles a lightweight hover-detail interaction without relying on the stock `ToolTip` control.

## Why `GamificationPanel.qml` was the right next target
`GamificationPanel.qml` remained a good candidate even though it was slightly richer than the previous panels, because:
- it is still self-contained
- it is still primarily dashboard-style
- it does not use popups, menus, text-entry widgets, sliders, or editor-like controls
- its only richer convenience elements were a `ProgressBar`, one `GroupBox`, and a `ToolTip` on achievement badges

That made it a controlled next escalation: richer than the last four, but still local and understandable.

## What changed
Updated:
- `gui/panels/GamificationPanel.qml`

Removed:
- `import QtQuick.Controls 2.15`

### Replacements performed
- `Label` → `Text`
- action `Button` → local `PanelButton` helper component built from `Rectangle`, `Text`, `HoverHandler`, and `MouseArea`
- `ProgressBar` → local `SlimProgressBar` helper component built from plain rectangles
- `GroupBox` achievements section → explicit titled `Text` + `Rectangle` section container
- repeated summary cards → local `StatCard` helper component
- `ToolTip` on achievements → local hover-popup `Rectangle` anchored above the achievement badge

## What remained intact
- level/rank header
- XP summary and bar visualization
- stats cards
- achievements grid
- hover detail for achievements
- daily quest action row

## Measurement
### Before this session
Global `QtQuick.Controls` import count was:
- **35 QML files**

### After this session
Global `QtQuick.Controls` import count is now:
- **34 QML files**

## Practical significance
This conversion matters for two reasons:
1. It continues the dashboard-style routed-panel migration lane.
2. It proves that a small amount of hover-detail behavior can be replaced locally without pulling in the full Controls module.

That suggests the next candidate set can include slightly richer dashboard-like panels, as long as the richer behavior remains local and shallow.

## Recommended next steps
1. Continue prioritizing self-contained routed panels with low widget diversity.
2. Allow small local hover-detail replacements when they stay simple and readable.
3. Still defer panels with `Popup`, `Menu`, `ComboBox`, `TextField`, `TextArea`, `Slider`, and other denser widget families.
4. Preserve the import-count delta in every pass so migration progress remains quantitative.
