# Native UI Profile Listing — Option C Phase 8

## Summary
This change adds a small user-facing discovery affordance for the native launch-profile system.

The project now supports multiple named launch profiles, but until this change the only way to know what profiles existed was to inspect code or documentation.

This session adds a dedicated CLI flag to list the available native UI launch profiles and show the active selection precedence rule.

## New CLI flag
The new flag is:
- `--list-native-ui-profiles`

## Behavior
When passed to the native shell entrypoint, the process:
1. prints the available launch profile names
2. marks the default profile
3. prints the profile-selection precedence rule
4. exits without attempting to start the GUI runtime

## What changed

### Files updated
- `gui/omni/src/NativeUiProfileRegistry.hpp`
- `gui/omni/src/NativeUiProfileRegistry.cpp`
- `gui/omni/src/NativeUiBootstrap.cpp`

### New helper surface
Added:
- `native_ui_profile_list_argument()`
- `should_list_launch_profiles(int argc, char *argv[])`

## Output shape
The listing currently prints:
- the available profile names
- `(default)` next to the default profile
- the precedence rule:
  - `--native-ui-profile=<name>`
  - `BOBFILEZ_NATIVE_UI_PROFILE`
  - default profile

## Why this matters
This is a small but real usability improvement:
- launch profiles are now discoverable without reading code
- the profile architecture is operationally visible, not just internally structured
- profile experimentation is easier and safer

## Behavior preserved
This change preserves normal behavior when the flag is absent.
The default launch flow is unchanged.

## What this does NOT do
This change does **not**:
- add another runtime provider
- change the default shell path
- solve the BTK `Declarative` boundary
- modify the actual selected profile unless the listing flag is used

It only adds profile discovery.

## Recommended next steps
1. Keep future profile management discoverable through the same registry/helper layer.
2. Add new profiles only when they represent real alternate launch workflows.
3. Continue provider-neutral GUI work while BTK remains a research/runtime baseline rather than the active shell provider.
