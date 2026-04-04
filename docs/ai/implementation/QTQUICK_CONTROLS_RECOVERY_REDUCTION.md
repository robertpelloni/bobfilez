# QtQuick.Controls Recovery Reduction — Third Routed Panel Milestone

## Summary
This session removed `QtQuick.Controls` usage from `gui/panels/RecoveryPanel.qml`.

This is the third successful routed-panel conversion after `DigitalRotPanel.qml` and `SwarmPanel.qml`, and it further strengthens the current migration thesis: the panel layer is most approachable when work begins with self-contained, dashboard-like panels that avoid richer widget families.

## Why `RecoveryPanel.qml` was the right next target
`RecoveryPanel.qml` was chosen because it is structurally very close to `DigitalRotPanel.qml`:
- self-contained
- dashboard-oriented
- interaction-light
- no popups
- no menus
- no text-entry widgets
- no sliders
- no tooltips

Its stock-controls usage was limited to:
- `Label`
- `Button`
- one `GroupBox`

That made it one of the cleanest remaining routed-panel candidates.

## What changed
Updated:
- `gui/panels/RecoveryPanel.qml`

Removed:
- `import QtQuick.Controls 2.15`

### Replacements performed
- `Label` → `Text`
- action `Button`s → local `PanelButton` helper component built from `Rectangle`, `Text`, `HoverHandler`, and `MouseArea`
- `GroupBox` integrity-exceptions section → explicit titled `Text` + `Rectangle` section container
- repeated dashboard metrics → local `StatCard` helper component

### Small structural refinement
A local `corruptionColor()` helper function was added so the corruption-count styling remains explicit and readable without repeating inline conditional expressions in the stat card setup.

## What remained intact
- scrub toggle behavior (`isScrubbing`)
- health dashboard presentation
- integrity-exceptions list layout
- per-row heal action surface
- footer action row

## Measurement
### Before this session
Global `QtQuick.Controls` import count was:
- **37 QML files**

### After this session
Global `QtQuick.Controls` import count is now:
- **36 QML files**

## Practical significance
With three routed-panel conversions now complete, the migration pattern is no longer anecdotal. It is increasingly clear that the best next-wave panel targets are:
- dashboard-style
- self-contained
- low in control diversity
- free of popup/menu/editor/form-heavy widget clusters

This gives the next phase a stable selection rule rather than an ad-hoc file-by-file guess.

## Recommended next steps
1. Continue prioritizing routed panels that look and behave like dashboards or control summaries.
2. Defer panels that introduce `Popup`, `Menu`, `ComboBox`, `TextField`, `ToolTip`, `Slider`, and heavier editor/form patterns.
3. Keep using small local helper components where they improve clarity without prematurely creating shared cross-panel UI infrastructure.
4. Preserve the import-count delta as the primary migration metric.
