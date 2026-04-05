# HANDOFF.md — bobfilez Session 60

## Current Status (2026-04-05)
**Version:** 6.0.45  
**Focus:** BobUI provider restore — BobUI/Qt6 is the active native path again, with current blockers honestly captured at missing `Qt6Qml` and an upstream BobUI corelib build failure on this host

---

## What Was Done This Session

### 1. Restored BobUI as the active native provider path
- Reintroduced BobUI-first native discovery for GUI / Omni builds.
- Restored:
  - `cmake/BobUIQtSetup.cmake`
  - `scripts/build_bobui_gui.bat`
  - `scripts/build_bobui_inplace.bat`
- Updated the root/native build wiring so bobfilez again prefers BobUI as a **Qt6 package provider** instead of treating BTK/CopperSpice as the active app path.

### 2. Restored BobUI omnicore wiring in the active GUI path
- Reintroduced BobUI `OmniUI/omnicore` source and include wiring for the native GUI targets.
- Restored active BobUI registration by calling:
  - `OmniUI::registerQmlTypes();`
- Preserved bobfilez-local registrations on top of that:
  - `Omni.File`
  - `Omni.Viz`
  - `Omni.Native`

### 3. Revalidated the BobUI-backed GUI probe honestly
Ran:
- `scripts/build_bobui_gui.bat`

Result:
- bobfilez now correctly discovers:
  - `libs/bobui/build-bobui/lib/cmake/Qt6/Qt6Config.cmake`
- configure still stops at the real downstream BobUI boundary on this machine:
  - missing `Qt6Qml`
- expected file still absent:
  - `libs/bobui/build-bobui/lib/cmake/Qt6Qml/Qt6QmlConfig.cmake`

### 4. Revalidated the BobUI in-place build helper honestly
Ran:
- `scripts/build_bobui_inplace.bat`

Result:
- the helper now provides a repeatable BobUI in-place developer-build probe
- a fresh in-place BobUI build on this host currently fails upstream in BobUI corelib while compiling `qlocale.cpp`
- current failure signature includes:
  - `qtmochelpers.h`
  - `'result': undeclared identifier`

### 5. Updated release/docs metadata to the BobUI-active truth
Updated:
- `VERSION.md`
- `core/include/fo/core/version.hpp`
- `CHANGELOG.md`
- `README.md`
- `DEPLOY.md`
- `AGENTS.md`
- `HANDOFF.md`
- `docs/ai/implementation/BOBUI_PROVIDER_RESTORE_2026_04_05.md`

### 6. Added a session handoff log
Added:
- `logs/handoffs/2026-04-05-session-60.md`

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Active native provider direction | 🟢 Settled for now | BobUI/Qt6 is the active app-side provider path again. BTK remains research/background, not the current app target. |
| BobUI-backed GUI configure | 🔴 Blocked on current local BobUI export surface | bobfilez reaches BobUI's `Qt6Config.cmake`, but configure still fails because `Qt6Qml` is not exported in the current local BobUI build tree. |
| BobUI in-place build on this host | 🔴 Upstream/provider build failure | A fresh BobUI in-place build currently stops in BobUI corelib (`qtmochelpers.h` / `qlocale.cpp`) before becoming a complete provider surface. |
| Headless path | 🟢 Stable | Headless build/test workflow remains the reliable validation lane. |
| BTK research baseline | 🟡 Preserved but not active | Prior BTK findings remain valuable documentation, but BTK is no longer the active runtime direction. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Keep BobUI as the active native strategy**
   - avoid re-flipping provider direction again unless there is a concrete new reason
   - keep docs and helper scripts aligned to BobUI-first reality

2. **Treat the current BobUI provider problem as two-layered**
   - downstream: current exposed BobUI build tree lacks `Qt6Qml`
   - upstream: fresh BobUI in-place build currently fails in `qtmochelpers.h` / `qlocale.cpp`

3. **Investigate the BobUI corelib build failure next**
   - inspect the current `qtmochelpers.h` / generated metaobject path on this host
   - determine whether this is an MSVC-specific regression, a configuration mismatch, or a stale generated-source issue

4. **Only after BobUI builds cleanly, re-probe required Qt6 components**
   - confirm whether a successful BobUI build exports:
     - `Qt6Qml`
     - `Qt6Quick`
     - `Qt6QuickControls2`
   - if not, decide whether bobfilez must reduce dependency surface further or BobUI must expose more modules

5. **Continue provider-neutral shell cleanup only where it still pays off**
   - launch-profile selection/listing is already in a good place
   - keep further GUI work aligned to real BobUI viability rather than abstract provider churn

---

## Validation Snapshot

### Confirmed
- BobUI is restored as the active native provider path in project wiring.
- `OmniUI::registerQmlTypes()` is restored in `gui/omni/src/OmniQmlRegistration.cpp`.
- `scripts/build_bobui_gui.bat` now reaches BobUI's exported `Qt6Config.cmake`.
- Current BobUI GUI configure blocker is still missing `Qt6Qml`.
- `scripts/build_bobui_inplace.bat` now provides a repeatable in-place probe and currently fails upstream in BobUI corelib.

### Not Yet Resolved
- A complete BobUI build/install prefix that exports the QML/Quick modules bobfilez needs.
- A successful native GUI / Omni configure/build on this machine using BobUI.

---

## Handoff Summary
This session cleanly restored **BobUI/Qt6** as the active native runtime direction and documented the result honestly. The app-side provider path is now back where the user wanted it, but the current host still cannot complete native GUI startup because the local BobUI state is incomplete in two ways: the exposed build tree lacks `Qt6Qml`, and a fresh BobUI in-place build currently fails in BobUI corelib (`qtmochelpers.h` / `qlocale.cpp`). The next real work should stay focused on BobUI provider readiness rather than reopening BTK as the active path.
