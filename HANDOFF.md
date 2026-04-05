# HANDOFF.md — bobfilez Session 45

## Current Status (2026-04-05)
**Version:** 6.0.30  
**Focus:** BTK native provider probe round 3 — BTK MSVC build fixed, Declarative/QML capability now the blocker

---

## What Was Done This Session

### 1. Repaired BTK's Earlier MSVC Build Failures
- Patched the active `libs/btk` submodule to remove the earlier BTK-side blockers that prevented a complete in-place build.
- Replaced unsupported `Q_DECLARE_FLAGS(...)` usage with BTK-compatible `using ... = QFlags<...>;` aliases in:
  - **`libs/btk/src/core/kernel/btkinputowner.h`**
  - **`libs/btk/src/gui/widgets/btkfocusoverlay.h`**
  - **`libs/btk/src/plugins/bearer/networkmanager/qnetworkmanagerservice.h`**
- Normalized BTK/CopperSpice string and property API usage in:
  - **`libs/btk/src/gui/kernel/btkfocusdiagnostics.cpp`**
  - **`libs/btk/src/gui/kernel/qapplication_cs.cpp`**
  - **`libs/btk/src/gui/widgets/btkfocusoverlay.cpp`**

### 2. Verified BTK Now Builds Successfully on This Host
- Re-ran **`scripts/build_btk_inplace.bat`** after the BTK-side fixes.
- Result:
  - BTK now completes its in-place MSVC build successfully
  - expected outputs now exist under `libs/btk/build-btk/lib` / `bin`, including:
    - `CsCore2.1.lib`
    - `CsGui2.1.lib`
    - `CsNetwork2.1.lib`
    - plugin/runtime artifacts such as `CsGuiWin2.1.dll`
- This closes the earlier Round 2 blocker where bobfilez still failed only because BTK had not produced its own libraries yet.

### 3. Re-Ran the BTK-Backed bobfilez GUI Probe
- Executed **`scripts/build_btk_gui.bat`** again after the successful BTK build.
- Result:
  - bobfilez no longer fails on missing `CsCore2.1.lib`
  - the active configure failure is now:
    - missing BTK/CopperSpice component/target: **`Declarative`**
- The failure now occurs during `fo_resolve_btk_target(Declarative ...)`, which is a much more honest consumer boundary than earlier package-layout or missing-binary failures.

### 4. Confirmed the Missing Declarative/QML Capability is Structural
- Inspected the BTK source tree and found:
  - **`libs/btk/src/declarative/CMakeLists.txt`** exists and defines `CsDeclarative`
- Inspected BTK's top-level **`CMakeLists.txt`** and confirmed:
  - `CS_OPTIONAL_COMPONENTS` currently includes:
    - `Gui Multimedia Network OpenGL Sql Svg Vulkan WebKit XmlPatterns`
  - **`Declarative` is not included**
- Therefore:
  - the BTK top-level build never adds `src/declarative`
  - no `CsDeclarative` / `CopperSpice::CsDeclarative` target is built or exported
  - bobfilez's QML-based GUI cannot currently resolve the framework module it needs

### 5. Improved bobfilez-Side Probe Diagnostics
- Updated **`cmake/BTKFrameworkSetup.cmake`**.
- `fo_resolve_btk_target(...)` now reports:
  - which candidate targets were checked
  - which BTK/CopperSpice imported targets are actually available
  - a clearer hint when the missing component is `Declarative`
- This makes future BTK probe failures much easier to interpret and avoids re-investigating already solved package-layout issues.

### 6. Documented the Probe Round 3 Findings
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND3.md`**.
- The document records:
  - the BTK-side MSVC build fixes
  - the successful BTK in-place build result
  - the new missing-`Declarative` boundary
  - the conclusion that the remaining issue is now framework capability, not missing library outputs

### 7. Release / Metadata Alignment
- Reconciled release/docs metadata to **6.0.30**.
- Updated:
  - **`VERSION.md`**
  - **`core/include/fo/core/version.hpp`**
  - **`CHANGELOG.md`**
  - **`HANDOFF.md`**

---

## Current Risks / Gaps

| Area | Status | Notes |
|------|--------|-------|
| Full BTK-backed GUI / Omni build | 🟡 Still blocked | BTK now builds successfully and produces real framework binaries on this host, but the current BTK package surface still does not export `Declarative`, so bobfilez's QML-based native GUI cannot yet configure against it. |
| BTK Declarative/QML support | 🟡 Unresolved upstream/provider issue | `src/declarative` exists in BTK, but the top-level BTK component list does not currently build/export it. The `src/declarative` module also appears to assume additional `QtScript` / `CsScript` style dependencies, so enabling it may require more than a one-line component-list change. |
| BTK native migration plan | 🟡 In progress | Active BobUI-specific provider/bootstrap assumptions remain removed from bobfilez, but the remaining blocker is now whether BTK can provide the QML/Declarative layer bobfilez still depends on. |
| Dirty submodules/worktrees | 🟡 Pending | Existing unrelated dirty submodules remain intentionally unstaged. |

---

## Recommended Next Steps

1. **Treat BTK MSVC build correctness as validated for the currently enabled BTK modules**
   - the old `btkinputowner.*` blocker is resolved
   - `CsCore2.1.lib` and related outputs now exist

2. **Investigate BTK's missing Declarative component as the next real upstream task**
   - determine whether `Declarative` was intentionally excluded from `CS_OPTIONAL_COMPONENTS`
   - verify what is needed to build/export `CsDeclarative`
   - inspect the apparent `CsScript` / QtScript-style dependency chain before assuming a trivial enablement

3. **Keep the bobfilez BTK consumer path honest**
   - do not pretend BTK is a full QML-capable provider until `Declarative` actually exists in the built package surface

4. **Resume additional GUI dependency-reduction only if BTK investigation pauses**
   - if the framework-path investigation stalls, the next productive lane is still incremental `QtQuick.Controls` reduction in richer routed panels
