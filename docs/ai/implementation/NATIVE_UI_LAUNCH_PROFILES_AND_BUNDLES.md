# Native UI Launch Profiles and Runtime Bundles — Option C Phase 4

## Summary
This change evolves the previous launch-config extraction into a clearer policy model.

Instead of representing shell launch policy as a single bag of callback fields, the code now separates two concepts:
- a **runtime bundle**
- a **launch profile**

This makes the active shell bootstrap read less like "assemble some callbacks" and more like "select a named launch package."

## Problem
`NativeUiLaunchConfig` was a useful step because it made launch policy explicit.

However, it still grouped together two different concerns:
1. **runtime bundle concerns**
   - runtime factory
   - QML registration policy
2. **launch profile concerns**
   - shell root QML URL
   - root-object failure behavior
   - selected runtime bundle

Keeping those concepts separate makes future extension clearer.

## What changed

### New files
Added:
- `gui/omni/src/NativeUiRuntimeBundle.hpp`
- `gui/omni/src/NativeUiRuntimeBundle.cpp`
- `gui/omni/src/NativeUiLaunchProfile.hpp`
- `gui/omni/src/NativeUiLaunchProfile.cpp`

### Removed from active build path
Replaced previous launch-config usage with the new profile/bundle model.

### Refactored file
Updated:
- `gui/omni/src/NativeUiBootstrap.cpp`

### Build wiring updated
Updated:
- `gui/CMakeLists.txt`
- `gui/omni/CMakeLists.txt`

## New concepts

### `NativeUiRuntimeBundle`
Owns the runtime-side policy:
- bundle name
- registration routine
- runtime factory

Current default bundle:
- `qt-qml-default-runtime`

### `NativeUiLaunchProfile`
Owns the shell launch policy:
- profile name
- main QML URL
- chosen runtime bundle
- root-object failure handler

Current default profile:
- `omni-shell-default`

## Bootstrap flow after this change
`run_omni_shell(...)` now:
1. constructs `QGuiApplication`
2. creates a default launch profile
3. validates the profile
4. invokes the selected runtime bundle's registration policy
5. creates the selected runtime bundle's runtime
6. applies the profile's root-object failure handler
7. loads the profile's main QML URL
8. enters the event loop

## Why this matters
This is a cleaner Option C design because it moves the code one step closer to explicit, named launch policy selection.

Benefits:
- clearer distinction between runtime policy and shell launch policy
- easier future addition of alternate bundles or profiles
- easier comparison of runtime choices without rewriting bootstrap logic
- better long-term fit for provider-neutral launch selection

## Behavior preserved
This change preserves current active behavior:
- same QQml-based runtime path internally
- same QML type registration set
- same shell root: `qrc:/main.qml`
- same failure behavior on missing root object

## What this does NOT do
This change does **not**:
- add a second working runtime provider
- make BTK usable as the current shell runtime
- introduce user-facing runtime selection
- solve the BTK `QDeclarative*` vs bobfilez `QQml*` mismatch

It is a policy-structure refinement only.

## Recommended next steps
1. If Option C continues, define one or more additional named launch profiles only when they represent a real alternate runtime or shell entry policy.
2. Keep future provider/runtime work focused on adding new bundles or profiles, not on regrowing the bootstrap.
3. Continue provider-neutral GUI progress while the BTK runtime question remains strategically deferred.
