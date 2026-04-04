# QtCharts Removal — Phase 2 Native UI Simplification

## Summary
This session removed the remaining `QtCharts` dependency from the bobfilez native UI layer.

## Why this was low-risk
The migration audit recommended removing smaller, high-value GUI module dependencies before attempting broad UI rewrites.

`QtCharts` turned out to be a particularly clean target because:
- it appeared in only one place:
  - `gui/panels/EnhancedFileOpsPanel.qml`
- the panel was **already not using QtCharts widgets at runtime**
- the speed graph had already been reimplemented as a lightweight `Canvas` drawing routine

In practice, the `QtCharts` import had become dead weight.

## What changed
Updated:
- `gui/panels/EnhancedFileOpsPanel.qml`

Removed:
- `import QtCharts 2.5`

## Existing replacement already in place
The panel's "Real-time speed graph" is already drawn by:
- `Canvas { ... onPaint: ... }`
- a periodic `Timer` to trigger repaint

So this change did **not** require inventing a new visualization path.
It simply removed an unnecessary module dependency declaration.

## Practical impact
### Before
The panel implied a dependency on:
- `QtCharts`

### After
The panel depends only on:
- `QtQuick`
- `QtQuick.Controls`
- `QtQuick.Layouts`

This improves the native UI dependency story and further narrows the gap between current bobfilez UI requirements and what a future BobUI-first stack might support.

## Validation performed
- Confirmed the speed graph implementation is canvas-driven and not based on `ChartView` / `LineSeries` / other QtCharts types.
- Confirmed there are no remaining `QtCharts` references under `gui/` after this change.

## Architectural significance
This is exactly the kind of migration step recommended by the prior BobUI migration audit:
- remove unnecessary optional GUI modules first
- preserve current functionality
- avoid broad rewrites until the remaining architectural blockers are better isolated

## Recommended next steps
1. Continue scanning for dead or easily replaceable imports in QML.
2. Target `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects` next, since those remain widespread but are more stylistic than architectural.
3. Keep preserving behavior while steadily shrinking the dependency surface.
