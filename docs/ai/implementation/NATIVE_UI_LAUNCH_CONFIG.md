# Native UI Launch Config — Option C Phase 3

## Summary
This change continues the native UI bootstrap refactor by making launch policy explicit data.

After the prior two steps, the active shell path already had separate seams for:
- bootstrap orchestration
- declarative runtime adapter/factory
- QML type-registration policy

This step adds a dedicated launch configuration object so the bootstrap no longer hardcodes:
- main QML URL
- registration bundle choice
- default runtime factory choice
- root-object failure policy

## Problem
Even after the earlier extractions, `NativeUiBootstrap.cpp` still implicitly owned the active shell launch policy.

That meant the bootstrap still contained hardcoded answers to questions like:
- which QML file is the shell root?
- which registration routine should run?
- which runtime factory should be used?
- what should happen if the root object fails to load?

These are not bootstrap mechanics.
They are launch policy.

## What changed

### New files
Added:
- `gui/omni/src/NativeUiLaunchConfig.hpp`
- `gui/omni/src/NativeUiLaunchConfig.cpp`

### Refactored file
Updated:
- `gui/omni/src/NativeUiBootstrap.cpp`

### Build wiring updated
Updated:
- `gui/CMakeLists.txt`
- `gui/omni/CMakeLists.txt`

## New structure
### `NativeUiLaunchConfig`
A new configuration object now owns:
- `main_qml`
- `register_types`
- `runtime_factory`
- `object_created_handler`

It also exposes:
- `bool is_valid() const`

### Default launch-policy factory
Added:
- `create_default_omni_ui_launch_config()`

This centralizes the current default launch policy for the shell.

## Bootstrap flow after this change
`run_omni_shell(...)` now does the following:
1. construct `QGuiApplication`
2. create a `NativeUiLaunchConfig`
3. validate the config
4. invoke registration policy from config
5. create the runtime from config
6. apply root-object failure handler from config (if provided)
7. load the configured main QML URL
8. enter the event loop

## Why this matters
This is the third concrete Option C reduction in provider coupling.

It matters because future changes can now target launch policy separately from:
- shell entrypoint shape
- runtime adapter implementation
- registration implementation

That makes it easier to evolve or test things like:
- alternate shell entry QML files
- alternate registration bundles
- alternate runtime factories
- alternate failure policies

without expanding `main.cpp` or re-entangling the bootstrap layer.

## Behavior preserved
This change preserves the current behavior:
- still launches the same shell root: `qrc:/main.qml`
- still uses the same active QQml-based runtime internally
- still registers the same QML types
- still exits with `-1` on root-object creation failure

## What this does NOT do
This change does **not**:
- add a second real runtime
- make BTK the active runtime
- solve the BTK declarative mismatch
- introduce user-facing runtime selection

It only turns implicit launch policy into explicit configuration.

## Recommended next steps
1. If Option C continues, consider introducing a small runtime/registration bundle concept so the bootstrap config can refer to named launch profiles rather than individual function slots.
2. Keep future provider/runtime work focused on the seams already extracted instead of re-growing the bootstrap.
3. Continue provider-neutral GUI work while the BTK runtime question remains strategically deferred.
