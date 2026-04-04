# QtQuick.Controls Dashboard Reduction — Shell Asset Completion Milestone

## Summary
This session removed `QtQuick.Controls` usage from `gui/omni/assets/Dashboard.qml`.

Unlike the previous shell-host reductions, this target was slightly richer because it used several convenience controls (`Label`, `Button`, `GroupBox`, and `ProgressBar`). Even so, it remained a good incremental candidate because the file is still self-contained, shell-adjacent, and structurally easy to reason about.

## Why `Dashboard.qml` was a safe next target
`Dashboard.qml` was the right escalation point because:
- it lives in the shell asset layer rather than the routed panel layer
- its structure is local and static
- its controls usage was mostly visual convenience rather than deep interactive state
- the richer controls could still be replaced with lightweight local primitives

That made it a better next step than jumping immediately into panel-form files with heavy `ComboBox`, `CheckBox`, `Menu`, or editor-like usage.

## What changed
Updated:
- `gui/omni/assets/Dashboard.qml`

Removed:
- `import QtQuick.Controls 2.15`

### Replacements performed
- `Label` → `Text`
- settings `Button` → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`
- `GroupBox` sections → explicit titled `Text` + `Rectangle` section containers
- `ProgressBar` → local inline `SlimProgressBar` component built from plain `Rectangle` primitives
- quick action `Button`s → local inline `QuickActionChip` component built from `Rectangle`, `Text`, `Row`, `MouseArea`, and `HoverHandler`

## What remained intact
- dashboard geometry and visual hierarchy
- storage summary section
- nexus task monitor section
- system performance cards
- quick action surface
- shell embedding behavior

## Measurement
### Before this session
Global `QtQuick.Controls` import count was:
- **40 QML files**

### After this session
Global `QtQuick.Controls` import count is now:
- **39 QML files**

### Shell-assets milestone
After this conversion, the entire shell-assets directory:
- `gui/omni/assets/`

now contains:
- **0 QML files importing `QtQuick.Controls`**

This is a meaningful milestone because the shell host and its adjacent Omni shell surfaces are now fully off the stock controls module.

## Analysis
This conversion shows the migration strategy can safely handle a richer shell surface as long as:
- the file is self-contained
- the interactive behaviors are shallow
- convenience controls can be replaced locally without introducing cross-file abstractions prematurely

It also sharpens the project boundary for the next phase:
- the remaining `QtQuick.Controls` imports now live in routed panel files rather than the shell assets themselves
- future work will likely need more careful strategies because those panels contain denser form and widget patterns

## Recommended next steps
1. Treat the shell-assets layer as a completed controls-removal milestone.
2. Begin selecting the simplest routed panel candidates rather than the heaviest feature panels.
3. Keep the global import count as the main migration metric.
4. Preserve the incremental approach: small, measurable, low-risk reductions instead of broad rewrites.
