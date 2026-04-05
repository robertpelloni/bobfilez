# Native UI Bootstrap Seam — First Option C Execution Step

## Summary
This change implements the first concrete code step from the BTK framework decision matrix.

The goal is **not** to make BTK work immediately.
The goal is to reduce direct provider coupling in the active bobfilez native shell bootstrap so future framework decisions touch a smaller surface area.

## Problem
Before this change, `gui/omni/src/main.cpp` directly hardcoded the active declarative runtime path:
- `QGuiApplication`
- `QQmlApplicationEngine`
- `qmlRegisterType(...)`
- root-object failure handling
- QML loading of `qrc:/main.qml`

That meant the shell entrypoint itself was tightly coupled to the current provider choice.

Given the BTK findings, that coupling is now undesirable because:
- BTK is not currently an immediate runtime target for bobfilez's active shell
- future provider changes should not require rewriting the top-level entrypoint again

## What changed

### New files
Added:
- `gui/omni/src/NativeUiBootstrap.hpp`
- `gui/omni/src/NativeUiBootstrap.cpp`

### Refactored file
Updated:
- `gui/omni/src/main.cpp`

### Build wiring updated
Updated:
- `gui/CMakeLists.txt`
- `gui/omni/CMakeLists.txt`

## New structure
`main.cpp` is now a minimal provider-neutral launcher:
- it includes `NativeUiBootstrap.hpp`
- it delegates to `fo::gui::run_omni_shell(argc, argv)`

The runtime-specific logic moved into `NativeUiBootstrap.cpp`.

## Internal seam introduced
Inside `NativeUiBootstrap.cpp`, the active runtime is now isolated behind a small internal interface:
- `DeclarativeUiRuntime`

Current implementation:
- `QtQmlDeclarativeUiRuntime`

This preserves existing behavior while reducing the blast radius of future provider/runtime changes.

## Behavior preserved
The following behavior remains unchanged:
- `QGuiApplication` creation
- Omni QML type registration for:
  - `FileModel`
  - `TreemapModel`
  - `NativeMarkdownView`
- loading `qrc:/main.qml`
- exiting with `-1` if the root object fails to load
- normal event loop startup via `app.exec()`

## Why this matters
This is the first real implementation step for **Option C** from the framework decision matrix:
- keep bobfilez on a modern QQml-style shell path
- reduce provider assumptions
- avoid pretending BTK is the immediate runtime target

The practical benefit is that future runtime/provider work can now focus on:
- the bootstrap adapter layer
- engine-creation strategy
- registration and loading policy

instead of repeatedly rewriting `main.cpp` directly.

## What this does NOT do
This change does **not**:
- make BTK `Declarative` available
- make BTK compatible with `QQmlApplicationEngine`
- solve the BTK `QDeclarative*` vs bobfilez `QQml*` generation mismatch
- introduce a second working provider implementation yet

This is intentionally a seam-reduction change, not a false compatibility claim.

## Additional cleanup included
While updating the GUI bootstrap source lists, `gui/CMakeLists.txt` was also aligned to include:
- `omni/src/TreemapModel.cpp`
- `omni/src/TreemapModel.h`

This keeps the GUI source list coherent with the QML type registration path.

## Recommended next steps
1. Continue extracting provider-specific assumptions into narrow seams.
2. Keep runtime selection isolated from shell/business logic.
3. Only attempt additional provider work where the abstraction boundary is clear and honest.
4. Continue provider-neutral GUI progress even while BTK remains a research/runtime probe rather than the active shell provider.
