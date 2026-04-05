# BTK Provider Switch — Replacing the Active BobUI/Qt Path

## Summary
This session retargeted bobfilez's active native-framework integration path from the older BobUI/Qt-fork setup to **BTK** (`https://github.com/robertpelloni/btk`).

The switch is broader than a naming change. BobUI and BTK have different packaging and architecture models, so the work in this session focused on replacing the active build/bootstrap assumptions rather than pretending BTK is just another Qt6 provider.

## Key finding
### BobUI vs BTK are not equivalent integration shapes
The previous BobUI path assumed:
- a Qt-fork-style provider
- `Qt6::*` package discovery
- a BobUI-specific `OmniUI/omnicore` source tree
- a BobUI-specific `OmniQmlRegistration.h` bootstrap call

BTK, by contrast, currently presents itself as:
- a BTK / CopperSpice-line framework
- `find_package(BTK CONFIG)` entry points
- BTK/CopperSpice imported targets rather than `Qt6::*`
- no BobUI-style `OmniUI/omnicore` source subtree

That means the correct migration is **not** a blind string replacement of `Qt6` to `BTK`. It requires removing BobUI-specific source assumptions and switching the native build system to a BTK-aware provider model.

## What changed
### 1. Build-system retargeting
Updated:
- `CMakeLists.txt`
- `cmake/BTKFrameworkSetup.cmake`
- `gui/CMakeLists.txt`
- `gui/omni/CMakeLists.txt`

Changes:
- replaced the old BobUI helper inclusion with a new BTK-focused helper
- introduced `FO_BTK_ROOT` / `BTK_ROOT` discovery instead of `FO_BOBUI_ROOT` / `BOBUI_ROOT`
- changed GUI / Omni provider messaging from BobUI/Qt6 to BTK
- removed BobUI `omnicore` source inclusion from GUI targets
- removed the old assumption that GUI targets should link through `Qt6::*`
- introduced BTK/CopperSpice target resolution through a helper function that accepts:
  - `BTK::Core`
  - `BTK::BtkCore`
  - `CopperSpice::CsCore`
  - and equivalent forms for other components

### 2. Native bootstrap cleanup
Updated:
- `gui/omni/src/main.cpp`

Changes:
- removed `#include "OmniQmlRegistration.h"`
- removed `OmniUI::registerQmlTypes()`
- documented that the native bootstrap now only registers bobfilez-local bridge types

This was necessary because that bootstrap was specifically tied to BobUI's internal `OmniQmlRegistration` surface, which does not exist in BTK.

### 3. BobUI-specific QML imports reduced
Updated:
- `gui/panels/PhotoDevelopPanel.qml`
- `gui/panels/PhotoLibraryPanel.qml`

Changes:
- removed `import OmniUI 1.0`
- removed `import OmniLayout 1.0`
- removed `import OmniData 1.0`
- replaced `OmniUI.Button`, `OmniUI.ListView`, `OmniUI.Slider`, `OmniUI.Dial`, and `OmniUI.ProgressBar` usage with local Qt Quick / Qt Quick Controls primitives

This prevents those panels from depending on the BobUI-specific QML namespace after the provider switch.

### 4. Script renaming
Renamed:
- `scripts/build_bobui_gui.bat` → `scripts/build_btk_gui.bat`
- `scripts/build_bobui_inplace.bat` → `scripts/build_btk_inplace.bat`

Updated behavior:
- use `BTK_ROOT`
- default to `libs/btk`
- use BTK-oriented logging/messages
- configure BTK in place with a standard CMake/Ninja flow rather than BobUI's Qt `configure.bat` flow

### 5. Repo metadata / docs direction
Updated:
- `.gitmodules`
- `README.md`
- `DEPLOY.md`
- `ARCHITECTURE.md`
- `docs/ROADMAP.md`
- `docs/VISION.md`
- `.github/copilot-instructions.md`
- `GPT.md`
- `GEMINI.md`
- `CLAUDE.md`
- `MEMORY.md`

These files now describe BTK as the active native-framework direction rather than the older BobUI/Qt-fork path.

## Important limitation
This session **does not claim that bobfilez is now fully BTK-native**.

What it does establish is:
- the active build-system references no longer pretend the project is still on BobUI/Qt6
- the native bootstrap no longer depends on BobUI-specific internals
- the remaining BTK integration work is now more honest and better scoped

The main follow-up challenge is framework compatibility:
- BTK's current package surface and API model differ from the previous Qt6/BobUI assumptions
- the existing QML/Quick-heavy shell still needs further retargeting or adaptation to become a real BTK-native consumer

## Recommended next steps
1. Populate `libs/btk` fully and validate the BTK package exports with a longer clone/build window.
2. Re-probe `FO_BUILD_GUI=ON` / `FO_BUILD_OMNI=ON` against a real BTK build/install prefix.
3. Continue removing the remaining BobUI-specific or Qt6-specific assumptions from active docs and GUI wiring.
4. Decide whether the long-term BTK path should keep QML composition, migrate toward CopperSpice declarative compatibility, or move to more explicitly BTK-native surfaces.
