# BTK Alias Layer - Session Notes

## What was added
### CMake package wrappers
- `cmake/BTKConfig.cmake`
- `cmake/BTKConfigVersion.cmake`
- `cmake/BTKMacros.cmake`
- `cmake/BTKDeploy.cmake`

### New BTK conveniences
For each built framework component, the project now exposes both:
- `BTK::Btk<Component>`
- `BTK::<Component>`

Examples:
- `BTK::BtkCore`
- `BTK::Core`
- `BTK::BtkGui`
- `BTK::Gui`

These are compatibility aliases over the existing exported `CopperSpice::Cs*` targets.

### Wrapper APIs
- `BTK_RESOURCES(...)` forwards to `COPPERSPICE_RESOURCES(...)`
- `btk_copy_library(...)` forwards to `cs_copy_library(...)`
- `btk_copy_plugins(...)` forwards to `cs_copy_plugins(...)`

### First-wave public C++ alias headers
- `src/core/global/btkcore.h`
- `src/core/global/btkstring.h`
- `src/core/global/btkpointer.h`
- `src/core/global/btksignal.h`

These are exported through the QtCore include surface via:
- `<QtCore/BTKCore>`
- `<QtCore/BTKString>`
- `<QtCore/BTKPointer>`
- `<QtCore/BTKSignal>`

They provide a small, compatibility-first public alias layer for selected `CsString`, `CsPointer`, and `CsSignal` types without changing ABI.

## Why this matters
This creates a safe migration path:
1. downstreams can begin writing BTK-branded CMake today
2. the existing internal/export target graph remains stable
3. broad symbol/file renames can happen later in controlled phases

## What this does NOT do
- It does not rename ABI symbols.
- It does not rename most headers or source files.
- It does not remove `CopperSpice::Cs*` compatibility.
- It only adds a small first-wave of `Btk*` C++ aliases; it does not yet cover the broader public API.

## Recommended next implementation step
Use the generated rename inventory to introduce carefully-scoped public header aliases for the safest high-value API surfaces first.
