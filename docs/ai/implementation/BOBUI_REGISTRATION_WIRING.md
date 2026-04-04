# BobUI Registration Wiring — From Aspirational to Structural Integration

## Summary
This session corrected the bobfilez native GUI wiring so it reflects the **current BobUI repository layout** and actually calls BobUI's QML type registration during startup.

## Why this mattered
Earlier auditing showed an important mismatch:
- bobfilez QML already referenced namespaces like:
  - `OmniUI 1.0`
  - `OmniLayout 1.0`
  - `OmniData 1.0`
- but bobfilez startup was only registering:
  - `FileModel`
  - `TreemapModel`
- and the GUI CMake wiring still pointed at a stale BobUI path:
  - `libs/bobui/OmniUI/core`

The current BobUI tree no longer uses that path. It uses:
- `libs/bobui/OmniUI/omnicore`

So even before the broader Qt module blocker is solved, the integration structure itself needed to be corrected.

## What changed

### 1. GUI CMake path correction
Updated:
- `gui/CMakeLists.txt`
- `gui/omni/CMakeLists.txt`

Changes:
- switched BobUI source root from the stale path:
  - `OmniUI/core`
- to the current path:
  - `OmniUI/omnicore`
- added recursive BobUI source discovery from:
  - `libs/bobui/OmniUI/omnicore/src/*.cpp`
- added include paths for:
  - `omnicore/include`
  - `omnicore/src`
  - `deps/juce`
  - `deps/imgui`

## 2. Real BobUI registration call in startup
Updated:
- `gui/omni/src/main.cpp`

Added:
- `#include "OmniQmlRegistration.h"`
- `OmniUI::registerQmlTypes();`

This means the startup path is now structurally prepared to back QML imports such as:
- `OmniUI 1.0`
- `OmniLayout 1.0`
- `OmniData 1.0`
- `OmniAudio 1.0`
- `OmniNet 1.0`
- `OmniGfx 1.0`

before bobfilez registers its own local bridge namespaces:
- `Omni.File`
- `Omni.Viz`
- `Omni.Native`

## Important limitation
This does **not** mean the full GUI build now succeeds.

The previously identified blocker still remains:
- the current BobUI provider surface still does not satisfy bobfilez's required `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` consumer path in a full build on this machine

So this session fixes the **structural integration correctness**, not the full GUI buildability.

## Why this is still valuable
This change removes a major source of architectural ambiguity.

Before:
- bobfilez claimed BobUI-oriented imports in some QML files
- but bootstrap and CMake were not actually aligned with the current BobUI layout

After:
- the codebase now reflects the real BobUI repository structure
- the startup path now explicitly attempts to register BobUI's QML surfaces
- future GUI failures are easier to interpret because they are less likely to be caused by stale pathing assumptions

## Practical significance
This is an integration hygiene step, but an important one.
It means that once the remaining Qt module/provider surface becomes sufficient, bobfilez is better positioned to consume BobUI namespaces in a real way rather than only in documentation or aspirational imports.

## Recommended next steps
1. Re-run BobUI consumer probing after any future BobUI module-surface expansion.
2. Audit whether the recursive `omnicore` source inclusion should later be narrowed to a curated subset once the full GUI path is buildable.
3. Continue reducing stock `QtQuick.Controls` / `QtQuick.Layouts` dependence now that BobUI registration is at least structurally wired.
