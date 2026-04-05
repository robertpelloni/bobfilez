# HANDOFF.md — bobfilez Session 42

## Current Status (2026-04-04)
**Version:** 6.0.27  
**Focus:** BTK native-framework switch and BobUI reference replacement

---

## What Was Done This Session

### 1. Retargeted the Active Native UI Path from BobUI to BTK
- Updated the repo's active native-framework direction to **BTK**.
- Added **`cmake/BTKFrameworkSetup.cmake`** and switched the root build to use `BTK_ROOT` / `FO_BTK_ROOT` instead of BobUI-specific provider variables.
- Retargeted GUI / Omni CMake wiring away from a BobUI/Qt6 provider story and toward a BTK / CopperSpice package-discovery model.

### 2. Removed BobUI-Specific Bootstrap and Source-Tree Assumptions
- Updated **`gui/omni/src/main.cpp`** to remove:
  - `#include "OmniQmlRegistration.h"`
  - `OmniUI::registerQmlTypes()`
- Updated **`gui/CMakeLists.txt`** and **`gui/omni/CMakeLists.txt`** to remove the BobUI-specific `OmniUI/omnicore` source inclusion path.
- This makes the active native bootstrap honest about what it depends on instead of silently relying on BobUI internals.

### 3. Replaced Active BobUI QML Imports in Photo Panels
- Updated **`gui/panels/PhotoDevelopPanel.qml`** and **`gui/panels/PhotoLibraryPanel.qml`**.
- Removed BobUI-specific imports:
  - `OmniUI`
  - `OmniLayout`
  - `OmniData`
- Replaced the active BobUI widget usage in those panels with local Qt Quick / Qt Quick Controls primitives.

### 4. Renamed Native Build Scripts to BTK
- Renamed:
  - **`scripts/build_bobui_gui.bat`** → **`scripts/build_btk_gui.bat`**
  - **`scripts/build_bobui_inplace.bat`** → **`scripts/build_btk_inplace.bat`**
- Updated the scripts to use `BTK_ROOT`, `libs/btk`, and BTK-oriented logging/configure flow.

### 5. Updated Project Documentation and Recorded the Architectural Finding
- Added **`docs/ai/implementation/BTK_PROVIDER_SWITCH.md`**.
- Updated the active project-facing docs and model-instruction files so they describe BTK as the current native-framework direction.
- Key architectural finding:
  - **BTK is not laid out like the old BobUI Qt-fork tree**.
  - It is a BTK / CopperSpice-style framework, not a Qt6 drop-in provider with BobUI `OmniUI/omnicore` internals.
  - Therefore, the correct migration is a provider-model switch plus bootstrap cleanup, not a blind string rename.

### 6. Validation and Release Alignment
- Re-ran the normal headless validation path so the versioned state remains current.
- Reconciled release/docs metadata to **6.0.27**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BTK-backed GUI / Omni build | 🟡 Still blocked | The active provider and bootstrap references now point toward BTK instead of BobUI, but BTK is architecturally different from the older BobUI/Qt6 path. A real BTK-compatible GUI build still needs a completed `libs/btk` population and follow-up compatibility work against BTK/CopperSpice packages. |
| BTK native migration plan | 🟡 In progress | The BobUI-specific provider/bootstrap assumptions have been removed from the active native path. The shell-assets layer is complete, five routed-panel controls reductions are done, and the photo panels no longer import BobUI namespaces. Remaining GUI work still needs deeper BTK compatibility analysis. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BTK path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Use the new BTK provider model as the baseline**
   - future native GUI probing should assume BTK / CopperSpice package discovery via `BTK_ROOT`, not the old BobUI/Qt6 provider model.

3. **Continue routed-panel reductions from simplest to richest**
   - `DigitalRotPanel.qml`, `SwarmPanel.qml`, `RecoveryPanel.qml`, `ForensicPanel.qml`, and `GamificationPanel.qml` now provide a broader body of evidence for the dashboard-style selection rule, including one case with a local hover-detail replacement, but future work should still prioritize the most self-contained panels before menu-heavy or form-heavy surfaces.

4. **Validate BTK compatibility incrementally**
   - finish populating `libs/btk`, probe a real BTK build/install prefix, and only then decide how far the current QML/Quick-heavy shell can move toward BTK-native surfaces.
