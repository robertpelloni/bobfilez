# Native UI Profile Environment Selection — Option C Phase 6

## Summary
This change adds a tiny registry-backed selection path for native shell launch profiles.

The bootstrap no longer just asks the registry for the default profile by name.
It now asks the registry to resolve the launch profile from environment-driven selection logic with a safe fallback to the default profile.

## Goal
Allow alternate named launch profiles to be exercised without editing bootstrap code.

This is intentionally small and conservative:
- one environment variable
- named profile lookup through the registry
- warning + fallback if the requested profile does not exist
- unchanged default behavior when no override is set

## What changed

### Files updated
- `gui/omni/src/NativeUiProfileRegistry.hpp`
- `gui/omni/src/NativeUiProfileRegistry.cpp`
- `gui/omni/src/NativeUiBootstrap.cpp`

## New helper surface
Added registry functions:
- `native_ui_profile_environment_variable()`
- `selected_launch_profile_name()`
- `create_launch_profile_from_environment()`

## Environment variable
The active selection variable is:
- `BOBFILEZ_NATIVE_UI_PROFILE`

Behavior:
- if unset or empty:
  - use the default launch profile
- if set to a known profile name:
  - create that named profile
- if set to an unknown profile name:
  - emit a warning
  - fall back to the default launch profile

## Bootstrap flow after this change
`run_omni_shell(...)` now:
1. constructs `QGuiApplication`
2. asks the registry to resolve the launch profile from environment selection
3. validates the resulting profile
4. continues launch as before

## Why this matters
This is the next clean Option C refinement because it adds a real policy-selection seam without expanding bootstrap complexity.

Benefits:
- future alternate profiles can be tested without editing code in the bootstrap
- bootstrap stays orchestration-focused
- default behavior remains unchanged
- runtime/provider experiments can remain explicit and opt-in

## Behavior preserved
With no environment override set, behavior is unchanged:
- same launch profile
- same runtime bundle
- same root QML
- same failure policy
- same QQml-based active runtime

## What this does NOT do
This change does **not**:
- add a second runtime provider
- make BTK the active runtime
- change the current default shell launch path
- solve the BTK `Declarative` boundary

It only adds a clean selection hook for future named profiles.

## Recommended next steps
1. Only add new named launch profiles when they represent a real policy difference worth testing.
2. Keep selection logic inside the registry/helper layer rather than re-growing bootstrap code.
3. Continue provider-neutral GUI cleanup while BTK remains a documented research baseline rather than the active shell runtime.
