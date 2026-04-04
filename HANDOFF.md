# HANDOFF.md — bobfilez Session 30

## Current Status (2026-04-04)
**Version:** 6.0.15  
**Focus:** BobUI registration alignment and current-layout correction

---

## What Was Done This Session

### 1. Corrected the BobUI Source Layout Assumption
- Found that bobfilez GUI CMake was still pointing at an outdated BobUI path:
  - `libs/bobui/OmniUI/core`
- The current BobUI tree actually uses:
  - `libs/bobui/OmniUI/omnicore`
- Updated both GUI CMake entrypoints to target the real `omnicore` layout.

### 2. Wired BobUI Sources and Include Paths Structurally
- Updated **`gui/CMakeLists.txt`** and **`gui/omni/CMakeLists.txt`** to:
  - gather BobUI sources recursively from `omnicore/src`
  - include current BobUI headers from `omnicore/include`
  - include supporting dependency paths for:
    - `deps/juce`
    - `deps/imgui`
- This shifts the integration from stale assumptions toward the real BobUI tree shape.

### 3. Added a Real BobUI Registration Call to Startup
- Updated **`gui/omni/src/main.cpp`** to include:
  - `OmniQmlRegistration.h`
- Added a real startup call to:
  - `OmniUI::registerQmlTypes();`
- bobfilez now structurally attempts to back imported BobUI namespaces (`OmniUI`, `OmniLayout`, `OmniData`, etc.) before registering its local bridge types.

### 4. Documented the Difference Between Structural Wiring and Buildability
- Added **`docs/ai/implementation/BOBUI_REGISTRATION_WIRING.md`**.
- Important finding recorded there:
  - this fixes integration correctness
  - but it does **not** solve the deeper BobUI provider blocker
  - full GUI build is still blocked by missing `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` support in the current BobUI provider surface on this machine

### 5. Validation and Release Alignment
- Re-ran the normal headless validation path to keep versioned binaries aligned.
- Reconciled release/docs metadata to **6.0.15**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BobUI / Omni shell build | 🟡 Still blocked | BobUI integration is now more structurally correct (real omnicore path + real registration call), but the provider surface still lacks `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` for bobfilez's present GUI targets. |
| BobUI-native migration plan | 🟡 In progress | Four dependency-surface reductions are complete and BobUI registration is now structurally wired. Removing most QML is still expensive; removing `QtQuick` itself is still incompatible with current BobUI because BobUI widgets are Quick-based. |
| BOBGUI adoption | ⚪ Not recommended | `bobgui` is available for study, but it is not the right primary UI foundation for bobfilez’s current Qt/QML/Omni direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Stay on the BobUI path for native UI work**
   - Treat `bobgui` as a comparison/reference library, not the main bobfilez shell foundation.

2. **Use the improved structural wiring as the new baseline**
   - future BobUI GUI probing should assume the current `omnicore` path and the real `OmniUI::registerQmlTypes()` startup call.

3. **Move to the next real dependency cluster**
   - with GraphicalEffects gone, focus the next pass on stock `QtQuick.Controls` / `QtQuick.Layouts` surfaces.

4. **Do not plan around removing `QtQuick` yet**
   - the current BobUI implementation itself is Quick-based, so a true no-Quick target is premature.
