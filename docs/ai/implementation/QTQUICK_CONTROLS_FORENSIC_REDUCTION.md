# QtQuick.Controls Forensic Reduction — Fourth Routed Panel Milestone

## Summary
This session removed `QtQuick.Controls` usage from `gui/panels/ForensicPanel.qml`.

This is the fourth successful routed-panel conversion after `DigitalRotPanel.qml`, `SwarmPanel.qml`, and `RecoveryPanel.qml`, and it further confirms that the current migration lane is real: dashboard-style panels with shallow interaction models can be converted safely and incrementally.

## Why `ForensicPanel.qml` was the right next target
`ForensicPanel.qml` was selected because it remains on the simpler side of the remaining panel set:
- self-contained
- dashboard-like
- no popups
- no menus
- no text-entry widgets
- no sliders
- no tooltips

Its stock-controls usage was limited to:
- `Label`
- `Button`
- one `GroupBox`
- one `ScrollBar` attachment on the list

That made it a stronger candidate than panels already entangled with richer control families.

## What changed
Updated:
- `gui/panels/ForensicPanel.qml`

Removed:
- `import QtQuick.Controls 2.15`

### Replacements performed
- `Label` → `Text`
- action `Button`s → local `PanelButton` helper component built from `Rectangle`, `Text`, `HoverHandler`, and `MouseArea`
- `GroupBox` chain-of-custody section → explicit titled `Text` + `Rectangle` section container
- repeated ledger summary cards → local `StatCard` helper component
- `ScrollBar.vertical: ScrollBar {}` attachment removed to keep the list entirely free of stock controls

### Small structural refinements
Two local helper functions were added:
- `integrityFillColor()`
- `integrityTextColor()`

These keep the verification badge styling explicit and readable without repeating inline conditions across the header.

## What remained intact
- integrity-check toggle behavior (`isVerifying`)
- verification-state badge
- ledger summary cards
- chain-of-custody list layout
- footer action row

## Measurement
### Before this session
Global `QtQuick.Controls` import count was:
- **36 QML files**

### After this session
Global `QtQuick.Controls` import count is now:
- **35 QML files**

## Practical significance
With four routed-panel conversions now complete, the migration strategy is strongly reinforced:
- prioritize self-contained dashboard-style panels
- keep using small local helpers for repeated primitives
- postpone panels with richer widget diversity until the simpler panel inventory is exhausted

This keeps the reduction campaign low-risk while still producing steady, measurable progress.

## Recommended next steps
1. Continue with other dashboard-like routed panels that mostly use labels, simple buttons, and static sections.
2. Keep deferring panels that rely on `Popup`, `Menu`, `ComboBox`, `TextField`, `ToolTip`, `Slider`, and editor-style widgets.
3. Preserve the import-count delta in every pass so the migration remains quantitative.
