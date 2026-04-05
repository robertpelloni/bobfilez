# Native UI Profile CLI Selection — Option C Phase 7

## Summary
This change extends the native shell launch-profile selection seam so profile choice can be made from the command line as well as the environment.

The active bootstrap still uses the same registry/helper layer and the same default behavior, but selection precedence is now:
1. command-line override
2. environment variable override
3. default profile

## Goal
Allow real launch-profile experiments without editing bootstrap code and without requiring environment mutation when a one-off CLI override is more convenient.

## What changed

### Files updated
- `gui/omni/src/NativeUiProfileRegistry.hpp`
- `gui/omni/src/NativeUiProfileRegistry.cpp`
- `gui/omni/src/NativeUiBootstrap.cpp`

### New helper surface
Added:
- `native_ui_profile_argument_prefix()`
- `selected_launch_profile_name(int argc, char *argv[])`
- `create_launch_profile_from_selection(int argc, char *argv[])`

## Active CLI override
The active command-line selector is:
- `--native-ui-profile=<name>`

## Selection precedence
The new selection order is:
1. `--native-ui-profile=<name>` if present and non-empty
2. `BOBFILEZ_NATIVE_UI_PROFILE` if set and non-empty
3. default launch profile otherwise

If the requested profile name is unknown:
- emit a warning
- fall back to the default launch profile

## Bootstrap flow after this change
`run_omni_shell(...)` now:
1. constructs `QGuiApplication`
2. asks the registry to resolve the launch profile from CLI/env/default selection
3. validates the resulting profile
4. launches as before

## Why this matters
This is the next logical refinement after environment-driven selection:
- easier one-off profile experiments
- cleaner operational override path
- no bootstrap growth
- same provider-neutral structure

## Behavior preserved
If no CLI override and no env override are used, behavior remains unchanged.

## What this does NOT do
This change does **not**:
- add another runtime provider
- make BTK the active runtime
- change the default launch profile
- solve the BTK `Declarative` boundary

It only extends the profile-selection seam.

## Recommended next steps
1. Only add additional named profiles when they represent a meaningful alternate launch policy.
2. Keep selection rules centralized in the registry/helper layer.
3. Continue provider-neutral GUI work while BTK remains a research/runtime baseline rather than the active shell provider.
