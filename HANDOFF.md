# HANDOFF.md — bobfilez Session 56

## Current Status (2026-04-05)
**Version:** 6.0.41  
**Focus:** Native UI profile CLI selection — seventh concrete Option C refinement with CLI+env-driven named profile choice

---

## What Was Done This Session

### 1. Completed the BTK Upstream Refresh Checkpoint
- Updated the `libs/btk` submodule against the newer upstream `origin/master` tip:
  - **`18e3770af`** — `build: validate BTK focus reason package smoke`
- Rebased bobfilez's two required local BTK MSVC fixes on top of that newer upstream state:
  - **`0546ebd70`** — `fix: restore msvc build for focus and input routing`
  - **`4f5a809e4`** — `fix: restore qapplication property lookups for msvc`
- Force-updated the reproducible pushed BTK branch carrying those rebased fixes:
  - **`origin/pi/msvc-focus-fixes-20260405`**

### 2. Re-Validated BTK and the Downstream Consumer Boundary
- Re-ran **`scripts/build_btk_inplace.bat`** after the upstream refresh and rebase.
- Result:
  - BTK still configures and builds successfully on this host
- Re-ran **`scripts/build_btk_gui.bat`** against the refreshed BTK state.
- Result:
  - bobfilez still stops at the same honest downstream boundary:
    - missing BTK/CopperSpice component/target: **`Declarative`**
- This confirms the strategic conclusion is stable even on the newer upstream BTK snapshot and is not just an artifact of an older provider revision.

### 3. Executed the Seventh Real Option C Refinement in Code
- Extended the launch-profile selection seam so profile choice can now come from the command line as well as the environment.
- Updated:
  - **`gui/omni/src/NativeUiProfileRegistry.hpp`**
  - **`gui/omni/src/NativeUiProfileRegistry.cpp`**
  - **`gui/omni/src/NativeUiBootstrap.cpp`**
- Added registry helpers for:
  - `native_ui_profile_argument_prefix()`
  - `selected_launch_profile_name(int argc, char *argv[])`
  - `create_launch_profile_from_selection(int argc, char *argv[])`
- The active CLI selector is now:
  - **`--native-ui-profile=<name>`**
- Selection precedence is now explicit:
  1. command-line override
  2. environment variable override via **`BOBFILEZ_NATIVE_UI_PROFILE`**
  3. default launch profile
- Unknown profile names still warn and fall back to the default profile.

### 4. Preserved Default Behavior While Adding a Real Operational Override Path
- The active shell still launches the same way by default.
- No runtime/provider claim changed.
- This only adds a clean, registry-backed way to exercise alternate named launch profiles later without modifying bootstrap code, and without requiring environment changes for one-off runs.

### 5. Documented the New CLI+Env Selection Path
- Added:
  - **`docs/ai/implementation/BTK_UPSTREAM_REFRESH_2026_04_05.md`**
  - **`docs/ai/implementation/NATIVE_UI_PROFILE_REGISTRY.md`**
  - **`docs/ai/implementation/NATIVE_UI_PROFILE_ENV_SELECTION.md`**
  - **`docs/ai/implementation/NATIVE_UI_PROFILE_CLI_SELECTION.md`**
- The new CLI-selection doc records:
  - the active `--native-ui-profile=<name>` selector
  - the precedence order
  - the fallback behavior
  - the unchanged default runtime behavior

### 6. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.41**.
- Updated:
  - **`VERSION.md`**
  - **`core/include/fo/core/version.hpp`**
  - **`CHANGELOG.md`**
  - **`HANDOFF.md`**
  - **`AGENTS.md`**

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BTK-backed GUI / Omni build | 🔴 Still blocked at a strategic compatibility boundary | BTK now builds successfully for its normal enabled module set, but bobfilez's active native GUI bootstrap still cannot configure because `Declarative` is not exported and the framework path no longer looks like a same-generation QML provider. |
| BTK Declarative/QML support | 🔴 Deeper upstream/provider readiness gap confirmed | Round 4 proved the issue is broader than a missing component-list entry: when `Declarative` is experimentally re-enabled, the module immediately hits stale declarative-specific CMake integration, obsolete metatype declarations, and fatal missing QtScript-era headers such as `QtScript/qscriptvalue.h`. |
| BTK vs bobfilez declarative API generation | 🔴 Direct mismatch confirmed | Round 5 proved BTK's current declarative surface is `QDeclarative*`-era and exposes no discovered `QQml*` / `QQuick*` provider surface, while bobfilez's active bootstrap is explicitly `QQmlApplicationEngine`-based. |
| Recommended project strategy | 🟢 Decision documented | Round 6 formalized the least-destructive path: keep bobfilez on a modern QQml-style shell path, reduce provider coupling, and treat BTK modernization as a separate upstream R&D effort rather than the immediate app runtime target. |
| Option C execution status | 🟡 In progress | Round 7 extracted the active shell runtime bootstrap out of `main.cpp`, Round 8 separated bootstrap/runtime/registration responsibilities, Round 9 turned launch policy into explicit configuration, Round 10 refined that into named launch profiles and runtime bundles, Round 11 added a small profile-registry/helper layer, Round 12 added an env-driven selection seam for named launch profiles, and Round 13 added CLI-aware profile selection with explicit CLI > env > default precedence. |
| BTK upstream refresh status | 🟢 Revalidated | Latest upstream BTK master plus the two rebased MSVC fixes still builds successfully here, and bobfilez still stops at the same missing-`Declarative` boundary. |
| BTK native migration plan | 🟡 In progress | Active BobUI-specific provider/bootstrap assumptions remain removed from bobfilez, but the remaining blocker is now whether BTK can provide the QML/Declarative layer bobfilez still depends on. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Treat BTK MSVC build correctness as validated for the currently enabled BTK modules**
   - the old `btkinputowner.*` blocker is resolved
   - `CsCore2.1.lib` and related outputs now exist

2. **Continue executing Option C incrementally in code**
   - keep bobfilez aligned to a modern QQml-style shell path
   - continue reducing provider assumptions where practical
   - next likely target: add one additional named profile or bundle only if it represents a real alternate launch policy worth exercising, now that both env-driven and CLI-driven selection seams exist

3. **Treat the refreshed BTK state as the current research baseline**
   - use the newer upstream BTK master plus the two rebased local MSVC fixes when probing BTK further
   - do not re-open already-resolved questions about whether the blocker was caused only by an older BTK snapshot

4. **Treat BTK modernization as a separate upstream R&D lane, not the near-term app path**
   - the missing top-level component entry is only the first symptom
   - the current `src/declarative` path also needs declarative-specific CMake modernization and a real QtScript/CsScript-era dependency story before it can honestly satisfy bobfilez's QML requirements
   - even after that, a forward-compatibility story from `QDeclarative*` toward bobfilez's current `QQml*` bootstrap would still need to be solved explicitly

5. **Continue provider-neutral GUI work**
   - keep prioritizing the work that remains valuable independent of BTK readiness, such as incremental dependency reduction, asset cleanup, and provider-boundary cleanup
