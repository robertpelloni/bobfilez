# HANDOFF.md — bobfilez Session 43

## Current Status (2026-04-04)
**Version:** 6.0.28  
**Focus:** BTK native provider probe and build/export validation

---

## What Was Done This Session

### 1. Finished Registering `libs/btk` as a Real Submodule
- Repaired the earlier partial BTK clone.
- Checked out BTK's default branch inside `libs/btk` and re-ran the submodule add step successfully.
- Result:
  - `libs/btk` is now a real tracked gitlink in the superproject.

### 2. Ran the First Real BTK In-Place Build Probe
- Executed **`scripts/build_btk_inplace.bat`**.
- Result:
  - BTK **configures successfully** on this machine.
  - BTK then fails during its own compile in:
    - `src/core/kernel/btkinputowner.h`
    - `src/core/kernel/btkinputowner.cpp`
- The observed failure is centered on `Capabilities` / `Q_DECLARE_FLAGS`-style parsing fallout under MSVC.

### 3. Fixed the Initial BTK Package-Hint Bug in bobfilez
- Updated **`cmake/BTKFrameworkSetup.cmake`**.
- The original BTK helper still allowed CMake to pick the **raw source-tree** `cmake/BTKConfig.cmake` template before the generated build-tree config.
- The helper now prefers:
  - `libs/btk/build-btk`
  - other build/install-style prefixes
- This moved the GUI probe from a misleading template-config failure to a real BTK export/readiness failure.

### 4. Ran the First Real BTK-Backed bobfilez GUI Probe
- Executed **`scripts/build_btk_gui.bat`**.
- Result:
  - bobfilez now finds `libs/btk/build-btk/BTKConfig.cmake`
  - but that generated BTK config currently references missing companion files such as:
    - `CopperSpiceLibraryTargets.cmake`
    - `CopperSpiceBinaryTargets.cmake`
    - `BTKMacros.cmake`
    - `BTKDeploy.cmake`
- The active blocker has therefore shifted to:
  - **BTK build/export readiness for downstream consumers**
  - not the old BobUI provider/bootstrap assumptions

### 5. Documented the BTK Probe Findings
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE.md`**.
- The document records:
  - successful `libs/btk` submodule registration
  - the BTK in-place configure/build outcome
  - the build-tree package-export failure encountered by bobfilez
  - the new boundary for next-step work

### 6. Validation and Release Alignment
- Re-ran the normal headless validation path so the versioned state remains current.
- Reconciled release/docs metadata to **6.0.28**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BTK-backed GUI / Omni build | 🟡 Still blocked | `libs/btk` is now a real tracked submodule and BTK configures in place, but BTK currently fails its own build in `btkinputowner.*`, and the generated `BTKConfig.cmake` also references missing companion export files when bobfilez consumes it. |
| BTK native migration plan | 🟡 In progress | The BobUI-specific provider/bootstrap assumptions have been removed from the active native path. The shell-assets layer is complete, five routed-panel controls reductions are done, the photo panels no longer import BobUI namespaces, and the next blocker is explicitly BTK build/export readiness. |
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

4. **Fix BTK readiness before chasing deeper GUI integration**
   - first resolve BTK's own `btkinputowner.*` MSVC build failure and/or produce a valid installed export prefix, then re-probe bobfilez against that finished BTK package surface.
