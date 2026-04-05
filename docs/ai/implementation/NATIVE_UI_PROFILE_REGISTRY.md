# Native UI Profile Registry — Option C Phase 5

## Summary
This change adds a small registry/helper layer for native shell launch profiles and runtime bundles.

After the previous extraction, the code already had:
- `NativeUiRuntimeBundle`
- `NativeUiLaunchProfile`

However, the bootstrap still selected the default profile directly by calling its constructor/factory function.

This step introduces a dedicated lookup/helper layer so the bootstrap can ask for:
- the default profile name
- the default runtime bundle name
- a launch profile by name
- a runtime bundle by name

That is a small but meaningful shift toward provider-neutral policy selection.

## Problem
Without a lookup layer, the code still had an implicit assumption that:
- the bootstrap knows exactly which default profile factory to call

That is manageable for one profile, but it scales poorly.
As more named launch packages are added, bootstrap logic should not grow into a switchboard.

## What changed

### New files
Added:
- `gui/omni/src/NativeUiProfileRegistry.hpp`
- `gui/omni/src/NativeUiProfileRegistry.cpp`

### Refactored file
Updated:
- `gui/omni/src/NativeUiBootstrap.cpp`

### Build wiring updated
Updated:
- `gui/CMakeLists.txt`
- `gui/omni/CMakeLists.txt`

## New helper surface
The registry/helper currently provides:
- `default_runtime_bundle_name()`
- `default_launch_profile_name()`
- `available_runtime_bundle_names()`
- `available_launch_profile_names()`
- `create_runtime_bundle_by_name(...)`
- `create_launch_profile_by_name(...)`

At the moment, the registry contains one active runtime bundle and one active launch profile.
That is intentional: the value here is not size, but the new selection seam.

## Bootstrap flow after this change
`run_omni_shell(...)` now:
1. asks the registry for the default launch profile name
2. asks the registry to create the corresponding launch profile by name
3. validates the resulting profile
4. proceeds with launch as before

This keeps bootstrap logic focused on launch orchestration rather than profile-selection policy.

## Why this matters
This is the fifth concrete Option C refinement.

It matters because it gives the project a stable place to add future launch packages without regrowing bootstrap code.
That is useful for any later work involving:
- alternate shell entry profiles
- alternate runtime bundles
- test-specific launch setups
- provider-neutral launch selection

## Behavior preserved
This change preserves the current active behavior:
- same shell root
- same runtime bundle
- same QQml-based runtime internally
- same root-object failure handling

## What this does NOT do
This change does **not**:
- add a second runtime provider
- make BTK usable as the active runtime
- introduce user-facing runtime switching
- change the previously documented BTK boundary

It is a small policy-selection refinement only.

## Recommended next steps
1. If additional named launch packages are introduced later, add them through the registry/helper rather than by extending bootstrap logic directly.
2. Keep the bootstrap thin and orchestration-focused.
3. Continue provider-neutral GUI work while BTK remains a research/runtime baseline rather than the active shell provider.
