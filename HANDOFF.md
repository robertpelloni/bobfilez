# HANDOFF.md — bobfilez Session 44

## Current Status (2026-04-04)
**Version:** 6.0.29  
**Focus:** BTK native provider probe round 2 — build-tree package prep and narrowed blocker

---

## What Was Done This Session

### 1. Added a Dedicated BTK Build-Tree Package Prep Step
- Added:
  - **`scripts/prepare_btk_buildtree_package.py`**
  - **`scripts/prepare_btk_buildtree_package.bat`**
- Updated **`scripts/build_btk_gui.bat`** to run the prep step before probing bobfilez's BTK-backed GUI configure.

### 2. Repaired the Earlier Probe Workaround
- The first workaround around BTK's generated package layout still produced a bad `_IMPORT_PREFIX` for imported targets.
- The new prep step now:
  - copies the generated BTK/CopperSpice export files into `libs/btk/build-btk`
  - copies the required BTK/CopperSpice macro/deploy helpers there as well
  - patches the prepared target-export files so `_IMPORT_PREFIX` points explicitly at `libs/btk/build-btk`
- This removes the earlier misleading package-layout breakage and makes the downstream probe much more honest.

### 3. Re-Ran the BTK-Backed bobfilez GUI Probe
- Executed **`scripts/build_btk_gui.bat`** again.
- Result:
  - bobfilez now gets past the earlier missing-companion-file and bad-import-prefix issues
  - the next failure is now:
    - imported target `CopperSpice::CsCore` expects `libs/btk/build-btk/lib/CsCore2.1.lib`
    - that file does not exist
- This ties the active consumer failure directly back to BTK's own incomplete build output.

### 4. Sharpened the BTK Readiness Conclusion
- The remaining blocker is now even more specific:
  1. BTK still fails its own build in `btkinputowner.*`
  2. because that build does not finish, the expected BTK libraries never appear in `build-btk/lib` / `build-btk/bin`
  3. bobfilez therefore cannot yet consume BTK successfully as a native framework provider
- This is stronger evidence than the first probe because the package-layout/export-surface problems are no longer masking the underlying cause.

### 5. Documented the Probe Round 2 Findings
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND2.md`**.
- The document records:
  - the new build-tree prep step
  - the fixed import-prefix handling
  - the narrowed failure condition now centered on missing BTK libraries after an incomplete BTK build

### 6. Validation and Release Alignment
- Re-ran the normal headless validation path so the versioned state remains current.
- Reconciled release/docs metadata to **6.0.29**.

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BTK-backed GUI / Omni build | 🟡 Still blocked | `libs/btk` is now a real tracked submodule, BTK configures in place, and the build-tree package prep step gets bobfilez past the earlier export-layout issues. The current blocker is now explicitly that BTK has not produced `CsCore2.1.lib` / related binaries because BTK's own build still fails in `btkinputowner.*`. |
| BTK native migration plan | 🟡 In progress | The BobUI-specific provider/bootstrap assumptions have been removed from the active native path. The shell-assets layer is complete, five routed-panel controls reductions are done, the photo panels no longer import BobUI namespaces, and the next blocker is explicitly missing BTK build outputs after an incomplete BTK build. |
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
   - first resolve BTK's own `btkinputowner.*` MSVC build failure so BTK can actually produce `CsCore2.1.lib` and related binaries, then re-probe bobfilez against that finished BTK package surface.
