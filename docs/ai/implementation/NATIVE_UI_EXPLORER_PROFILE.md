# Native UI Explorer-Only Launch Profile — Second Real Alternate Profile

## Summary
This change adds a second genuinely alternate named native launch profile.

The existing profile set now includes:
- `omni-shell-default` → full shell root
- `omni-dashboard-only` → dashboard-focused root
- `omni-explorer-only` → explorer-focused root

This continues proving that the launch-profile architecture is supporting real alternate entry modes rather than just structural abstraction.

## What changed

### New QML root
Added:
- `gui/omni/assets/ExplorerShell.qml`

This provides a dedicated root window for explorer-focused launch mode.

### Resource wiring
Updated:
- `gui/omni/assets/qml.qrc`

Added:
- `ExplorerShell.qml`

### Launch-profile wiring
Updated:
- `gui/omni/src/NativeUiLaunchProfile.hpp`
- `gui/omni/src/NativeUiLaunchProfile.cpp`
- `gui/omni/src/NativeUiProfileRegistry.cpp`

Added profile factory:
- `create_explorer_only_launch_profile()`

Registered profile name:
- `omni-explorer-only`

## Launch behavior
### Default profile
- name: `omni-shell-default`
- root QML: `qrc:/main.qml`

### Dashboard-only profile
- name: `omni-dashboard-only`
- root QML: `qrc:/DashboardShell.qml`

### Explorer-only profile
- name: `omni-explorer-only`
- root QML: `qrc:/ExplorerShell.qml`

## ExplorerShell design
`ExplorerShell.qml` is intentionally lightweight:
- full window root
- local `FileModel`
- local minimal `shell` shim
- direct hosting of `ExplorerWindow`

The local `shell` shim only provides the minimal functions ExplorerWindow expects:
- `openPanel(name)`
- `togglePeek(filePath)`

In this focused mode:
- closing via `openPanel("none")` exits the app
- peek behavior is intentionally a no-op rather than pretending the full shell overlay exists

## How to select it
### CLI
```powershell
fo_omni.exe --native-ui-profile=omni-explorer-only
```

### Environment variable
```powershell
$env:BOBFILEZ_NATIVE_UI_PROFILE = "omni-explorer-only"
fo_omni.exe
```

CLI still takes precedence over environment selection.

## Why this profile is useful
This profile is a meaningful alternate mode because it:
- launches directly into the explorer surface
- avoids the surrounding full-shell chrome and routed-panel host
- provides a simpler focused entrypoint for file-manager-specific work

## What this does NOT do
This change does **not**:
- add a second runtime provider
- make BTK usable as the active runtime
- solve the BTK `Declarative` boundary
- claim full shell-parity behavior inside the explorer-only root

It only adds a second real alternate launch profile on the existing runtime path.

## Recommended next steps
1. Keep future profiles real and focused; avoid adding profile variants that are only aliases.
2. Use the explorer-only and dashboard-only profiles as proof that the launch architecture now supports meaningfully different entry roots.
3. Continue provider-neutral GUI work unless a new profile unlocks a concrete workflow need.
