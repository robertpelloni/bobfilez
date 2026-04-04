# QtQuick.Controls Initial Reduction — Phase 5 Native UI Simplification

## Summary
This session began the next migration cluster by reducing direct `QtQuick.Controls` usage in two shell-critical surfaces:
- `gui/omni/assets/Taskbar.qml`
- `gui/omni/assets/StartMenu.qml`

## Why these were chosen
The previous dependency cleanup removed:
- `WebEngineQuick`
- `QtCharts`
- all GraphicalEffects usage

The next largest remaining cluster is stock `QtQuick.Controls`.

A full controls migration across the whole UI would be too broad for one step, so this session targeted the best low-risk starting points:
- surfaces close to the shell chrome
- mostly using convenience controls (`Label`, `Button`, `ToolTip`)
- minimal reliance on heavy controls such as `ComboBox`, `ScrollView`, `GroupBox`, `ProgressBar`, etc.

## What changed

### 1. `Taskbar.qml`
Updated:
- `gui/omni/assets/Taskbar.qml`

Changes:
- removed:
  - `import QtQuick.Controls 2.15`
- replaced stock control usage with plain `Text` or existing `Rectangle` / `MouseArea` patterns
- removed the leftover `ToolTip` usage for pinned apps

Effectively, the taskbar now relies on:
- `QtQuick`
- `QtQuick.Layouts`

rather than `QtQuick.Controls`.

## 2. `StartMenu.qml`
Updated:
- `gui/omni/assets/StartMenu.qml`

Changes:
- removed:
  - `import QtQuick.Controls 2.15`
- replaced `Label` usage with `Text`
- replaced lightweight text-only `Button` usage (`All apps >`, `More >`) with small `Rectangle` + `Text` + `MouseArea` shells
- preserved layout and visual structure

The start menu still uses `TextInput`, which comes from Qt Quick rather than Qt Quick Controls, so this file can now also avoid the Controls import.

## Practical outcome
### Before this session
A broad QML scan showed:
- **48 QML files** importing `QtQuick.Controls`

### After this session
The count dropped to:
- **46 QML files** importing `QtQuick.Controls`

That is not the end state, but it is a real reduction on two high-visibility shell surfaces.

## Why this matters
This session proves the controls migration can proceed incrementally by:
- preferring `Text` over `Label` when a richer control abstraction is unnecessary
- using existing `Rectangle` / `MouseArea` shell patterns instead of trivial `Button`s
- shrinking dependency surface without touching more complex control-heavy panels yet

## What was deliberately not attempted yet
This session did **not** try to remove controls usage from files dominated by richer control sets such as:
- `ComboBox`
- `GroupBox`
- `ProgressBar`
- `CheckBox`
- `ScrollView`
- `TextArea`
- `Menu` / `MenuItem`

Those require a more deliberate replacement strategy.

## Validation
- Confirmed `Taskbar.qml` no longer imports `QtQuick.Controls`.
- Confirmed `StartMenu.qml` no longer imports `QtQuick.Controls`.
- Recomputed the global QML import count for `QtQuick.Controls` after the edit.

## Recommended next steps
1. Continue targeting shell-adjacent files that only use `Label` / trivial `Button` wrappers.
2. Build a replacement plan for the heavier control categories (`ComboBox`, `CheckBox`, `ProgressBar`, etc.).
3. Use BobUI-native controls only where the registration and API surface are truly known to be stable; otherwise prefer plain Qt Quick primitives over stock Controls where practical.
