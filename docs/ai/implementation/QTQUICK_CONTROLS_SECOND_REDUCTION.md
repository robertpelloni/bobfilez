# QtQuick.Controls Second Reduction — Shell-Adjacent Follow-Up

## Summary
This session continued the stock `QtQuick.Controls` reduction by converting three additional shell-adjacent assets away from the Controls module.

## Files updated
- `gui/omni/assets/DesktopIcons.qml`
- `gui/omni/assets/NexusPulse.qml`
- `gui/omni/assets/WindowManager.qml`

## Why these were chosen
These files were good follow-up targets because:
- they are part of shell chrome or shell orchestration
- they used either:
  - only `Label`
  - or no actual Controls types at all
- they did not depend on richer controls like `ComboBox`, `CheckBox`, `ProgressBar`, or `Menu`

This made them low-risk reductions consistent with the incremental migration strategy.

## What changed

### 1. `DesktopIcons.qml`
Removed:
- `import QtQuick.Controls 2.15`

Replaced:
- `Label` → `Text`

Result:
- desktop icon rendering remains layout-compatible
- no remaining Controls dependency in this file

## 2. `NexusPulse.qml`
Removed:
- `import QtQuick.Controls 2.15`

Replaced:
- final `Label` → `Text`

Result:
- pulse badge remains functionally identical
- file no longer requires Qt Quick Controls

## 3. `WindowManager.qml`
Removed:
- `import QtQuick.Controls 2.15`

Reason:
- the file was not actually using Controls types
- this was a dead import cleanup similar in spirit to earlier module-surface reductions

## Measurement
### Before this session
The QML tree had:
- **46 files** importing `QtQuick.Controls`

### After this session
The QML tree now has:
- **43 files** importing `QtQuick.Controls`

## Practical significance
This is the second successful reduction pass against the stock controls surface.

Combined with the first pass, bobfilez has now removed `QtQuick.Controls` from five shell-adjacent files:
- `Taskbar.qml`
- `StartMenu.qml`
- `DesktopIcons.qml`
- `NexusPulse.qml`
- `WindowManager.qml`

This is exactly the kind of incremental progress that reduces UI dependency weight without triggering a large rewrite.

## Recommended next steps
1. Continue targeting shell-adjacent files with mostly `Label` / trivial `Button` usage.
2. Delay heavy-control panels until a replacement strategy exists for:
   - `ComboBox`
   - `CheckBox`
   - `ProgressBar`
   - `GroupBox`
   - `Menu` / `MenuItem`
3. Keep measuring the import count after each pass so migration progress stays quantitative.
