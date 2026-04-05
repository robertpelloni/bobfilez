# Native UI Dashboard-Only Launch Profile — First Real Alternate Profile

## Summary
This change adds the first genuinely alternate named native shell launch profile.

Until now, the launch-profile architecture had been progressively extracted and made selectable, but there was still only one real profile behind the abstractions.

This session adds a second, meaningful launch mode:
- **`omni-dashboard-only`**

This profile launches a dedicated dashboard-focused shell root instead of the full Omni shell root.

## Why this profile is real
This is not a fake alias for the same root QML.

The existing default shell profile launches:
- `qrc:/main.qml`

The new dashboard-only profile launches:
- `qrc:/DashboardShell.qml`

So it represents a real alternate launch behavior rather than a naming exercise.

## What changed

### New QML root
Added:
- `gui/omni/assets/DashboardShell.qml`

This provides a dedicated root window for the dashboard-only mode.

### Resource wiring
Updated:
- `gui/omni/assets/qml.qrc`

Added:
- `DashboardShell.qml`

### Launch-profile wiring
Updated:
- `gui/omni/src/NativeUiLaunchProfile.hpp`
- `gui/omni/src/NativeUiLaunchProfile.cpp`
- `gui/omni/src/NativeUiProfileRegistry.cpp`

Added profile factory:
- `create_dashboard_only_launch_profile()`

Registered profile name:
- `omni-dashboard-only`

## Launch behavior
### Default profile
- name: `omni-shell-default`
- root QML: `qrc:/main.qml`

### New dashboard-only profile
- name: `omni-dashboard-only`
- root QML: `qrc:/DashboardShell.qml`

## How to select it
### CLI
```powershell
fo_omni.exe --native-ui-profile=omni-dashboard-only
```

### Environment variable
```powershell
$env:BOBFILEZ_NATIVE_UI_PROFILE = "omni-dashboard-only"
fo_omni.exe
```

CLI still takes precedence over environment selection.

## DashboardShell design
`DashboardShell.qml` is intentionally lightweight:
- full window root
- dark gradient background
- centered `Dashboard` surface
- no fake shell-window manager complexity

This makes it a good first alternate profile because it is:
- meaningfully different from the full shell
- simpler to reason about
- useful for future diagnostics/demo/telemetry-focused launch modes

## Why this matters
This is the first proof that the launch-profile architecture is now doing real work.

It demonstrates that:
- the profile registry is not just abstraction scaffolding
- alternate launch behavior can now be added without editing bootstrap logic
- the current Option C architecture can support real named shell entry modes

## What this does NOT do
This change does **not**:
- add a second runtime provider
- make BTK usable as the active runtime
- change the default shell profile
- solve the BTK `Declarative` boundary

It only adds the first real alternate launch profile on the existing runtime path.

## Recommended next steps
1. Keep additional profiles meaningful; do not add profile variants unless they represent a real alternate launch mode.
2. Consider whether future diagnostics/demo profiles should reuse the dashboard-only pattern.
3. Keep bootstrap logic unchanged when adding future profiles; extend the profile registry instead.
