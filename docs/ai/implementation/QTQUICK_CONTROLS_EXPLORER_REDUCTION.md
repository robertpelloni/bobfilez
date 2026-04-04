# QtQuick.Controls Explorer Reduction — Shell Surface Conversion

## Summary
This session converted `gui/omni/assets/ExplorerWindow.qml` away from `QtQuick.Controls`.

## Why this mattered
After the earlier shell-adjacent reductions, `ExplorerWindow.qml` became the next best target because:
- it is a central shell surface
- it still carried a visible amount of stock controls usage
- most of that usage was still structurally lightweight enough to replace with plain `QtQuick` primitives

It was a bigger step than the earlier shell files, but still far safer than jumping directly into a heavy panel dominated by complex controls.

## What changed
Updated:
- `gui/omni/assets/ExplorerWindow.qml`

Removed:
- `import QtQuick.Controls 2.15`

### Replacements performed
- `Label` → `Text`
- toolbar navigation `Button`s → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`
- command-bar deduplicate `Button` → `Rectangle` + `Text` + `MouseArea`
- sidebar `ItemDelegate` → `Rectangle` + `RowLayout` + `MouseArea` + `HoverHandler`
- file-list `ItemDelegate` → `Rectangle` + `RowLayout` + `MouseArea` + `HoverHandler`
- removed `ScrollBar.vertical: ScrollBar { }`

### What remained intentionally untouched
- `TextInput` remains, which is part of Qt Quick rather than Qt Quick Controls
- `ListView` remains, which is also part of Qt Quick
- the file-model-driven interaction flow remains intact

## Measurement
### Before this session
Global `QtQuick.Controls` import count was:
- **43 QML files**

### After this session
Global `QtQuick.Controls` import count is now:
- **42 QML files**

## Practical significance
This is an important step because it proves that even one of the larger shell surfaces can be converted away from stock controls without an immediate architectural collapse.

The no-Controls shell-adjacent set now includes:
- `Taskbar.qml`
- `StartMenu.qml`
- `DesktopIcons.qml`
- `NexusPulse.qml`
- `WindowManager.qml`
- `ExplorerWindow.qml`

## Tradeoffs
This change deliberately favors dependency reduction and UI-ownership over stock convenience widgets.

Potential minor tradeoffs:
- less built-in control behavior from stock delegates/buttons
- custom hover/click behavior must now be maintained manually
- file list no longer uses Controls scroll bar chrome

These are acceptable at this stage because the goal is to shrink dependency surface while preserving core shell behavior.

## Recommended next steps
1. Continue converting shell-level assets before attacking rich multi-control panels.
2. Keep measuring the controls-import count after each pass.
3. Leave highly control-dense panels for later until a replacement strategy for richer control types is in place.
