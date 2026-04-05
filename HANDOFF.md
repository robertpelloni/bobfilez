# HANDOFF.md — bobfilez Session 58

## Current Status (2026-04-05)
**Version:** 6.0.43  
**Focus:** Explorer-only native launch profile — second genuinely alternate named profile on the Option C launch architecture

---

## What Was Done This Session

### 1. Added the Second Genuinely Alternate Named Launch Profile
- Added a second real alternate native shell launch profile:
  - **`omni-explorer-only`**
- This is not a fake alias for the full shell. It launches a different root QML surface:
  - default profile → `qrc:/main.qml`
  - dashboard-only profile → `qrc:/DashboardShell.qml`
  - explorer-only profile → `qrc:/ExplorerShell.qml`

### 2. Added a Dedicated Explorer-Only Root Window
- Added:
  - **`gui/omni/assets/ExplorerShell.qml`**
- The new root window provides a focused explorer-only launch mode built around the existing `ExplorerWindow` surface.
- It includes a local `FileModel` and a minimal local `shell` shim so the explorer surface can function without the full shell host.
- Updated:
  - **`gui/omni/assets/qml.qrc`**
- Added `ExplorerShell.qml` to the packaged QML resources so it is available as a real alternate root.

### 3. Wired the Alternate Profile into the Launch-Profile Architecture
- Updated:
  - **`gui/omni/src/NativeUiLaunchProfile.hpp`**
  - **`gui/omni/src/NativeUiLaunchProfile.cpp`**
  - **`gui/omni/src/NativeUiProfileRegistry.cpp`**
- Added:
  - `create_explorer_only_launch_profile()`
- Registered the new profile name:
  - **`omni-explorer-only`**
- This means the existing selection seams can now exercise a second genuinely different launch mode instead of only the full shell and dashboard-only roots.

### 4. Preserved Default Behavior While Expanding the Real Profile Set
- The default shell launch path remains unchanged.
- No runtime/provider claim changed.
- This change proves the launch-profile architecture can support more than one meaningful alternate mode without requiring bootstrap rewrites.

### 5. Documented the Explorer-Only Launch Mode
- Added:
  - **`docs/ai/implementation/NATIVE_UI_EXPLORER_PROFILE.md`**
  - **`logs/handoffs/2026-04-05-session-57.md`**
- The new profile doc records:
  - the explorer-only root QML path
  - the local shell shim
  - how to select the profile via CLI or environment
  - why it is a meaningful alternate mode rather than an alias

### 6. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.43**.
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
| Option C execution status | 🟡 In progress | Round 7 extracted the active shell runtime bootstrap out of `main.cpp`, Round 8 separated bootstrap/runtime/registration responsibilities, Round 9 turned launch policy into explicit configuration, Round 10 refined that into named launch profiles and runtime bundles, Round 11 added a small profile-registry/helper layer, Round 12 added an env-driven selection seam for named launch profiles, Round 13 added CLI-aware profile selection with explicit CLI > env > default precedence, Round 14 added the first genuinely alternate named launch profile (`omni-dashboard-only`), and Round 15 added the second real alternate profile (`omni-explorer-only`). |
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
   - next likely target: only add another profile or runtime bundle if it represents a genuinely different launch workflow beyond the full shell, dashboard-only, and explorer-only modes already present

3. **Treat the refreshed BTK state as the current research baseline**
   - use the newer upstream BTK master plus the two rebased local MSVC fixes when probing BTK further
   - do not re-open already-resolved questions about whether the blocker was caused only by an older BTK snapshot

4. **Treat BTK modernization as a separate upstream R&D lane, not the near-term app path**
   - the missing top-level component entry is only the first symptom
   - the current `src/declarative` path also needs declarative-specific CMake modernization and a real QtScript/CsScript-era dependency story before it can honestly satisfy bobfilez's QML requirements
   - even after that, a forward-compatibility story from `QDeclarative*` toward bobfilez's current `QQml*` bootstrap would still need to be solved explicitly

5. **Continue provider-neutral GUI work**
   - keep prioritizing the work that remains valuable independent of BTK readiness, such as incremental dependency reduction, asset cleanup, and provider-boundary cleanup
