# QtQuick.Controls Main Host Reduction — Notification Surface Cleanup

## Summary
This session removed `QtQuick.Controls` usage from `gui/omni/assets/main.qml` by converting the localized notification-center surface to plain `QtQuick` primitives.

## Why this was a good next target
`main.qml` still imported `QtQuick.Controls`, but the actual usage was narrowly concentrated in the notification drawer section.

That made it a strong candidate because:
- the usage was localized
- it did not require touching the many routed panels hosted by `main.qml`
- most of the controls involved were still lightweight (`Label` and one trivial `Button`)

## What changed
Updated:
- `gui/omni/assets/main.qml`

Removed:
- `import QtQuick.Controls 2.15`

### Replacements performed
- `Label` → `Text`
- the `Clear All` button → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`

## What remained intact
- notification-center overlay routing
- dashboard/start/peek interactions
- notification list structure
- shell panel routing behavior

## Measurement
### Before this session
Global `QtQuick.Controls` import count was:
- **41 QML files**

### After this session
Global `QtQuick.Controls` import count is now:
- **40 QML files**

## Practical significance
This is another strong sign that the controls-reduction strategy is working best when:
- the target surface is shell-host or shell-adjacent
- stock controls are locally contained
- the migration can avoid entangling itself with the larger routed panel system

## Recommended next steps
1. Continue the same pattern on similarly localized shell surfaces.
2. Keep avoiding the heavy panel forms until richer control replacement strategies are ready.
3. Preserve the import-count measurement after each pass so migration progress remains quantitative.
