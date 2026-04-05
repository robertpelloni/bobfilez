# Native UI Runtime Policy Extraction — Option C Phase 2

## Summary
This change continues the provider-coupling reduction started by the native UI bootstrap seam.

The first seam moved runtime-specific behavior out of `main.cpp` and into `NativeUiBootstrap.cpp`.
This second step goes further by separating:
- bootstrap orchestration
- declarative runtime construction
- QML type-registration policy

The active runtime remains unchanged, but the responsibilities are now more cleanly isolated.

## Problem
After the first seam extraction, `NativeUiBootstrap.cpp` still owned too many unrelated concerns:
- bootstrap orchestration
- current declarative runtime adapter implementation
- runtime factory selection
- QML type-registration policy
- root-object failure handling

That was better than a hardcoded `main.cpp`, but it still mixed:
- "how the shell starts"
- "which runtime implementation is used"
- "which QML types are exported"

## What changed

### New files
Added:
- `gui/omni/src/NativeUiRuntime.hpp`
- `gui/omni/src/NativeUiRuntime.cpp`
- `gui/omni/src/OmniQmlRegistration.hpp`
- `gui/omni/src/OmniQmlRegistration.cpp`

### Refactored file
Updated:
- `gui/omni/src/NativeUiBootstrap.cpp`

### Build wiring updated
Updated:
- `gui/CMakeLists.txt`
- `gui/omni/CMakeLists.txt`

## New responsibility split

### `main.cpp`
- minimal launcher only
- delegates to `fo::gui::run_omni_shell(...)`

### `NativeUiBootstrap.cpp`
- bootstrap orchestration only
- creates `QGuiApplication`
- invokes QML registration policy
- obtains the default declarative runtime from the runtime factory
- wires the root-object failure handler
- loads `qrc:/main.qml`

### `NativeUiRuntime.cpp`
- owns the active declarative runtime adapter implementation
- currently provides:
  - `QtQmlDeclarativeUiRuntime`
- owns the default runtime factory:
  - `create_default_declarative_ui_runtime()`

### `OmniQmlRegistration.cpp`
- owns the current QML type-registration policy
- currently registers:
  - `FileModel`
  - `TreemapModel`
  - `NativeMarkdownView`

## Why this matters
This is a better fit for Option C because it narrows the provider-sensitive surfaces further.

Instead of one file owning everything, the project now has distinct seams for:
1. **bootstrap flow**
2. **runtime implementation/factory**
3. **QML registration policy**

That makes future work easier in several directions:
- testing a different runtime selection path
- changing registration policy without rewriting the bootstrap
- evolving the runtime adapter without touching `main.cpp`

## Behavior preserved
This change preserves the active behavior:
- still uses `QGuiApplication`
- still uses the current QQml-based runtime internally
- still registers the same QML types
- still loads `qrc:/main.qml`
- still exits with `-1` on root-object creation failure

## What this does NOT do
This change does **not**:
- add a second real runtime provider
- make BTK work as the active native runtime
- solve the `QQml*` vs `QDeclarative*` generation mismatch
- change the shell asset/runtime model itself

It is a responsibility extraction step, not a framework-compatibility claim.

## Recommended next steps
1. Continue keeping runtime/provider assumptions isolated behind narrow seams.
2. If additional provider-neutral cleanup is desired, consider separating shell launch configuration (main QML URL, failure policy, registration bundle) into a small configuration object.
3. Keep the active runtime unchanged until the project intentionally chooses a new runtime/provider path.
