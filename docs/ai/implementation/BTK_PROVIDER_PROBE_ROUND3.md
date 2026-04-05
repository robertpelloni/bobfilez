# BTK Provider Probe Round 3 — BTK Builds on MSVC, bobfilez Now Blocked by Missing Declarative/QML Module

## Summary
This session moved the BTK investigation past the earlier build-break stage and into a more meaningful framework-capability boundary.

The key result is:
- **BTK itself now builds successfully on this MSVC host** after a focused set of upstream-compatibility fixes
- the expected BTK/CopperSpice libraries are now produced in `libs/btk/build-btk/lib`
  - including `CsCore2.1.lib`
  - `CsGui2.1.lib`
  - related binaries and plugins
- after that success, re-running bobfilez's BTK-backed GUI probe no longer fails on missing built libraries
- the new blocker is now that the active BTK package surface does **not export a `Declarative` target** required by bobfilez's QML-based GUI path

This is a much stronger and more honest boundary than the earlier ones. The problem is no longer:
- BTK package-layout breakage
- bad `_IMPORT_PREFIX` handling
- missing BTK library outputs due to an incomplete BTK build

It is now specifically that the currently built/exposed BTK framework surface does **not include the Declarative/QML module family** that bobfilez's native GUI expects.

## What changed

### 1. Fixed BTK's earlier MSVC build blockers
The BTK submodule was patched locally to address several incompatibilities that were preventing an in-place build from finishing.

#### A. Flag-type declaration mismatch
Files updated:
- `libs/btk/src/core/kernel/btkinputowner.h`
- `libs/btk/src/gui/widgets/btkfocusoverlay.h`
- `libs/btk/src/plugins/bearer/networkmanager/qnetworkmanagerservice.h`

Finding:
- BTK's current headers define `Q_DECLARE_OPERATORS_FOR_FLAGS`
- but the active framework surface does **not** provide the newer `Q_DECLARE_FLAGS` macro expected by some newer headers
- those headers therefore failed under MSVC where BTK tried to declare flag aliases like:
  - `Q_DECLARE_FLAGS(Capabilities, Capability)`

Fix:
- replaced those declarations with the older, BTK-compatible style:
  - `using Capabilities = QFlags<Capability>;`

Result:
- the original `btkinputowner.*` blocker was removed and the BTK build advanced significantly.

#### B. BTK string API drift in focus-diagnostics code
File updated:
- `libs/btk/src/gui/kernel/btkfocusdiagnostics.cpp`

Finding:
- the file mixed Qt-style string APIs (`.arg(...)`, `QStringLiteral`, some incompatible construction patterns) with BTK/CopperSpice string behavior
- that produced additional compile failures once the earlier flag issue was cleared

Fix:
- normalized the formatting logic to BTK-compatible string usage
- replaced `.arg(...)` patterns with `.formatArg(...)`
- adjusted string construction/join behavior accordingly

Result:
- the BTK build advanced into the next GUI/widget layer.

#### C. BTK string/property API drift in overlay/app code
Files updated:
- `libs/btk/src/gui/widgets/btkfocusoverlay.cpp`
- `libs/btk/src/gui/kernel/qapplication_cs.cpp`

Finding:
- these files still relied on several Qt-style assumptions that BTK's current API surface does not accept cleanly under MSVC:
  - `.arg(...)`
  - `QStringList::join('\n')` / character separators where BTK expects `QString`
  - property/setProperty usage with raw `const char *` names where the active API expects `QString`
  - invalid `QString::fromLatin1(...)` usage around values already represented as `QString`

Fix:
- converted formatting calls to `.formatArg(...)`
- changed join separators to `QString(...)`
- normalized property-name handling to `QString`
- removed invalid `fromLatin1(QString)` usage

Result:
- BTK's in-place build completed successfully on this host.

## Validation result: BTK now builds successfully
After the above fixes, `scripts/build_btk_inplace.bat` completed successfully.

Observed outputs include:
- `libs/btk/build-btk/lib/CsCore2.1.lib`
- `libs/btk/build-btk/lib/CsGui2.1.lib`
- `libs/btk/build-btk/lib/CsNetwork2.1.lib`
- `libs/btk/build-btk/lib/CsWebKit2.1.dll`
- Windows platform/plugin artifacts such as `CsGuiWin2.1.dll`

This closes the earlier blocker documented in Round 2 where bobfilez could not consume BTK because BTK had not produced its own binaries yet.

## Re-running bobfilez's BTK-backed GUI probe
After BTK finished building successfully, `scripts/build_btk_gui.bat` was run again.

Result:
- package discovery succeeds
- the prepared BTK build-tree package surface is found successfully
- bobfilez now fails during BTK target resolution with:
  - missing component/target: `Declarative`

The configure now stops at:
- `fo_resolve_btk_target(Declarative ...)`

## Why the new failure matters
This is not the same as the earlier package/export failures.

The new state means:
1. BTK can be built successfully on this host
2. BTK now exports/imports at least the core framework targets bobfilez expects for lower layers
3. the specific module bobfilez still needs for its QML-based GUI — `Declarative` — is not available from the current BTK build/package surface

That is a **framework capability mismatch**, not a mere path or packaging bug.

## Additional structural finding inside BTK
Inspection of the BTK source tree shows:
- there **is** a `src/declarative/CMakeLists.txt`
- it defines `CsDeclarative` / `CopperSpice::CsDeclarative`

However, the BTK top-level `CMakeLists.txt` currently sets:
- `CS_OPTIONAL_COMPONENTS` to:
  - `Gui Multimedia Network OpenGL Sql Svg Vulkan WebKit XmlPatterns`

Notably:
- `Declarative` is **not** included in that component list
- so the top-level build never adds `src/declarative`
- therefore no `CsDeclarative` target is built or exported

This explains the current bobfilez consumer failure directly.

## Important nuance
This is not yet proof that enabling `Declarative` in BTK would be trivial.

Reasons:
- `src/declarative/CMakeLists.txt` references `QtScript` / `CsScript` include/link surfaces
- the current BTK top-level component layout does not obviously build a corresponding Script module in this host configuration
- that suggests `Declarative` support may require additional upstream CMake/component work beyond simply adding one more item to a list

So the current evidence supports the following conclusion:
- **BTK is now build-correct enough to validate against**
- but **the active BTK component model still does not provide the QML/Declarative stack bobfilez's current native GUI requires**

## bobfilez-side improvement made during this probe
Updated:
- `cmake/BTKFrameworkSetup.cmake`

Improvement:
- `fo_resolve_btk_target(...)` now reports:
  - the candidate targets it checked
  - the imported BTK/CopperSpice targets that are actually available
  - a clearer note when the missing component is `Declarative`

This makes future probe failures much easier to interpret and reduces the chance of re-investigating solved packaging issues.

## Practical conclusion
The investigation has now advanced through three increasingly honest boundaries:

### Round 1
- BTK config/package discovery was incomplete
- missing companion package files blocked useful consumer probing

### Round 2
- build-tree package prep was improved
- missing BTK binaries became the real blocker

### Round 3
- BTK binaries now exist because BTK builds successfully
- the next blocker is that the current BTK build does **not expose Declarative/QML support** required by bobfilez's GUI

That means the next work is no longer simple build repair.
It is now one of these higher-level choices:
1. extend/fix BTK's top-level component model so Declarative/QML is actually built and exported
2. adapt bobfilez away from the Declarative/QML dependency
3. choose a different native framework path if QML-class functionality remains a hard requirement

## Recommended next steps
1. **Treat BTK MSVC build correctness as validated for the currently enabled modules**
   - the earlier `btkinputowner.*` blocker is resolved
   - `CsCore2.1.lib` and related outputs now exist

2. **Investigate BTK's missing Declarative component as an upstream framework issue**
   - determine whether `Declarative` was intentionally excluded
   - inspect what would be required to build/export `CsDeclarative`
   - especially verify the missing `CsScript` / QtScript-style dependency chain

3. **Do not misclassify the current bobfilez failure as a packaging bug**
   - package discovery is working much better now
   - the failure is specifically missing framework capability

4. **Keep bobfilez's active BTK probe honest**
   - the GUI should continue to fail clearly when `Declarative` is unavailable
   - avoid pretending BTK is a drop-in QML-capable provider until that module family actually exists in the built package surface
