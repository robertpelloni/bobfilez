# HANDOFF.md — bobfilez Session 53

## Current Status (2026-04-05)
**Version:** 6.0.38  
**Focus:** BTK upstream refresh — updated to latest upstream master, rebased required MSVC fixes, boundary unchanged

---

## What Was Done This Session

### 1. Refreshed BTK to the Latest Upstream Master
- Updated the `libs/btk` submodule against the newer upstream `origin/master` tip:
  - **`18e3770af`** — `build: validate BTK focus reason package smoke`
- Verified the local BTK branch still needed bobfilez's two MSVC-focused fixes on top of that newer upstream state.

### 2. Rebased the Required Local BTK Fixes Cleanly
- Rebased the two required BTK fixes onto the newer upstream master:
  - **`0546ebd70`** — `fix: restore msvc build for focus and input routing`
  - **`4f5a809e4`** — `fix: restore qapplication property lookups for msvc`
- Force-updated the reproducible pushed BTK branch carrying those rebased fixes:
  - **`origin/pi/msvc-focus-fixes-20260405`**

### 3. Re-Validated BTK Itself on the New Upstream State
- Re-ran **`scripts/build_btk_inplace.bat`** after the upstream refresh and rebase.
- Result:
  - BTK still configures and builds successfully on this host
  - the refreshed upstream state does not invalidate the earlier MSVC fix work

### 4. Re-Ran the Downstream bobfilez BTK Consumer Probe
- Re-ran **`scripts/build_btk_gui.bat`** against the refreshed BTK state.
- Result:
  - bobfilez still stops at the same honest downstream boundary:
    - missing BTK/CopperSpice component/target: **`Declarative`**
- This confirms the strategic conclusion is stable even on the newer upstream BTK snapshot.

### 5. Documented the Upstream Refresh Checkpoint
- Added **`docs/ai/implementation/BTK_UPSTREAM_REFRESH_2026_04_05.md`**.
- The document records:
  - the newer BTK upstream tip
  - the rebased local fixes
  - the successful BTK rebuild result
  - the unchanged downstream `Declarative` boundary

### 6. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.38**.
- Updated:
  - **`VERSION.md`**
  - **`core/include/fo/core/version.hpp`**
  - **`CHANGELOG.md`**
  - **`HANDOFF.md`**

---

### 7. BTK Upstream Refresh Checkpoint
- Refreshed `libs/btk` to the newer upstream `origin/master` tip:
  - **`18e3770af`** — `build: validate BTK focus reason package smoke`
- Rebased the two required local BTK MSVC fixes on top of that newer upstream state:
  - **`0546ebd70`** — `fix: restore msvc build for focus and input routing`
  - **`4f5a809e4`** — `fix: restore qapplication property lookups for msvc`
- Re-ran:
  - **`scripts/build_btk_inplace.bat`** — success
  - **`scripts/build_btk_gui.bat`** — unchanged downstream failure at missing `Declarative`
- This confirms the strategic conclusion is stable against the newer upstream BTK snapshot and is not just an artifact of an older provider revision.

### 8. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.38**.
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
| Option C execution status | 🟡 In progress | Round 7 extracted the active shell runtime bootstrap out of `main.cpp`, Round 8 separated bootstrap/runtime/registration responsibilities, Round 9 turned launch policy into explicit configuration, and Round 10 refined that into named launch profiles and runtime bundles. |
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
   - next likely target: introduce a small runtime/registration bundle registry or profile-selection helper so additional named launch packages can be added without growing bootstrap logic again

3. **Treat the refreshed BTK state as the current research baseline**
   - use the newer upstream BTK master plus the two rebased local MSVC fixes when probing BTK further
   - do not re-open already-resolved questions about whether the blocker was caused only by an older BTK snapshot

4. **Treat BTK modernization as a separate upstream R&D lane, not the near-term app path**
   - the missing top-level component entry is only the first symptom
   - the current `src/declarative` path also needs declarative-specific CMake modernization and a real QtScript/CsScript-era dependency story before it can honestly satisfy bobfilez's QML requirements
   - even after that, a forward-compatibility story from `QDeclarative*` toward bobfilez's current `QQml*` bootstrap would still need to be solved explicitly

4. **Continue provider-neutral GUI work**
   - keep prioritizing the work that remains valuable independent of BTK readiness, such as incremental dependency reduction, asset cleanup, and provider-boundary cleanup
