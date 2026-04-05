# Changelog

## [6.0.44] - 2026-04-05

### Added — The "Native UI Profile Listing" Release

#### 🧩 Eighth Concrete Option C Refinement Added a User-Facing Discovery Affordance
- Added a dedicated CLI flag to list available native launch profiles without starting the GUI runtime:
  - `--list-native-ui-profiles`
- Updated:
  - `gui/omni/src/NativeUiProfileRegistry.hpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
  - `gui/omni/src/NativeUiBootstrap.cpp`
- Added registry helpers for:
  - `native_ui_profile_list_argument()`
  - `should_list_launch_profiles(int argc, char *argv[])`
- When invoked, the native entrypoint now:
  - prints the available launch profile names
  - marks the default profile
  - prints the active selection precedence rule
  - exits without attempting to launch the GUI runtime
- This makes the launch-profile architecture externally discoverable rather than only visible in code/docs.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_PROFILE_LISTING.md`** documenting:
  - the new CLI flag
  - the listing behavior
  - why it improves launch-profile usability
  - the unchanged runtime behavior when the flag is absent

### Version
- Bumped to **6.0.44**.

## [6.0.43] - 2026-04-05

### Added — The "Explorer-Only Native Launch Profile" Release

#### 🧩 Second Genuinely Alternate Named Launch Profile Added
- Added a second real alternate native shell launch profile:
  - **`omni-explorer-only`**
- This profile is backed by a different root QML surface:
  - `qrc:/ExplorerShell.qml`
- Added:
  - `gui/omni/assets/ExplorerShell.qml`
- Updated:
  - `gui/omni/assets/qml.qrc`
  - `gui/omni/src/NativeUiLaunchProfile.hpp`
  - `gui/omni/src/NativeUiLaunchProfile.cpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
- Added `create_explorer_only_launch_profile()` and registered the profile name in the launch-profile registry.
- The explorer-only profile can be selected through the existing selection seams:
  - CLI: `--native-ui-profile=omni-explorer-only`
  - env: `BOBFILEZ_NATIVE_UI_PROFILE=omni-explorer-only`

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_EXPLORER_PROFILE.md`** documenting:
  - the explorer-only root QML path
  - the local shell shim used by the wrapper root
  - how to select the profile via CLI or environment
  - why it is a meaningful alternate mode rather than an alias

### Version
- Bumped to **6.0.43**.

## [6.0.42] - 2026-04-05

### Added — The "Dashboard-Only Native Launch Profile" Release

#### 🧩 First Genuinely Alternate Named Launch Profile Added
- Added the first real alternate native shell launch profile:
  - **`omni-dashboard-only`**
- This is not a fake alias for the existing shell root. The new profile launches a different root QML surface:
  - default profile → `qrc:/main.qml`
  - dashboard-only profile → `qrc:/DashboardShell.qml`
- Added:
  - `gui/omni/assets/DashboardShell.qml`
- Updated:
  - `gui/omni/assets/qml.qrc`
  - `gui/omni/src/NativeUiLaunchProfile.hpp`
  - `gui/omni/src/NativeUiLaunchProfile.cpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
- Added `create_dashboard_only_launch_profile()` and registered the profile name in the launch-profile registry.
- The new profile can be selected through the already-added selection seams:
  - CLI: `--native-ui-profile=omni-dashboard-only`
  - env: `BOBFILEZ_NATIVE_UI_PROFILE=omni-dashboard-only`

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_DASHBOARD_PROFILE.md`** documenting:
  - why this profile is a real alternate launch mode
  - the new root QML path
  - how to select the profile via CLI or environment
  - why the dashboard-only surface is a good first alternate profile

### Version
- Bumped to **6.0.42**.

## [6.0.41] - 2026-04-05

### Refactored — The "Native UI Profile CLI Selection" Release

#### 🧩 Seventh Concrete Option C Refinement Landed in Code
- Extended the native shell launch-profile selection seam so profile choice can be made from the command line as well as the environment.
- Updated:
  - `gui/omni/src/NativeUiProfileRegistry.hpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
  - `gui/omni/src/NativeUiBootstrap.cpp`
- Added registry helpers for:
  - `native_ui_profile_argument_prefix()`
  - `selected_launch_profile_name(int argc, char *argv[])`
  - `create_launch_profile_from_selection(int argc, char *argv[])`
- The active CLI selector is now:
  - `--native-ui-profile=<name>`
- Selection precedence is now explicit:
  1. command-line override
  2. environment variable override (`BOBFILEZ_NATIVE_UI_PROFILE`)
  3. default launch profile
- Unknown profile names still warn and fall back to the default profile.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_PROFILE_CLI_SELECTION.md`** documenting:
  - the new CLI selector
  - the precedence order
  - the unchanged default behavior
  - the limitations that remain unchanged

### Version
- Bumped to **6.0.41**.

## [6.0.40] - 2026-04-05

### Refactored — The "Native UI Profile Environment Selection" Release

#### 🧩 Sixth Concrete Option C Refinement Landed in Code
- Added a tiny registry-backed environment selection seam for named native shell launch profiles.
- Updated:
  - `gui/omni/src/NativeUiProfileRegistry.hpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
  - `gui/omni/src/NativeUiBootstrap.cpp`
- Added registry helpers for:
  - `native_ui_profile_environment_variable()`
  - `selected_launch_profile_name()`
  - `create_launch_profile_from_environment()`
- The active selection variable is now:
  - `BOBFILEZ_NATIVE_UI_PROFILE`
- Behavior:
  - no env var → use the default launch profile
  - known profile name → use that named profile
  - unknown profile name → emit a warning and fall back to the default profile
- This keeps bootstrap logic small while giving future alternate launch profiles a real, explicit selection hook.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_PROFILE_ENV_SELECTION.md`** documenting:
  - the new environment-driven selection path
  - the fallback behavior
  - the unchanged default runtime behavior
  - the limitations that remain unchanged

### Version
- Bumped to **6.0.40**.

## [6.0.39] - 2026-04-05

### Refactored — The "Native UI Profile Registry" Release

#### 🧩 Fifth Concrete Option C Refinement Landed in Code
- Added a small lookup/helper layer for named launch profiles and runtime bundles:
  - `gui/omni/src/NativeUiProfileRegistry.hpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
- Updated the active bootstrap so it no longer directly constructs the default profile factory result itself.
- `gui/omni/src/NativeUiBootstrap.cpp` now asks the registry for:
  - the default launch profile name
  - the corresponding launch profile by name
- This keeps bootstrap logic focused on orchestration rather than policy lookup as the number of named launch packages grows.

#### 🧹 GUI Build Wiring Updated
- Updated:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
- Added the new profile-registry files to the GUI source lists so the selection seam is represented honestly in both native GUI targets.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_PROFILE_REGISTRY.md`** documenting:
  - the new registry/helper surface
  - the updated bootstrap flow
  - why this improves Option C without changing runtime behavior
  - the limitations that remain unchanged

### Version
- Bumped to **6.0.39**.

## [6.0.38] - 2026-04-05

### Validated — The "BTK Upstream Refresh" Release

#### 🔄 BTK Updated to Latest Upstream Master and Rebased Cleanly
- Refreshed `libs/btk` to the newer upstream `origin/master` tip:
  - `18e3770af` — `build: validate BTK focus reason package smoke`
- Rebased bobfilez's two required MSVC fixes on top of that newer BTK tip:
  - `0546ebd70` — `fix: restore msvc build for focus and input routing`
  - `4f5a809e4` — `fix: restore qapplication property lookups for msvc`
- Updated the reproducible pushed BTK branch carrying those rebased fixes:
  - `origin/pi/msvc-focus-fixes-20260405`

#### ✅ BTK Build Result Still Good
- Re-ran `scripts/build_btk_inplace.bat` after the upstream refresh and rebase.
- Confirmed BTK still configures and builds successfully on this host with the rebased MSVC fixes applied.

#### 🔍 Downstream bobfilez Boundary Unchanged
- Re-ran `scripts/build_btk_gui.bat` against the refreshed BTK state.
- bobfilez still stops at the same honest compatibility boundary:
  - missing BTK/CopperSpice component `Declarative`
- This confirms the strategic conclusion did not depend on an older BTK snapshot; the same blocker persists on the newer upstream master.

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_UPSTREAM_REFRESH_2026_04_05.md`** documenting:
  - the newer upstream BTK tip
  - the rebased local MSVC fixes
  - the successful BTK rebuild
  - the unchanged downstream `Declarative` boundary

### Version
- Bumped to **6.0.38**.

## [6.0.37] - 2026-04-05

### Refactored — The "Native UI Launch Profiles and Runtime Bundles" Release

#### 🧩 Fourth Concrete Option C Extraction Step Landed in Code
- Evolved the prior launch-config extraction into a clearer named-policy model.
- Added:
  - `gui/omni/src/NativeUiRuntimeBundle.hpp`
  - `gui/omni/src/NativeUiRuntimeBundle.cpp`
  - `gui/omni/src/NativeUiLaunchProfile.hpp`
  - `gui/omni/src/NativeUiLaunchProfile.cpp`
- Replaced the previous raw launch-config callback bag in the active bootstrap flow with:
  - a **runtime bundle** (runtime factory + registration policy)
  - a **launch profile** (shell root QML + failure policy + chosen bundle)
- Refactored `gui/omni/src/NativeUiBootstrap.cpp` so it now reads as explicit launch-profile selection rather than ad hoc callback assembly.

#### 🧹 GUI Build Wiring Updated
- Updated:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
- Replaced the active launch-config files in the GUI source lists with the new launch-profile/runtime-bundle files.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_LAUNCH_PROFILES_AND_BUNDLES.md`** documenting:
  - the difference between runtime bundles and launch profiles
  - the new bootstrap flow
  - the preserved behavior
  - the remaining limitations

### Version
- Bumped to **6.0.37**.

## [6.0.36] - 2026-04-05

### Refactored — The "Native UI Launch Config" Release

#### 🧩 Third Concrete Option C Extraction Step Landed in Code
- Continued reducing provider coupling by turning active shell launch policy into explicit configuration data.
- Added:
  - `gui/omni/src/NativeUiLaunchConfig.hpp`
  - `gui/omni/src/NativeUiLaunchConfig.cpp`
- Refactored `gui/omni/src/NativeUiBootstrap.cpp` so it no longer hardcodes:
  - the main QML URL
  - the registration bundle choice
  - the default runtime factory choice
  - the root-object failure policy
- Introduced `NativeUiLaunchConfig` with explicit fields for:
  - `main_qml`
  - `register_types`
  - `runtime_factory`
  - `object_created_handler`
- Added `create_default_omni_ui_launch_config()` so the current shell launch policy is centralized rather than being implicit bootstrap behavior.

#### 🧹 GUI Build Wiring Updated
- Updated:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
- Added the new launch-config files to the GUI source lists so the extraction is represented honestly in both native GUI targets.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_LAUNCH_CONFIG.md`** documenting:
  - the remaining implicit launch policy after earlier extractions
  - the new configuration object
  - the bootstrap flow after the refactor
  - the behavior preserved and the limitations unchanged

### Version
- Bumped to **6.0.36**.

## [6.0.35] - 2026-04-05

### Refactored — The "Native UI Runtime Policy Extraction" Release

#### 🧩 Second Concrete Option C Extraction Step Landed in Code
- Continued reducing direct provider coupling in the active shell bootstrap by separating three concerns that had still been mixed inside `NativeUiBootstrap.cpp`:
  - bootstrap orchestration
  - declarative runtime construction/adapter selection
  - QML type-registration policy
- Added:
  - `gui/omni/src/NativeUiRuntime.hpp`
  - `gui/omni/src/NativeUiRuntime.cpp`
  - `gui/omni/src/OmniQmlRegistration.hpp`
  - `gui/omni/src/OmniQmlRegistration.cpp`
- Refactored `gui/omni/src/NativeUiBootstrap.cpp` so it now primarily orchestrates:
  - `QGuiApplication` creation
  - invocation of registration policy
  - default runtime creation via the runtime factory
  - root-object failure handling
  - loading `qrc:/main.qml`
- This keeps the active runtime behavior the same while making runtime implementation and registration policy independently swappable later.

#### 🧹 GUI Build Wiring Updated
- Updated:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
- Added the new runtime/registration files to the GUI source lists so the extraction is reflected honestly in both native GUI targets.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_RUNTIME_POLICY_EXTRACTION.md`** documenting:
  - the remaining coupling after the first bootstrap seam
  - the new responsibility split
  - the behavior preserved
  - what this extraction does and does not solve

### Version
- Bumped to **6.0.35**.

## [6.0.34] - 2026-04-05

### Refactored — The "Native UI Bootstrap Seam" Release

#### 🧩 First Concrete Option C Execution Step Landed in Code
- Added a provider-neutral bootstrap seam for the active native shell entrypoint:
  - `gui/omni/src/NativeUiBootstrap.hpp`
  - `gui/omni/src/NativeUiBootstrap.cpp`
- Refactored `gui/omni/src/main.cpp` into a minimal launcher that now delegates to `fo::gui::run_omni_shell(...)` instead of directly hardcoding the active declarative runtime path.
- Moved current runtime-specific responsibilities into the bootstrap layer, including:
  - `QGuiApplication` creation
  - active declarative-engine creation/loading
  - QML type registration for `FileModel`, `TreemapModel`, and `NativeMarkdownView`
  - root-object failure handling for `qrc:/main.qml`
- Introduced a narrow internal runtime interface so future provider/runtime work can target a smaller seam instead of rewriting the top-level entrypoint again.

#### 🧹 GUI Source List Alignment
- Updated:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
- Ensured the GUI source lists include the new bootstrap files.
- Also aligned `gui/CMakeLists.txt` to include `TreemapModel.cpp/.h`, keeping the source list coherent with the shell's QML type registration path.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_BOOTSTRAP_SEAM.md`** documenting:
  - the rationale for extracting the seam
  - the exact behavior preserved
  - what this refactor does and does not solve
  - how it fits into Option C from the BTK framework decision matrix

### Version
- Bumped to **6.0.34**.

## [6.0.33] - 2026-04-05

### Documented — The "BTK Framework Decision Matrix" Release

#### 🧭 Strategy Converted from Probe History into an Actionable Architecture Decision
- Added **`docs/ai/design/BTK_FRAMEWORK_DECISION_MATRIX.md`** to turn the BTK probe series into a decision-ready architecture artifact.
- The matrix compares four realistic paths:
  - upstream BTK modernization toward a `QQml*` / `QQuick*`-class provider
  - porting bobfilez backward to a `QDeclarative*` era stack
  - keeping bobfilez on a modern QQml-style shell path while decoupling provider assumptions
  - replacing the QML shell architecture entirely
- Included a scored comparison across preservation of current shell work, delivery risk, upstream burden, regression risk, and strategic optionality.
- The documented recommendation is now explicit:
  - **primary app strategy:** keep bobfilez on a modern QQml-style shell path and decouple provider assumptions
  - **optional parallel R&D strategy:** treat BTK modernization as a separate upstream framework effort, not as an immediate bobfilez integration unblock

#### 📊 Architectural Conclusion Sharpened Again
- Round 5's findings are now translated into a concrete project decision:
  - BTK is currently blocked not just by missing package exports and stale declarative internals, but also by a direct `QDeclarative*` vs `QQml*` generation mismatch
  - therefore the least-destructive near-term path is to stop treating BTK as the immediate native runtime for bobfilez's active shell bootstrap

#### 📘 Documentation
- Added **`docs/ai/design/BTK_FRAMEWORK_DECISION_MATRIX.md`** including:
  - a scored option matrix
  - a phased plan
  - a mermaid decision flow
  - explicit non-recommendations

### Version
- Bumped to **6.0.33**.

## [6.0.32] - 2026-04-05

### Validated — The "BTK Native Provider Probe Round 5" Release

#### 🧭 Architectural Compatibility Boundary Sharpened Further
- Confirmed the current blocker is not just missing `Declarative` exposure and not just stale declarative internals.
- Verified that bobfilez's active native GUI bootstrap in `gui/omni/src/main.cpp` is explicitly **QQml-style**, using:
  - `QGuiApplication`
  - `QQmlApplicationEngine`
  - `QQmlContext`
  - `qmlRegisterType(...)`
- Verified that BTK's declarative surface is still explicitly **QDeclarative-era**, centered on classes such as:
  - `QDeclarativeEngine`
  - `QDeclarativeComponent`
  - `QDeclarativeView`
  - `QDeclarativeItem`
- Searched the BTK tree and found no actual `QQml*` or `QQuick*` source/header surface, including no discovered:
  - `QQmlApplicationEngine`
  - `QQmlEngine`
  - `QQmlContext`
  - `QQuickItem`
  - `QQuickView`
  - `qqml*.h`
  - `qquick*.h`
- This proves the BTK compatibility gap is also a **framework-generation mismatch**: even a revived BTK declarative module would still not directly satisfy bobfilez's current `QQmlApplicationEngine`-based GUI path.

#### 📊 Quantified Declarative Revival Footprint
- A structural scan over `libs/btk/src/declarative` found:
  - **61 files** with direct QtScript-related usage
  - **24 files** referencing `QScriptDeclarativeClass`
  - **23 files** still using `Q_DECLARE_METATYPE(...)`
- Confirmed additional missing internal-script surface signs, including absent expected files such as:
  - `qscriptdeclarativeclass_p.h`
  - `qscriptengine.h`
- This gives a more honest estimate of the scale involved if BTK declarative revival is pursued.

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND5.md`** documenting:
  - the `QQml*` vs `QDeclarative*` API-generation mismatch
  - the absence of `QQml*` / `QQuick*` provider surface in BTK
  - the quantified QtScript/metatype dependency footprint inside BTK declarative
  - the conclusion that the next step is now a strategic framework decision, not just another build fix

### Version
- Bumped to **6.0.32**.

## [6.0.31] - 2026-04-05

### Validated — The "BTK Native Provider Probe Round 4" Release

#### 🧪 Experimental Declarative Enablement Proved the Gap is Deeper Than a Missing Component List Entry
- Confirmed bobfilez's active native entrypoint still genuinely requires a QML stack (`QQmlApplicationEngine` in `gui/omni/src/main.cpp`), so the missing `Declarative` capability remains a hard blocker.
- Temporarily re-enabled `Declarative` in BTK's top-level component list as an experiment to determine whether the module was merely omitted or actually stale/incomplete.
- The experiment showed the current BTK line is not just missing a list entry:
  - `src/declarative/CMakeLists.txt` immediately failed on stale build-system usage such as obsolete `target_add_definitions(...)`
  - after minimal CMake modernization to keep probing, the build advanced into declarative compilation and then failed on deeper issues including:
    - obsolete `Q_DECLARE_METATYPE(TYPE)` diagnostics requiring `CS_DECLARE_METATYPE(TYPE)`
    - fatal missing QtScript-era headers such as `QtScript/qscriptvalue.h`
- This confirms the current BTK `Declarative` / QML path is structurally incomplete in this branch and not yet an honest provider for bobfilez's active GUI architecture.

#### 🔧 BTK Submodule Correctness Preserved While Probing
- While testing against the newer upstream BTK tip, restored the normal BTK MSVC build by fixing a regressed property-name path in:
  - `libs/btk/src/gui/kernel/qapplication_cs.cpp`
- Re-verified that the normal BTK module set still builds successfully after reverting the temporary declarative-enablement experiment.
- Updated the bobfilez gitlink to a pushed BTK branch commit carrying the corrected MSVC fix:
  - branch: `pi/msvc-focus-fixes-20260405`

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND4.md`** documenting:
  - the temporary declarative-enablement experiment
  - the stale declarative CMake findings
  - the deeper compile-time `QtScript` dependency gap
  - the conclusion that the remaining blocker is broader framework readiness, not just a missing component toggle

### Version
- Bumped to **6.0.31**.

## [6.0.30] - 2026-04-05

### Validated — The "BTK Native Provider Probe Round 3" Release

#### ✅ BTK MSVC Build Repaired Enough to Produce Real Framework Outputs
- Patched the active `libs/btk` submodule to resolve the earlier MSVC build blockers which had prevented BTK from producing consumer-usable binaries.
- Replaced unsupported `Q_DECLARE_FLAGS(...)` usage with BTK-compatible `using ... = QFlags<...>;` aliases in:
  - `libs/btk/src/core/kernel/btkinputowner.h`
  - `libs/btk/src/gui/widgets/btkfocusoverlay.h`
  - `libs/btk/src/plugins/bearer/networkmanager/qnetworkmanagerservice.h`
- Normalized newer Qt-style string/property API usage in:
  - `libs/btk/src/gui/kernel/btkfocusdiagnostics.cpp`
  - `libs/btk/src/gui/kernel/qapplication_cs.cpp`
  - `libs/btk/src/gui/widgets/btkfocusoverlay.cpp`
- Confirmed `scripts/build_btk_inplace.bat` now completes successfully and produces BTK/CopperSpice outputs such as:
  - `libs/btk/build-btk/lib/CsCore2.1.lib`
  - `libs/btk/build-btk/lib/CsGui2.1.lib`
  - related plugin and DLL artifacts

#### 🔍 Sharper Downstream BTK Consumer Finding
- Re-ran `scripts/build_btk_gui.bat` after BTK's successful in-place build.
- bobfilez no longer fails because BTK libraries are missing.
- The current failure is now more honest and architectural:
  - bobfilez cannot resolve a BTK/CopperSpice target for component `Declarative`
- Investigation confirmed:
  - `libs/btk/src/declarative/CMakeLists.txt` exists and defines `CsDeclarative`
  - but BTK's top-level `CS_OPTIONAL_COMPONENTS` list does **not** include `Declarative`
  - therefore the current BTK build/package surface does not provide the QML/Declarative module family bobfilez's native GUI expects

#### 🧰 Better Probe Diagnostics
- Updated `cmake/BTKFrameworkSetup.cmake` so missing BTK/CopperSpice components now report:
  - the candidate targets checked
  - the imported BTK/CopperSpice targets actually available
  - an explicit hint when the missing component is `Declarative`

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND3.md`** documenting:
  - the BTK MSVC build fixes
  - the successful BTK in-place build result
  - the new `Declarative`/QML capability boundary
  - the conclusion that the active blocker is now framework capability, not package layout or missing library outputs

### Version
- Bumped to **6.0.30**.

## [6.0.29] - 2026-04-04

### Validated — The "BTK Native Provider Probe Round 2" Release

#### 🧪 BTK Build-Tree Package Prep Improved
- Added:
  - `scripts/prepare_btk_buildtree_package.py`
  - `scripts/prepare_btk_buildtree_package.bat`
- Updated `scripts/build_btk_gui.bat` to prepare a consumable BTK build-tree package surface before probing bobfilez's BTK-backed GUI configure.

#### 🔍 Sharper BTK Consumer Finding
- The earlier missing-companion-file and bad-import-prefix issues were bypassed by preparing and patching the generated BTK/CopperSpice export files in `libs/btk/build-btk`.
- After that improvement, the BTK-backed bobfilez GUI probe now fails on a more meaningful downstream condition:
  - imported target `CopperSpice::CsCore` references `libs/btk/build-btk/lib/CsCore2.1.lib`
  - that file does not exist yet
- This confirms the remaining blocker is now the absence of successfully built BTK libraries, which traces back to BTK's own compile failure in `btkinputowner.*`.

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND2.md`** documenting the refined package-prep step, the corrected import-prefix handling, and the new narrowed blocker.

### Version
- Bumped to **6.0.29**.

## [6.0.28] - 2026-04-04

### Validated — The "BTK Native Provider Probe" Release

#### 🧪 Real BTK Consumer Validation
- Registered **`libs/btk`** as a real tracked submodule in the superproject.
- Ran the new BTK-native validation scripts:
  - `scripts/build_btk_inplace.bat`
  - `scripts/build_btk_gui.bat`

#### 🔍 BTK In-Place Build Findings
- BTK **configures successfully** on this host.
- The current BTK build then fails in BTK's own source during compilation, centered on:
  - `src/core/kernel/btkinputowner.h`
  - `src/core/kernel/btkinputowner.cpp`
- The error shape points to MSVC-side parsing / macro fallout around `Capabilities` / `Q_DECLARE_FLAGS` in `BtkInputOwner`.

#### 🧩 BTK Build-Tree Export Findings
- Fixed the initial BTK provider hint ordering in **`cmake/BTKFrameworkSetup.cmake`** so bobfilez prefers the generated `libs/btk/build-btk` package config over the raw source-tree template.
- After that fix, the BTK-backed bobfilez GUI probe advanced to a more meaningful failure:
  - `find_package(BTK CONFIG)` locates `libs/btk/build-btk/BTKConfig.cmake`
  - but that config currently references missing companion files such as:
    - `CopperSpiceLibraryTargets.cmake`
    - `CopperSpiceBinaryTargets.cmake`
    - `BTKMacros.cmake`
    - `BTKDeploy.cmake`
- Result:
  - the active blocker is now in **BTK build/export readiness**, not in the older BobUI wiring path

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE.md`** documenting the repaired submodule registration, the BTK in-place build result, the BTK build-tree export failure, and the new next-step boundary.

### Version
- Bumped to **6.0.28**.

## [6.0.27] - 2026-04-04

### Migrated — The "BTK Native Framework Switch" Release

#### 🔁 Native Framework Direction Changed
- Replaced the active BobUI/Qt-fork native-framework direction with **BTK** (`https://github.com/robertpelloni/btk`).
- Updated the primary native-provider terminology, docs, and build scripts to refer to:
  - `libs/btk`
  - `BTK_ROOT`
  - BTK-backed native GUI / Omni builds

#### 🧩 Build-System Retargeting
- Replaced **`cmake/BobUIQtSetup.cmake`** with **`cmake/BTKFrameworkSetup.cmake`**.
- Updated root and GUI CMake wiring to:
  - prefer BTK discovery via `BTK_ROOT` / `FO_BTK_ROOT`
  - stop assuming a BobUI/Qt6 provider model
  - resolve BTK / CopperSpice imported targets instead of `Qt6::*`
- Updated GUI helper scripts:
  - `scripts/build_bobui_gui.bat` → `scripts/build_btk_gui.bat`
  - `scripts/build_bobui_inplace.bat` → `scripts/build_btk_inplace.bat`

#### 🚫 BobUI-Specific Bootstrap Assumptions Removed
- Updated **`gui/omni/src/main.cpp`** to remove the BobUI-specific bootstrap dependency:
  - removed `#include "OmniQmlRegistration.h"`
  - removed `OmniUI::registerQmlTypes()`
- Updated GUI CMake targets to remove the BobUI-specific `OmniUI/omnicore` source-tree dependency.

#### 🖼️ BobUI QML Imports Replaced in Active Panels
- Updated **`gui/panels/PhotoDevelopPanel.qml`** and **`gui/panels/PhotoLibraryPanel.qml`**.
- Removed BobUI-specific imports:
  - `import OmniUI 1.0`
  - `import OmniLayout 1.0`
  - `import OmniData 1.0`
- Replaced their BobUI widget usage with local Qt Quick / Qt Quick Controls primitives so those panels no longer depend on BobUI namespaces.

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_SWITCH.md`** documenting the architectural difference between BobUI and BTK, the retargeted provider model, and the remaining compatibility work.
- Updated key project docs and agent instructions to describe BTK as the current native-framework direction.

### Version
- Bumped to **6.0.27**.

## [6.0.26] - 2026-04-04

### Reduced — The "QtQuick.Controls Gamification Reduction" Release

#### 🏆 Fifth Routed-Panel Conversion
- Updated **`gui/panels/GamificationPanel.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with plain `QtQuick` primitives and small local helpers, including:
  - `Label` → `Text`
  - action `Button` → local `PanelButton` helper
  - `ProgressBar` → local `SlimProgressBar` helper
  - `GroupBox` achievements section → explicit titled `Text` + `Rectangle` container
  - repeated summary cards → local `StatCard` helper
- Replaced the stock `ToolTip` behavior on achievement badges with a small local hover-popup rectangle, preserving lightweight hover detail without retaining the Controls dependency.

#### 📊 Routed-Panel Pattern Expanded Slightly
- This is the **fifth successful routed-panel controls-removal pass**.
- It also proves that limited hover-detail behavior can be replaced locally while keeping the panel self-contained and readable.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **35 QML files** importing `QtQuick.Controls`
  - to **34 QML files**
- The reduction campaign remains cumulative, measurable, and low-risk.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_GAMIFICATION_REDUCTION.md`** documenting why `GamificationPanel.qml` was a safe next escalation and how the local hover-popup replacement fits the migration strategy.

### Version
- Bumped to **6.0.26**.

## [6.0.25] - 2026-04-04

### Reduced — The "QtQuick.Controls Forensic Reduction" Release

#### 🛡️ Fourth Routed-Panel Conversion
- Updated **`gui/panels/ForensicPanel.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with plain `QtQuick` primitives and small local helpers, including:
  - `Label` → `Text`
  - action `Button`s → local `PanelButton` helper
  - `GroupBox` chain-of-custody section → explicit titled `Text` + `Rectangle` container
  - repeated ledger summary cards → local `StatCard` helper
- Removed the stock `ScrollBar.vertical: ScrollBar {}` attachment so the list view is fully free of Controls usage.
- Added local `integrityFillColor()` and `integrityTextColor()` helpers to keep badge styling explicit and readable.

#### 📊 Routed-Panel Pattern Reinforced Again
- This is the **fourth successful routed-panel controls-removal pass** after `DigitalRotPanel.qml`, `SwarmPanel.qml`, and `RecoveryPanel.qml`.
- The dashboard-style, interaction-light migration lane is now strongly established.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **36 QML files** importing `QtQuick.Controls`
  - to **35 QML files**
- The migration remains cumulative, measurable, and low-risk.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_FORENSIC_REDUCTION.md`** documenting why `ForensicPanel.qml` was a strong next candidate and how the helper-based rewrite preserved clarity.

### Version
- Bumped to **6.0.25**.

## [6.0.24] - 2026-04-04

### Reduced — The "QtQuick.Controls Recovery Reduction" Release

#### 🩹 Third Routed-Panel Conversion
- Updated **`gui/panels/RecoveryPanel.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with plain `QtQuick` primitives and small local helpers, including:
  - `Label` → `Text`
  - action `Button`s → local `PanelButton` helper
  - `GroupBox` integrity-exceptions section → explicit titled `Text` + `Rectangle` container
  - repeated dashboard metrics → local `StatCard` helper
- Added a local `corruptionColor()` helper to keep conditional stat styling readable and explicit.

#### 📊 Routed-Panel Pattern Strengthened
- This is the **third successful routed-panel controls-removal pass** after `DigitalRotPanel.qml` and `SwarmPanel.qml`.
- It further validates the current strategy of targeting dashboard-style, interaction-light panels before richer widget-heavy surfaces.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **37 QML files** importing `QtQuick.Controls`
  - to **36 QML files**
- The migration remains incremental, quantitative, and cumulative.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_RECOVERY_REDUCTION.md`** documenting why `RecoveryPanel.qml` was such a strong follow-up target and how the helper-based conversion preserves readability.

### Version
- Bumped to **6.0.24**.

## [6.0.23] - 2026-04-04

### Reduced — The "QtQuick.Controls Swarm Reduction" Release

#### 🛸 Second Routed-Panel Conversion
- Updated **`gui/panels/SwarmPanel.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with plain `QtQuick` primitives and small local helpers, including:
  - `Label` → `Text`
  - action `Button`s → local `PanelButton` helper
  - `GroupBox` detected-nodes section → explicit titled `Text` + `Rectangle` container
  - `ProgressBar` → local `SlimProgressBar` helper
- Added a small local `statusColor(status)` helper to keep status styling explicit and readable.

#### 📊 Routed-Panel Pattern Reinforced
- This is the **second successful routed-panel controls-removal pass** after `DigitalRotPanel.qml`.
- It strengthens the emerging migration rule that dashboard-style, interaction-light panels are the best next-wave candidates.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **38 QML files** importing `QtQuick.Controls`
  - to **37 QML files**
- The reduction campaign remains quantitative and cumulative.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_SWARM_REDUCTION.md`** documenting why `SwarmPanel.qml` was a cleaner next target than richer panels and how the helper-based conversion preserved readability.

### Version
- Bumped to **6.0.23**.

## [6.0.22] - 2026-04-04

### Reduced — The "QtQuick.Controls Digital Rot Reduction" Release

#### 🧹 First Routed-Panel Conversion
- Updated **`gui/panels/DigitalRotPanel.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with plain `QtQuick` primitives and small local helpers, including:
  - `Label` → `Text`
  - action `Button`s → local `PanelButton` helper
  - `GroupBox` suggestions section → explicit titled `Text` + `Rectangle` container
  - repeated stat cards → local `StatCard` helper
- Preserved the panel's existing scanning toggle behavior and dashboard-style presentation.

#### 🧭 Migration Phase Shift
- This is the **first successful controls-removal pass in the routed panel layer** after completion of the shell-assets layer.
- It confirms that the incremental strategy can continue into panels, provided the panel is self-contained and interaction-light.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **39 QML files** importing `QtQuick.Controls`
  - to **38 QML files**
- This keeps the campaign quantitative while the migration frontier moves from shell assets into routed panels.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_DIGITAL_ROT_REDUCTION.md`** documenting why `DigitalRotPanel.qml` was the correct first panel target, what helper-component strategy was used, and why this represents a new migration phase.

### Version
- Bumped to **6.0.22**.

## [6.0.21] - 2026-04-04

### Reduced — The "QtQuick.Controls Dashboard Reduction" Release

#### 📊 Dashboard Shell-Asset Conversion
- Updated **`gui/omni/assets/Dashboard.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock convenience controls with plain `QtQuick` primitives and local inline helpers, including:
  - `Label` → `Text`
  - settings `Button` → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`
  - `GroupBox` sections → explicit titled `Text` + `Rectangle` containers
  - `ProgressBar` → local `SlimProgressBar` helper component
  - quick action `Button`s → local `QuickActionChip` helper component

#### 🧭 Shell-Assets Milestone
- Confirmed that **`gui/omni/assets/` now has zero QML files importing `QtQuick.Controls`**.
- This marks the shell-assets layer as fully converted away from stock controls, leaving the remaining imports concentrated in routed panel files.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **40 QML files** importing `QtQuick.Controls`
  - to **39 QML files**
- The migration remains incremental, measurable, and low-risk.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_DASHBOARD_REDUCTION.md`** documenting the conversion strategy, the helper-component approach, and the new shell-assets milestone.

### Version
- Bumped to **6.0.21**.

## [6.0.20] - 2026-04-04

### Reduced — The "QtQuick.Controls Main Host Reduction" Release

#### 🧭 Main Shell Host Cleanup
- Updated **`gui/omni/assets/main.qml`** to remove `QtQuick.Controls` usage entirely.
- Converted the notification-center surface from stock controls to lighter `QtQuick` primitives, including:
  - `Label` → `Text`
  - `Clear All` button → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`
- This removes a stock-controls dependency from the shell host itself without touching the routed tool/panel components.

#### 📊 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **41 QML files** importing `QtQuick.Controls`
  - to **40 QML files**
- This keeps the controls-reduction effort measurable and cumulative.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_MAIN_REDUCTION.md`** documenting why `main.qml` was a good target, what was localized enough to convert safely, and how this fits the incremental migration plan.

### Version
- Bumped to **6.0.20**.

## [6.0.19] - 2026-04-04

### Reduced — The "QtQuick.Controls OmniPeek Reduction" Release

#### 👁️ OmniPeek Overlay Conversion
- Updated **`gui/omni/assets/OmniPeekOverlay.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with lighter `QtQuick` primitives, including:
  - `Label` → `Text`
  - header/buttons → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`
  - viewer buttons → lightweight custom button shells
  - `ScrollView` + `TextArea` → `Flickable` + read-only `TextEdit`

#### 📊 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **42 QML files** importing `QtQuick.Controls`
  - to **41 QML files**
- This demonstrates the controls-reduction strategy can extend beyond minimal shell chrome into richer overlay surfaces while remaining incremental.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_OMNIPEEK_REDUCTION.md`** documenting the replacement set, tradeoffs, and why OmniPeek was a safe next-step target.

### Version
- Bumped to **6.0.19**.

## [6.0.18] - 2026-04-04

### Reduced — The "QtQuick.Controls Explorer Reduction" Release

#### 🪟 Explorer Window Conversion
- Updated **`gui/omni/assets/ExplorerWindow.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with lighter `QtQuick` patterns, including:
  - `Label` → `Text`
  - toolbar `Button`s → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`
  - command-bar action button → custom rectangle button shell
  - `ItemDelegate` → rectangle/list-row shells
  - removed the stock `ScrollBar` attachment

#### 📊 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **43 QML files** importing `QtQuick.Controls`
  - to **42 QML files**
- The shell-adjacent no-Controls set now includes:
  - `Taskbar.qml`
  - `StartMenu.qml`
  - `DesktopIcons.qml`
  - `NexusPulse.qml`
  - `WindowManager.qml`
  - `ExplorerWindow.qml`

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_EXPLORER_REDUCTION.md`** documenting the replacement set, tradeoffs, and why this was the next safe escalation after the earlier shell control cleanups.

### Version
- Bumped to **6.0.18**.

## [6.0.17] - 2026-04-04

### Reduced — The "QtQuick.Controls Second Reduction" Release

#### 🧱 Additional Shell-Adjacent Control Cleanup
- Updated **`gui/omni/assets/DesktopIcons.qml`** to remove `QtQuick.Controls` usage.
- Updated **`gui/omni/assets/NexusPulse.qml`** to remove `QtQuick.Controls` usage.
- Updated **`gui/omni/assets/WindowManager.qml`** to remove a dead `QtQuick.Controls` import.
- Replaced the remaining trivial `Label` usage in the first two files with `Text`.

#### 📊 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **46 QML files** importing `QtQuick.Controls`
  - to **43 QML files**
- Combined with the first reduction pass, the shell-adjacent no-Controls set now includes:
  - `Taskbar.qml`
  - `StartMenu.qml`
  - `DesktopIcons.qml`
  - `NexusPulse.qml`
  - `WindowManager.qml`

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_SECOND_REDUCTION.md`** documenting the file choices, measurement update, and what categories remain intentionally deferred.

### Version
- Bumped to **6.0.17**.

## [6.0.16] - 2026-04-04

### Reduced — The "QtQuick.Controls Initial Reduction" Release

#### 🧱 First Controls-Surface Cut
- Updated **`gui/omni/assets/Taskbar.qml`** to remove `QtQuick.Controls` usage entirely.
- Updated **`gui/omni/assets/StartMenu.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock convenience controls (`Label`, trivial `Button`, `ToolTip`) with simpler `QtQuick` primitives such as:
  - `Text`
  - `Rectangle`
  - `MouseArea`

#### 📊 Surface Measurement
- Recomputed the current `QtQuick.Controls` import footprint across the QML tree.
- Result:
  - reduced from **48 QML files** importing `QtQuick.Controls`
  - to **46 QML files**
- This is a small but real reduction on two shell-critical surfaces.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_INITIAL_REDUCTION.md`** documenting the exact files changed, why they were chosen, what was intentionally deferred, and how this fits the phased migration plan.

### Version
- Bumped to **6.0.16**.

## [6.0.15] - 2026-04-04

### Wired — The "BobUI Registration Alignment" Release

#### 🧩 BobUI Layout / Bootstrap Correction
- Updated **`gui/CMakeLists.txt`** and **`gui/omni/CMakeLists.txt`** to point at the current BobUI source layout:
  - `libs/bobui/OmniUI/omnicore`
- Removed the stale `OmniUI/core` assumption.
- Added recursive BobUI omnicore source inclusion and the corresponding include paths for:
  - `omnicore/include`
  - `omnicore/src`
  - `deps/juce`
  - `deps/imgui`

#### 🚀 Real BobUI QML Registration Call
- Updated **`gui/omni/src/main.cpp`** to include:
  - `OmniQmlRegistration.h`
- Added a real startup call to:
  - `OmniUI::registerQmlTypes();`
- This means bobfilez now structurally attempts to back imports such as `OmniUI`, `OmniLayout`, and `OmniData` with BobUI registrations instead of only registering its local bridge types.

#### 📘 Documentation
- Added **`docs/ai/implementation/BOBUI_REGISTRATION_WIRING.md`** documenting the stale-path bug, the current BobUI layout, the bootstrap correction, and the remaining limitations.

#### 🔍 Important Limitation
- This does **not** solve the already-known full GUI blocker:
  - BobUI still lacks the full `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` provider surface needed by bobfilez's current GUI targets on this machine.
- This release improves integration correctness, not end-to-end GUI buildability.

### Version
- Bumped to **6.0.15**.

## [6.0.14] - 2026-04-04

### Simplified — The "GraphicalEffects Full Removal" Release

#### ✨ Final Effect Users Replaced
- Reworked the remaining real `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects` users in:
  - `gui/omni/assets/AcrylicBackground.qml`
  - `gui/omni/assets/Dashboard.qml`
  - `gui/omni/assets/ExplorerWindow.qml`
  - `gui/omni/assets/OmniPeekOverlay.qml`
  - `gui/omni/assets/Taskbar.qml`
  - `gui/panels/OmniVersePanel.qml`
- Removed the last actual uses of:
  - `DropShadow`
  - `FastBlur`
- Replaced them with simpler native borders / tint layers / layout-preserving fallbacks.

#### ✅ Verification / Finding
- Confirmed there are now **no remaining** `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects` imports under `gui/`.
- Confirmed there are now **no remaining** GraphicalEffects primitives under `gui/` such as `DropShadow` or `FastBlur`.
- This completes the GraphicalEffects cleanup sequence started in the prior release.

#### 📘 Documentation
- Added **`docs/ai/implementation/GRAPHICALEFFECTS_FULL_REMOVAL.md`** documenting the exact files changed, what visual simplifications were made, and why the tradeoff is acceptable at this stage.

### Version
- Bumped to **6.0.14**.

## [6.0.13] - 2026-04-04

### Cleaned — The "GraphicalEffects Import Cleanup" Release

#### ✨ Dead Effect Imports Removed
- Audited `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects` usage across the QML tree and separated real effect users from dead imports.
- Removed stale effect imports from 27 QML files where no effect items (`DropShadow`, `FastBlur`, etc.) were actually instantiated.
- Left the six real effect-using files unchanged for now so visual behavior remains stable while the dependency surface becomes easier to reason about.

#### 🔍 Key Finding
- The actual current GraphicalEffects dependency footprint is much smaller than the raw import count previously suggested.
- Remaining real effect users are concentrated in a small set of shell/chrome surfaces, which makes the next replacement phase more focused and less risky.

#### 📘 Documentation
- Added **`docs/ai/implementation/GRAPHICALEFFECTS_IMPORT_CLEANUP.md`** documenting the audit, exact file sets, and the recommended next step.

### Version
- Bumped to **6.0.13**.

## [6.0.12] - 2026-04-04

### Cleaned — The "QtCharts Removal" Release

#### 📉 Dead GUI Dependency Removed
- Updated **`gui/panels/EnhancedFileOpsPanel.qml`** to remove:
  - `import QtCharts 2.5`
- Confirmed that the panel's real-time speed graph was already implemented with a native `Canvas`-based painter path rather than `ChartView` / `LineSeries` / other QtCharts types.
- This means the `QtCharts` import had become unused dependency residue and could be removed safely.

#### ✅ Verification / Finding
- Confirmed there are no remaining `QtCharts` references under `gui/` after this cleanup.
- This further shrinks the native UI dependency surface without changing behavior.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTCHARTS_REMOVAL.md`** documenting why the removal was safe, what the panel was already using instead, and why this matches the phased BobUI migration strategy.

### Version
- Bumped to **6.0.12**.

## [6.0.11] - 2026-04-04

### Fixed — The "Markdown WebEngine Removal" Release

#### 🧾 Native Markdown Preview
- Added **`gui/omni/src/NativeMarkdownView.h`** and **`gui/omni/src/NativeMarkdownView.cpp`**.
- The new preview item renders markdown natively through `QQuickPaintedItem` + `QTextDocument` while reusing the existing core `MarkdownRenderer`.
- Exposed basic preview metadata back to QML:
  - word count
  - reading time
  - document title

#### 🧹 WebEngineQuick Dependency Reduction
- Updated **`gui/panels/MarkdownViewerPanel.qml`** to remove:
  - `import QtWebEngine 1.9`
  - `WebEngineView`
- Replaced the preview pane with the new native `MarkdownView` type registered under `Omni.Native 1.0`.
- Updated **`gui/omni/CMakeLists.txt`** to remove `WebEngineQuick` from the required Qt component set and target link libraries.

#### 🧩 Bootstrap / Shared GUI Wiring
- Updated **`gui/omni/src/main.cpp`** to register the native markdown preview QML type.
- Updated **`gui/CMakeLists.txt`** so the shared `fo_gui` path also includes `NativeMarkdownView` sources.
- Updated **`core/include/fo/core/markdown_viewer_interface.hpp`** comments so browser/WebEngine rendering is described as an optional frontend path rather than the only rendering model.

#### ✅ Verification / Finding
- Confirmed there are no remaining `QtWebEngine`, `WebEngineView`, or `WebEngineQuick` references under `gui/`.
- Re-ran the BobUI consumer probe and confirmed the remaining blocker is still:
  - `Qt6Qml`
- In other words, this phase successfully removed one heavyweight GUI dependency without changing the deeper declarative-stack blocker already identified.

#### 📘 Documentation
- Added **`docs/ai/implementation/MARKDOWN_WEBENGINE_REMOVAL.md`** documenting the implementation, validation, tradeoffs, and next-step implications.

### Version
- Bumped to **6.0.11**.

## [6.0.10] - 2026-04-04

### Analyzed — The "BobUI Native Migration Audit" Release

#### 🧭 Migration Cost Audit
- Added **`docs/ai/implementation/BOBUI_NATIVE_MIGRATION_AUDIT.md`**.
- Audited the real cost of removing the current bobfilez dependency set on:
  - QML composition
  - `QtQuick.Controls`
  - `QtQuick.Layouts`
  - `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects`
  - `QtWebEngine`
  - `QtCharts`
- Quantified the current native shell surface as:
  - **49 QML files**
  - **9,844 QML lines**
  - **39 routed shell/panel surfaces** in `main.qml`

#### 🔍 Critical Architectural Finding
- Confirmed that current BobUI widgets/layouts are themselves based on:
  - `QQuickItem`
  - `QQuickPaintedItem`
- This means a BobUI-first migration does **not** currently imply removal of **QtQuick** itself.
- The practical migration targets must therefore be separated:
  - remove `WebEngineQuick` → realistic
  - reduce stock Quick Controls / Effects → hard but incremental
  - remove QML composition entirely → major rewrite
  - remove QtQuick itself → not realistic with current BobUI architecture

#### 🧱 Additional Integration Finding
- Confirmed that bobfilez currently does **not** call BobUI's `OmniUI::registerQmlTypes()` in its bootstrap path.
- This means the project is still largely a **stock Qt Quick shell with thin C++ model bridges**, not yet a genuinely BobUI-driven shell.

#### ✅ Recommendation Captured
- The best near-term path is:
  1. remove `WebEngineQuick`
  2. trim other nonessential stock Qt modules
  3. adopt BobUI widgets/layouts incrementally while keeping QML as a composition layer for now
- A wholesale "delete QML" push is currently the wrong first move.

### Version
- Bumped to **6.0.10**.

## [6.0.9] - 2026-04-04

### Investigated — The "In-Place BobUI Build Probe" Release

#### 🧪 BobUI Build Attempt
- Added **`scripts/build_bobui_inplace.bat`** to configure and build BobUI in place under `libs/bobui/build-bobui`.
- Confirmed that BobUI can be configured as an in-place **developer build** on this machine when the MSVC minimum-version guard is explicitly bypassed.
- The configure step generated a real:
  - `libs/bobui/build-bobui/lib/cmake/Qt6/Qt6Config.cmake`

#### 🔍 Critical Finding
- Re-ran bobfilez against the in-place BobUI build tree.
- bobfilez was finally able to see the top-level **Qt6** package config from BobUI, but failed immediately on required missing components:
  - `Qt6Qml`
  - therefore also no practical `Qt6Quick`, `Qt6QuickControls2`, or `Qt6WebEngineQuick`
- This demonstrates that the current BobUI tree is **not yet a full drop-in provider** for the bobfilez GUI stack as currently written.

#### 📘 Documentation
- Updated **`docs/ai/implementation/BOBUI_PROVIDER_SETUP.md`** with the exact in-place build/probe result and the current decision tree.

### Version
- Bumped to **6.0.9**.

## [6.0.8] - 2026-04-04

### Added — The "BOBGUI Evaluation" Release

#### 📦 New Submodule
- Added **`libs/bobgui`** as a new git submodule:
  - `https://github.com/robertpelloni/bobgui`

#### 🔎 Comparative Analysis
- Added **`docs/ai/implementation/BOBGUI_VS_BOBUI.md`**.
- Compared `bobgui` and `bobui` across:
  - architecture lineage
  - language/runtime model
  - build system
  - platform/toolkit assumptions
  - practical fit for bobfilez
- Conclusion:
  - **`bobui` is the better library for bobfilez** because it aligns with the existing Qt/QML/Omni shell direction and avoids a frontend rewrite.
  - `bobgui` appears stronger as a standalone GTK-style toolkit foundation, but is a poor strategic fit for the current bobfilez UI architecture.

### Version
- Bumped to **6.0.8**.

## [6.0.7] - 2026-04-04

### Changed — The "BobUI Update & Port Cleanup" Release

#### 🧩 BobUI Submodule Update
- Updated **`libs/bobui`** from `01634f269f` to `581de545a4` on `origin/main`.
- This brings in the latest BobUI / OmniUI upstream state, including broader framework changes and the current BobUI repository structure.

#### 🧹 Alternate-Port Cleanup
- Removed the obsolete **`filez-java/`** experiment from the repository.
- Removed Java-only packaging artifacts:
  - `scripts/package_java_msi.bat`
  - `wix/filez_java.wxs`
- Updated deployment and architecture notes so bobfilez no longer describes any maintained alternate port currently living in-tree.

#### 🧪 Feasibility Finding
- Added **`scripts/build_bobui_gui.bat`** as a repeatable MSVC/Ninja probe/build entrypoint for BobUI-backed GUI targets.
- BobUI can **likely** be made to work as the GUI provider for bobfilez, but not by pointing bobfilez at a raw BobUI source checkout alone.
- A real configure probe against `BOBUI_ROOT=libs/bobui` now fails with the expected package-discovery blocker:
  - missing `Qt6Config.cmake` / `qt6-config.cmake`
- The current integration therefore still needs a **BobUI-built/exported Qt6 package layout** so `find_package(Qt6 ...)` can resolve successfully.
- In other words, the path forward is:
  1. build/install BobUI
  2. point `BOBUI_ROOT` at that built/exported prefix
  3. configure bobfilez GUI / Omni targets against those package configs

### Version
- Bumped to **6.0.7**.

## [6.0.6] - 2026-04-04

### Improved — The "BobUI Qt Provider Preference" Release

#### 🖥️ BobUI-First GUI Discovery
- Added **`cmake/BobUIQtSetup.cmake`**.
- The root build now prefers **`github.com/robertpelloni/bobui`** as the Qt provider whenever `FO_BUILD_GUI` or `FO_BUILD_OMNI` is enabled.
- The helper checks:
  - `BOBUI_ROOT`
  - `FO_BOBUI_ROOT`
  - local `libs/bobui`
- It appends likely BobUI build/install prefixes to `CMAKE_PREFIX_PATH` so `find_package(Qt6 ...)` can resolve against BobUI rather than assuming a separately installed stock Qt.

#### 🔧 Build Wording Cleanup
- Updated root option descriptions to refer to the **BobUI GUI** / **BobUI Omni shell** instead of generic Qt wording.
- Added status messages in:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
  to make the BobUI-backed Qt resolution path explicit during configure.

#### 📘 Documentation
- Added **`docs/ai/implementation/BOBUI_PROVIDER_SETUP.md`** documenting the correct integration model:
  - BobUI is a Qt fork
  - bobfilez should still consume normal `Qt6::*` targets
  - but those packages should come from BobUI when GUI builds are enabled
- Updated **`README.md`** to describe BobUI as the preferred native GUI stack.

#### 🧭 Go Port Status Clarification
- Confirmed there is **no active Go port** in the current bobfilez tree.
- At that point the repo still contained the older `filez-java/` experiment; this was removed in **6.0.7**.

### Version
- Bumped to **6.0.6**.

## [6.0.5] - 2026-04-04

### Fixed — The "Headless Build Stabilization" Release

#### 🛠️ Windows/MSVC Build Recovery
- Added **`scripts/build_headless.bat`** to provide a repeatable Windows/MSVC fallback build that:
  - locates `vcvars64.bat`
  - configures `build-msvc` with `FO_BUILD_GUI=OFF` and `FO_BUILD_OMNI=OFF`
  - builds the core library, CLI, tests, and benchmarks with conservative parallelism
- This gives the repo a reliable non-Qt build path even when the full Omni/Qt environment is not installed.

#### 📦 Dependency Strategy Improvement
- Updated **`vcpkg.json`** so native `ffmpeg` and `chromaprint` are no longer forced in the default dependency set.
- Added a new optional **`media-analysis`** feature containing those heavier packages.
- This prevents the default Windows build from being blocked by FFmpeg compilation/link failures on hosts that only need the core/CLI/test toolchain.

#### 🔧 Core/CLI Compile Fixes
- Fixed multiple latent compile issues that were surfaced once the dependency blockers were reduced, including problems across:
  - `core/CMakeLists.txt`
  - `cli/fo_cli.cpp`
  - `core/include/fo/core/batch_rename_interface.hpp`
  - `core/include/fo/core/conversion_interface.hpp`
  - `core/include/fo/core/enhanced_fileops_interface.hpp`
  - `core/include/fo/core/omnivision_engine_interface.hpp`
  - `core/include/fo/core/search_interface.hpp`
  - `core/src/advanced_archive_manager.cpp`
  - `core/src/batch_rename_engine.cpp`
  - `core/src/conversion_engine.cpp`
  - `core/src/enhanced_fileops.cpp`
  - `core/src/hex_editor.cpp`
  - `core/src/markdown_viewer.cpp`
  - `core/src/omniverse_engine.cpp`
  - `core/src/wasm_bridge.cpp`
- Removed the remaining MSVC warning in the CLI `convert` command caused by a shadowed `output_path` variable.

#### ✅ Verification
- Verified successful headless build outputs for:
  - `fo_core`
  - `fo_cli`
  - `fo_tests`
  - benchmark binaries
- Ran **`build-msvc/tests/fo_tests.exe`** successfully:
  - **63 / 63 tests passed**
- Ran **`build-msvc/cli/fo_cli.exe --help`** successfully as a CLI smoke test.

#### 📘 Documentation
- Added **`docs/ai/implementation/HEADLESS_BUILD_STABILIZATION.md`** to record the dependency strategy, script, compile fixes, verification results, and the remaining Qt6 blocker for the full GUI/shell build.

### Version
- Bumped to **6.0.5**.

## [6.0.4] - 2026-04-03

### Added — The "Tracked-Only Status Workflow" Release

#### 🧰 Repo Status Tooling
- Added **`scripts/repo_status.py`** as a cross-platform helper for a safer day-to-day status view.
- The helper runs:
  - `git status --short --branch --untracked-files=no`
- This captures the important tracked-change signal while avoiding the noisy long-path untracked scan currently seen on this Windows host.

#### 🔍 New Diagnostic Finding
- Confirmed that the remaining long-path warning is specifically tied to **untracked-file scanning**.
- `git status --untracked-files=no` completes cleanly and still reports:
  - tracked file changes
  - deletions
  - dirty submodules
- Updated **`docs/ai/implementation/REPO_HYGIENE_CLEANUP.md`** to record this finding and the temporary workflow recommendation.

### Version
- Bumped to **6.0.4**.

## [6.0.3] - 2026-04-03

### Cleaned — The "Generated Build Artifact Purge" Release

#### 🧹 Build Artifact Cleanup
- Removed tracked/generated CMake snapshot directories from the working tree for:
  - `benchmarks/build_output/`
  - `cli/build_output/`
  - `core/build_output/`
  - `fuzz/build_output/`
  - `gui/build_output/`
  - `tests/build_output/`
- Removed transient `tests/build/` output from the working tree as part of the hygiene pass.
- Added **`docs/ai/implementation/REPO_HYGIENE_CLEANUP.md`** to document what was removed and why these files should not remain part of normal repo state.

#### 🔍 Findings
- Confirmed that several component-level `build_output/` trees contained environment-specific CMake-generated artifacts such as `CMakeCache.txt`, compiler-ID files, and configure logs.
- These artifacts are poor candidates for version control because they are machine-specific and rebuildable.
- **Important:** root-level `git status` still emitted filename-too-long warnings for nested `tests/test_cmake_build/.../pybind11/...` paths even after the cleanup.
- This indicates that the remaining long-path issue likely requires a more specialized extended-path pruning pass beyond simple build-output deletion.

### Version
- Bumped to **6.0.3**.

## [6.0.2] - 2026-04-03

### Improved — The "OmniShell Route Audit & Repo Hygiene" Release

#### 🧭 Route Audit & Launcher Coverage
- Added **`docs/ai/implementation/OMNISHELL_ROUTE_AUDIT.md`** to document every `shell.activePanel` route currently hosted by `gui/omni/assets/main.qml`.
- Audited which routes are reachable from the **Taskbar**, **Start Menu**, and **Explorer/sidebar** surfaces.
- Identified the current hidden/contextual route set (`rename`, `convert`, `hex`, `image`, `md`, `watcher`, `fileops`, `visual_dedup`, `pruner`, `achievements`, `forensic`, `develop`) for deliberate future exposure decisions.

#### 📌 Taskbar Wiring
- Upgraded **`gui/omni/assets/Taskbar.qml`** from placeholder pinned-app icons to real Bobfilez launchers.
- Added direct taskbar routing for:
  - `explorer`
  - `omnigit`
  - `omnivision`
  - `omniaudio`
  - `terminal`
  - `omnishare`
- Added active-route indicator logic based on `shell.activePanel`, so the taskbar now reflects which pinned subsystem is currently open.

#### 🧹 Repo Hygiene Notes
- Added targeted ignore rules to **`.gitignore`** for generated test/build trees such as `tests/test_cmake_build/` and `tests/**/build_output/`.
- This is intended to reduce noise from generated artifacts and deeply nested dependency outputs.
- **Finding:** `git status` still emitted filename-too-long warnings even after these ignores were added, indicating the underlying generated tree may need direct cleanup/pruning rather than ignore-only mitigation.

### Version
- Bumped to **6.0.2**.

## [6.0.1] - 2026-04-03

### Fixed — The "OmniShell Bootstrap Stabilization" Release

#### 🛠️ OmniUI / OmniShell Stabilization
- Fixed **`gui/omni/CMakeLists.txt`** to compile **`src/TreemapModel.cpp`**, which is required by `TopologyPanel.qml` and was previously missing from the target source list.
- Added a local include path for `gui/omni/src` so shell bridge headers are resolved consistently during compilation.
- Replaced the BobUI-specific OmniApp bootstrap in **`gui/omni/src/main.cpp`** with a leaner Qt-native startup path using `QGuiApplication` + `QQmlApplicationEngine`.
- Corrected QML type registration to explicitly register **`fo::gui::TreemapModel`** under `Omni.Viz 1.0`, matching the namespace used by `TopologyPanel.qml`.
- Switched shell startup to load **`qrc:/main.qml`**, aligning runtime boot with the packaged QML resource file instead of a filesystem-relative path.

#### 🧭 OmniShell Surface Wiring
- Expanded **`StartMenu.qml`** pinned apps to expose the newer shell capabilities directly:
  - `Search`
  - `OmniTerminal`
  - `OmniShare`
  - `OmniCluster`
  - `Topology`
  - `Vault`
- This reduces routing drift between the shell's `activePanel` host and the visible launch surface.

#### 🔍 Verification Findings
- Attempted a fresh CMake configure/build path for the repo. Configuration progressed into dependency resolution but **timed out while vcpkg was building FFmpeg**, so full compile validation remains pending.
- Reconfirmed repo-hygiene issues already captured in docs:
  - filename-too-long warnings under nested `tests/test_cmake_build/.../pybind11/...`
  - unrelated dirty submodules/worktrees remained intentionally untouched

### Version
- Bumped to **6.0.1**.

## [6.0.0] - 2026-04-03

### Added — The "Grand Architecture & Unification" Release

#### 🏛️ Documentation Consolidation
- Added **`ARCHITECTURE.md`** as a high-level system map for the modern Bobfilez stack.
- Documented the OmniShell surface area, subsystem groupings, architectural layers, and product-parity targets.
- Recorded current project metrics gathered from the repository:
  - **82 C++ headers**
  - **85 C++ source files**
  - **39 QML panels**
  - **10 OmniShell QML assets**
  - **15,235 core C++ LOC**
  - **9,833 QML LOC**
  - **307 git commits**
- Reconciled version metadata to **6.0.0** in `VERSION.md` and `core/include/fo/core/version.hpp`.
- Updated operational docs (`AGENTS.md`, `HANDOFF.md`, handoff archive) to reflect the current architecture-first stabilization phase.

#### 🔍 Findings & Analysis
- The repository now spans a broad Omni-tool surface area and needs a stabilization phase focused on build verification, backend wiring, and doc/version reconciliation.
- `git status` shows several unrelated submodule worktree modifications (`ai-file-sorter`, `libs/dokany`, `libs/image-hash`, `libs/libjpeg`, `libs/libjpeg-turbo`, `libs/sumatrapdf`) that were intentionally **not** staged in this documentation release.
- `git status` also emits filename-too-long warnings under nested `tests/test_cmake_build/.../pybind11/...` paths; this should be addressed separately during repo hygiene work.

### Version
- Bumped to **6.0.0**.

## [5.8.0] - 2026-04-03

### Added — The "OmniShare Secure File Drops" Release

#### 📤 OmniShare: Zero-Config Secure Sharing
- Added **`omnishare_interface.hpp`** and **`omnishare_engine.cpp`** for temporary secure file sharing.
- Added **`OmniSharePanel.qml`** for creating, inspecting, copying, and revoking self-hosted share links.
- Wired OmniShare into OmniShell QML resources and the main panel host.
- Introduced the HTTPS/self-hosted share-link concept with max-download and expiry metadata.

### Version
- Bumped to **5.8.0**.

## [5.7.0] - 2026-04-03

### Added — The "OmniTerminal AI Shell" Release

#### ⌨️ OmniTerminal: Embedded AI Terminal
- Added **`omniterminal_interface.hpp`** and **`omniterminal_engine.cpp`** for terminal-session orchestration and AI suggestions.
- Added **`OmniTerminalPanel.qml`** with terminal output, input line, tab styling, and quick AI suggestion chips.
- Wired OmniTerminal into OmniShell QML resources and the shell panel switcher.
- Established the UX foundation for PTY-backed shell sessions integrated with OmniOracle guidance.

### Version
- Bumped to **5.7.0**.

## [5.6.0] - 2026-04-03

### Added — The "OmniCluster Distributed Grid Computing" Release

#### ⚡ OmniCluster: Multi-Node Compute Grid
- Added **`omnicluster_interface.hpp`** and **`omnicluster_engine.cpp`** for node discovery, distributed task tracking, and cluster dispatch concepts.
- Added **`OmniClusterPanel.qml`** to visualize available nodes, active distributed jobs, and per-node work allocation.
- Introduced the Bobfilez concept of farmed-out rendering, cracking, and OCR/AI workloads across the Data Swarm mesh.

### Version
- Bumped to **5.6.0**.

## [5.5.0] - 2026-04-03

### Added — The "OmniCrypt Steganography" Release

#### 🕵️ OmniCrypt: Advanced Data Concealment
- **`omnicrypt_interface.hpp` & `omnicrypt_engine.cpp`**: Implemented a military-grade cryptographic engine utilizing `libsodium` (XChaCha20-Poly1305) and `opencv`.
- **LSB Steganography**: Allows users to hide highly sensitive payloads (e.g., password databases, private keys) entirely inside the pixel data of standard `.jpg` or `.png` images.
- **Plausible Deniability**: The carrier image looks and opens normally in any image viewer, leaving no trace that a file is hidden inside.
- **`OmniCryptPanel.qml`**: A sleek, high-security dashboard allowing users to drag and drop payload files into carrier images and extract them with a master password.

### Version
- Bumped to **5.5.0**.

## [5.4.0] - 2026-04-03

### Added — The "OmniVerse 3D Spatial Computing" Release

#### 🌌 OmniVerse 3D File Explorer
- **`omniverse_interface.hpp` & `omniverse_engine.cpp`**: Built a hardware-accelerated 3D Force-Directed Graph using the `raylib` submodule.
- Converts the flat, 2D folder hierarchy into an interactive, spatial 3D universe.
- **Folders as Solar Systems**: Directories form gravity wells.
- **Files as Planets**: Individual files orbit their parent directories, with their physical size mapped to the file size, and color-coded by file type (Code, Video, Image).
- **`OmniVersePanel.qml`**: A sleek launchpad UI to configure the 3D physics (Gravity, Repulsion, Friction) and launch the OpenGL context.
- **VR Readiness**: Laid the groundwork for OpenXR compatibility, allowing users to literally walk through their file system.

### Version
- Bumped to **5.4.0**.

## [5.3.0] - 2026-04-03

### Added — The "OmniGit Version Control" Release

#### 🐙 OmniGit: Professional Git Client Integration
- **`omnigit_interface.hpp` & `omnigit_engine.cpp`**: Deep integration with the `libgit2` submodule, embedding a full Git client directly into the file manager shell (bypassing the need for GitKraken or Sourcetree).
- **Zero-CLI Execution**: Push, pull, commit, branch, and stash operations happen natively.
- **`OmniGitPanel.qml`**: A highly polished Version Control UI featuring:
  - Visual, color-coded commit history graphs.
  - Interactive staging area (drag-and-drop to stage).
  - Inline code diff viewer showing explicit patch additions and removals (e.g., `@@ -0,0 +1,24 @@`).

### Version
- Bumped to **5.3.0**.

## [5.2.0] - 2026-04-03

### Added — The "OmniPeek Universal Quick Look" Release

#### 👀 OmniPeek Instant Preview Engine
- **`omnipeek_interface.hpp` & `omnipeek_engine.cpp`**: Implemented a universal C++ bridge that instantly categorizes and renders any file type.
- **Spacebar Quick Look**: Selecting any file in the Explorer and pressing `Spacebar` now summons a glassmorphic overlay (`OmniPeekOverlay.qml`).
- **Submodule Integration**:
  - **`raylib`**: Renders 3D models (`.obj`, `.fbx`) natively.
  - **`mpv`**: Instantly plays video and audio files without launching an external player.
  - **`sqlite3`**: Opens database files read-only and displays the schema and table row counts.
  - **`radare2`**: Analyzes executables and dynamically displays the entry-point assembly instructions.
  - **`libarchive`**: Shows the folder tree of `.zip` and `.7z` files without extracting them.

### Version
- Bumped to **5.2.0**.

## [5.1.0] - 2026-04-03

### Added — The "OmniClerk AI Secretary" Release

#### 💼 OmniClerk Autonomous Pipeline
- **`omniclerk_interface.hpp` & `omniclerk_engine.cpp`**: Created a high-level orchestration engine that combines `FileWatcher`, `Tesseract OCR`, `BERT (NER)`, `BatchRename`, and `EnhancedCopy`.
- **Autonomous Filing**: Bobfilez can now watch your `~/Downloads` or `~/Scans` folders. When a new receipt, invoice, or contract arrives, it automatically extracts the text, identifies the vendor/date/amount, renames the file mathematically, and moves it to the correct sorted directory.
- **`OmniClerkPanel.qml`**: A live "Secretary Dashboard" that shows the real-time feed of documents being ingested, analyzed, and filed, complete with confidence scores and auto-generated metadata tags.

### Version
- Bumped to **5.1.0**.

## [5.0.0] - 2026-04-03

### Added — The "OmniSec & God-Tier" Release

#### ☠️ OmniSec Cybersecurity & Reverse Engineering Suite
- **`omnisec_engine.cpp`**: Built a high-performance C++ backend wrapping three major submodules (`radare2`, `hashcat`, and `ssdeep`).
- **Binary Analysis**: Disassemble executables directly in the file manager using the interactive `radare2` engine.
- **Malware Scanning**: Calculates `ssdeep` fuzzy hashes to detect malicious files or code permutations locally, automatically suggesting quarantine to the Encrypted Vault.
- **Password Recovery**: Integrated GPU-accelerated `hashcat` allowing users to brute-force or dictionary-attack forgotten passwords on ZIP/7z/RAR archives directly inside the shell.
- **`OmniSecPanel.qml`**: A specialized, hacker-themed dashboard with three tabs: Malware Scan, Disassembly, and Password Recovery.

### Version
- Bumped to **5.0.0**.

## [4.9.0] - 2026-04-03

### Added — The "OmniAudio DAW" Release

#### 🎧 OmniAudio: In-Shell Digital Audio Workstation
- **`omniaudio_engine_interface.hpp` & `omniaudio_engine.cpp`**: Implemented a professional Digital Audio Workstation (DAW) engine directly into the file manager shell.
- **AI Stem Separation**: Integrated Meta `audiocraft` (Demucs) concepts to automatically extract isolated stems (Vocals, Drums, Bass, Melody) from any mixed audio track with a single click.
- **Acoustic Fingerprinting**: Implemented `ShazamIO` / `audio-recognizer` logic to instantly identify unknown audio files and tag them with correct artist/album metadata.
- **`OmniAudioPanel.qml`**: A comprehensive multi-track timeline UI featuring draggable audio clips, simulated waveform rendering, a master mixer with vertical VU meters and faders, and a VST/DSP plugin rack.

### Version
- Bumped to **4.9.0**.

## [4.8.0] - 2026-04-03

### Added — The "OmniVision NLE" Release

#### 🎬 OmniVision: In-Shell Video Editing & Color Grading
- **`omnivision_engine_interface.hpp` & `omnivision_engine.cpp`**: Implemented a professional Non-Linear Editor (NLE) engine.
- Integrates `OpenTimelineIO` for sequence arrangement and `OpenColorIO` for cinematic color management.
- Uses `FFmpeg` and `mpv` bindings for real-time scrubbing and final rendering.
- **`OmniVisionPanel.qml`**: A comprehensive video editing workspace inside the file manager, featuring a Media Pool, Video Monitor, Color Wheels/LUT selectors, and a multi-track Timeline.

### Version
- Bumped to **4.8.0**.

## [4.7.0] - 2026-04-03

### Added — The "OmniMount" Release

#### 💽 OmniMount VFS (Dokany/FUSE Integration)
- **`omni_mount_interface.hpp` & `omni_mount.cpp`**: Implemented a kernel-level bridge using Dokany (Windows) and FUSE (Linux).
- Allows bobfilez Virtual File Systems (Live Folders, Semantic Searches, Cloud Drives, and Data Swarms) to be mounted as native OS drive letters (e.g., `Z:\`).
- **`OmniMountPanel.qml`**: A comprehensive UI for managing active virtual mounts, configuring drive letters, and setting Read/Write or Read-Only permissions.
- You can now run a semantic search for "Invoices from 2023", mount the result as `Z:\`, and open those scattered files natively in any third-party application like Microsoft Word or Excel.

### Version
- Bumped to **4.7.0**.

## [4.6.0] - 2026-04-03

### Added — The "OmniOracle & Time Machine" Release

#### 🤖 OmniOracle AI Copilot (Local RAG)
- **`omni_oracle_interface.hpp` & `omni_oracle.cpp`**: Implemented a local Retrieval-Augmented Generation (RAG) engine.
- Users can now chat directly with their filesystem. OmniOracle uses the `DocumentEmbedder` (BERT) to read PDFs/DOCXs, summarizes contents, and suggests automated `OmniFlow` actions.
- **`OraclePanel.qml`**: A sleek chat interface integrated into the OmniShell Taskbar, complete with markdown support, file citation chips, and actionable trigger buttons.

#### ⏳ OmniTimeMachine (Continuous Data Protection)
- **`time_machine.cpp`**: Captures binary deltas (using librsync/zstd concepts) as files are modified, allowing instantaneous rewind without duplicating full file sizes.
- **`TimeMachinePanel.qml`**: A visual timeline scrubber that lets users review modification history and restore old states.

### Version
- Bumped to **4.6.0**.

## [4.4.0] - 2026-04-03

### Added — The "Visual Automation" Release

#### 🌊 OmniFlow Visual Automation Engine
- **`omniflow_engine.cpp`**: Implemented a robust C++ execution engine for custom file workflows.
- **`OmniFlowPanel.qml`**: A stunning Node-based Visual Editor (similar to Unreal Blueprints or Apple Shortcuts).
- **Node Types**: Users can drag, drop, and connect `Triggers` (Folder Watcher, Timers), `Conditions` (Regex, AI Semantic Tags), and `Actions` (Convert, Encrypt, OCR, Move).

#### 🕸️ OmniGraph Knowledge Database
- **Semantic Mapping**: Shifted from pure folder hierarchies to a Personal Knowledge Graph for files (`omni_graph.cpp`). Files are linked via semantic relationships (`DEPICTS`, `LOCATED_IN`, `MENTIONS`).
- **`OmniGraphPanel.qml`**: Interactive force-directed graph visualization of your file relationships.

### Version
- Bumped to **4.4.0**.

## [3.0.0] - 2026-04-03

### Added — The "Next-Gen Data Custodian" Release

#### 🖥️ OmniShell: Windows 11 Shell Replacement
- **`Taskbar.qml` & `StartMenu.qml`**: Fully functional shell UI with Acrylic/Mica effects, system tray, and AI-powered recommendations.
- **`DesktopIcons.qml`**: Grid-based desktop management system.
- **Multi-Window Coordinator**: `main.qml` now manages separate process-like windows for all sub-tools.

#### ⚡ High-Performance & Security
- **Zero-Copy Sorter**: Integrated ReFS Block Cloning (Windows) and Reflink (Linux) for near-instant file duplication on the same volume.
- **Data Topology Map**: SequoiaView-style Tree-map visualization for disk usage analysis.
- **PII Sentinel**: Autonomous scanning for Social Security Numbers, Credit Cards, and API keys.
- **Forensic Audit Ledger**: Immutable, append-only operation log with tamper-detection hashes (Migration 5).

#### 🧹 Autonomous Cleanup
- **Digital Rot Agent**: AI identifying obsolete installers, temporary caches, and forgotten data.
- **Visual Discovery**: Tinder-style "swipe" interface for near-duplicate photo cleanup.

#### 🕰️ Unification & Parity
- **Nexus Master Clock**: Unified scheduler for priority-based task arbitration across all engines.
- **Java 21 Parity**: Brought the Java project up to v3.0 architectural parity with new interfaces and command features.

### Version
- Bumped to **3.0.0**.

## [2.9.0] - 2026-04-03

### Added — Vector Semantic Search & Backend Finalization

#### 🧠 Vector Semantic Search
- **`clip_search_interface.hpp` & `clip_search_engine.cpp`**: Implemented an offline natural-language-to-image search engine using OpenAI's CLIP model.
- Uses ONNX Runtime C++ API to run `clip-image` and `clip-text` models entirely locally for complete privacy.
- Parses user queries via BPE Tokenizer, converts them to 512-dimensional float vectors, and computes L2-normalized cosine similarity against cached image vectors.
- **UI Integration**: Added a new "Semantic Search" mode to `SearchPanel.qml` featuring a dynamic threshold slider and a GridView thumbnail layout for image results with relevance score badges.

#### ⚙️ Core Engines Implemented
- **`batch_rename_engine.cpp`**: Fully implemented the `apply()` methods for all 13 Batch Rename rules (Replace, Insert, Delete, Trim, Case, Number, DateTime, Metadata, Extension, Truncate, Sanitize, Hash, Transliterate, RegexGroups) using `std::regex` and string manipulation.
- **`search_engine.cpp`**: Fully implemented the core recursive folder traversal and content search loops, featuring multi-threading and PCRE2-compatible (via `<regex>`) text grepping.
- **`enhanced_fileops.cpp`**: Implemented `EnhancedCopyEngine::copy_single_enhanced()` with FastCopy-style custom buffering, TeraCopy-style checksum verification, and error-recovery callbacks.
- **`advanced_archive_manager.cpp`**: Implemented the `AdvancedArchiveManager` bindings mapped directly to the `7z` command-line executable for in-archive browsing, extraction, modification, testing, and volume splitting.
- **`hex_editor.cpp`**: Implemented the `HexBuffer` class backing the Hex Editor panel, using OS-level memory mapping (`MapViewOfFile` on Windows, `mmap` on Linux) to instantly load arbitrarily large files without RAM exhaustion. Added `DataInterpretation` parser for 30+ types.
- **`markdown_viewer.cpp`**: Implemented the `MarkdownRenderer::render()` function wrapping the fast `md4c` C API to convert Markdown AST to HTML, injecting CSS templating and external libraries (KaTeX, Mermaid, highlight.js) for the QML `WebEngineView`.
- **`conversion_engine.cpp`**: Added the `CalibreConverter` for ebooks and `GhostscriptConverter` for PDFs.
- **`FileModel`**: Implemented C++ context menu actions (`openFile`, `deleteFile`) using `QDesktopServices` and `<filesystem>`.

### Version
- Bumped to **2.8.0**.

## [2.7.0] - 2026-04-03

### Added — Enhanced File Operations (TeraCopy/FastCopy/UltraCopier/SuperCopier parity)

#### 📎 `EnhancedCopyEngine` (C++)
- **`enhanced_fileops_interface.hpp`**: `EnhancedCopyOptions` extends `CopyMoveOptions` with:
  - **FastCopy I/O tuning**: separate read/write buffer sizes (up to 512 MiB), dedicated read/write thread pools, `FILE_FLAG_NO_BUFFERING`, `FILE_FLAG_WRITE_THROUGH`, sequential hint, smart same-drive detection.
  - **TeraCopy verification**: per-file checksum after transfer (xxHash64/MD5/SHA-1/SHA-256/CRC-32), abort on mismatch.
  - **TeraCopy error handling**: `ErrorHandlerCb` callback per file error (`FileError` with type/source/dest/message), per-error decision: Skip/SkipAll/Retry/RetryAll/Overwrite/OverwriteAll/RenameAuto/RenameAll/Abort.
  - **TeraCopy post-actions**: open dest, eject drive, shutdown, sleep, logoff, play sound.
  - **TeraCopy favorites**: save source+dest pairs for 1-click operations.
  - **TeraCopy log**: per-job log file with per-file status, written to configurable directory.
  - **UltraCopier job queue**: multi-job deque, pause/resume/cancel individual jobs, drag reorder (`move_job_up`/`move_job_down`), save/load queue to disk.
  - **SuperCopier stats**: `TransferStats` with instantaneous/average/peak speed, ETA, elapsed, speed history ring-buffer (60 samples for graph).
  - **FastCopy estimate mode**: calculate total size and file count without copying.
  - **NTFS preservation**: ACL, ADS (Alternate Data Streams), compressed flag, encrypted flag.
  - **Date-based filtering**: `only_newer_than`, `only_modified_before`.
  - **Free-space check**: abort before starting if destination has insufficient space.

#### 🗃️ `AdvancedArchiveManager` (C++)
- Extends `ArchiveEngine` with **in-archive editing**:
  - `browse()`: list archive contents as virtual filesystem tree.
  - `extract_entries()`: extract specific files/folders only.
  - `add_to_archive()`: add files to existing archive at any path.
  - `delete_entries()`: remove entries (rebuilds archive).
  - `rename_entry()`: rename path inside archive.
  - `update_entry()`: replace entry with new file from disk.
  - `convert()`: convert between archive formats without full extract.
  - `test()`: verify all CRCs, return list of corrupt entries.
  - `repair_zip()`: attempt to recover corrupt ZIP archives.
  - `search_in_archives()`: find files by pattern without extracting.
  - `benchmark()`: compare compression ratio/speed across all algorithms.
  - `info()`: detailed archive metadata (format, ratio, solid, multivolume).
  - `create_multivolume()`: split archives across volumes.
  - `merge_volumes()`: reassemble split archives.

#### 🎨 `EnhancedFileOpsPanel.qml`
- **Copy/Move Tab** (TeraCopy+FastCopy+UltraCopier+SuperCopier combined):
  - Job queue panel: add multiple jobs, drag-reorder, pause/resume/cancel individual jobs, global pause/cancel.
  - Favorites panel: quick-add source+dest pairs.
  - Per-file list with status tabs (All / OK / Skip / Error / Pending), right-click error recovery menu (Retry/Skip/SkipAll/Overwrite/Rename).
  - Real-time stats display: current speed, peak speed, average speed, ETA, files done, data done, elapsed, failed.
  - **Live speed graph** (SuperCopier-style): Canvas-drawn 60-second bandwidth chart with fill gradient.
  - Per-job mini progress bars in queue.
  - Options panel: error handling, verification algorithm, I/O tuning (buffer sizes, threads, no-cache, write-through, smart mode), NTFS options, file filters, post-action.
- **Archive Tab** (7-Zip parity + in-archive browser/editor):
  - Create archive: format, method, level, dictionary, word size, threads, solid mode, SFX, AES-256 encryption with filename encryption, volume splitting, comment, delete-after.
  - Operations panel: Test Integrity, Repair ZIP, Convert Format, Benchmark, Merge Volumes.
  - **In-archive browser**: breadcrumb navigation inside archive, hierarchical file tree with Name/Size/Packed/Ratio/Modified/CRC/Method columns.
  - Right-click context menu: Extract to.../Extract here/Open/Rename/Delete/Replace with file/Copy path/Properties.
  - Add files to existing archive, delete entries, rename entries, replace entries — all via UI.
  - Extract controls: destination, keep structure, overwrite, Extract All/Extract Selection.
  - Progress bar during all archive operations.

### Version
- Bumped to **2.7.0**.



### Added — Comprehensive File Management Suite

#### 🔄 File Format Conversion Engine
- **`conversion_interface.hpp`**: Universal converter interface with `IConverter` abstract base, `ConversionEngine` orchestrator, and `ConversionResult` tracking.
- **`conversion_engine.cpp`**: Three fully-wired converter backends:
  - **FFmpegConverter**: Video (mp4/mkv/avi/webm/mov/gif/flv) ↔ all codecs (H.264/H.265/VP9/AV1), audio (mp3/flac/aac/ogg/opus/wav), with CRF, bitrate, preset, resolution, trim, and thread options.
  - **ImageMagickConverter**: 200+ image formats (png/jpg/webp/avif/heic/tiff/bmp/ico/psd/raw/jp2/jxl/exr) with quality, resize, colorspace, EXIF-strip, PDF page extraction.
  - **PandocConverter**: Documents (md/rst/html/docx/odt/epub/latex/pdf) with TOC, templates, CSS, numbered headings, PDF engine choice.
- **`gui/panels/BatchConvertPanel.qml`**: Full-featured batch conversion UI with source file list, output format picker, per-backend options panels (video/image/document), output directory picker, collision policy, thread slider, live progress, error log.
- **New submodules**: `pandoc`, `ghostpdl`, `poppler`, `LibRaw`, `libde265`, `calibre`, `wkhtmltopdf`, `Magick.NET`, `pcre2`, `re2`, `ripgrep`, `the_silver_searcher`, `sigil`, `md4c`, `cmark`, `highlightjs`, `rapidjson`.

#### 📝 Batch File Renaming Engine
- **`batch_rename_interface.hpp`**: Complete rename rule system with 13 rule types: Replace, Insert, Delete, Trim, Case (9 modes), Number (counter with padding), DateTime, EXIF/Metadata, Extension, Truncate, Sanitize, Hash, Transliterate, RegexGroups.
- **`BatchRenameEngine`**: Chain-based rule application, preview generation with conflict detection, execute with undo support, preset save/load.
- **`gui/panels/BatchRenamePanel.qml`**: Split-pane UI — left rule chain builder with per-rule configuration panels, right live preview table (original→new with conflict coloring). Built-in preset library.

#### 🔍 Search & Find-in-Files Engine
- **`search_interface.hpp`**: Comprehensive `SearchOptions` covering Everything+AgentRansack+grepWin+dnGrep feature sets: filename/content modes, PCRE2/wildcard/fuzzy match, size/date/attribute filters, `.gitignore` awareness, archive search, multi-threaded parallel search, find-and-replace in files with backup/undo.
- **`gui/panels/SearchPanel.qml`**: Three-mode tabbed UI (Filename Search / Content Search / Find & Replace), all filter options, results list with expandable content match lines, sort/group controls, export CSV.

#### 📦 Batch File Operations
- **`fileops_interface.hpp`**: Five operation engines:
  - **FileCopyMoveEngine**: TeraCopy-style copy/move with verify (xxHash/MD5/SHA-256), collision policy, throttle, retry, hardlink/symlink support.
  - **FolderSyncEngine**: FreeFileSync-style mirror/update/two-way/custom sync with versioning, sync database, real-time watch, delta detection.
  - **FileDiffEngine**: WinMerge-style file/folder diff (Myers/Patience/Histogram algorithms), side-by-side or unified output, HTML export.
  - **BackupEngine**: Areca/Duplicati-style full/incremental/differential backup with Zstd/LZ4/Brotli/7z compression, AES-256/ChaCha20 encryption, volume splitting, rsync delta.
  - **ArchiveEngine**: 7-Zip-style create/extract for zip/7z/tar.*/brotli/lz4/zstd with solid mode, SFX, volume split, password.
- **`gui/panels/FileOpsPanel.qml`**: Five-tab comprehensive panel (Copy/Move / Sync / Diff / Backup / Archive).
- **New submodules**: `libarchive`, `librsync`, `zstd`, `lz4`, `brotli`, `dtl-diff`, `p7zip`, `7zip`.

#### 🔬 Hex Editor
- **`hex_editor_interface.hpp`**: Full `HexBuffer` with mmap-backed sparse overlay (edit any file without full copy), undo/redo, insert/delete bytes, PCRE2/hex-pattern/ASCII/UTF-16 search with `??` wildcards.
- **`DataInterpretation`**: Interprets selection as int8/16/32/64 (LE+BE), uint variants, float32/64, ASCII, UTF-8, UTF-16, GUID, Windows FILETIME, Unix timestamp, CRC-32/CRC-16, binary, octal.
- **`gui/panels/HexEditorPanel.qml`**: Full hex view with offset column, hex bytes, ASCII panel, configurable columns (8/16/32), Data Inspector sidebar, search bar with mode selector, Go-to-offset, keyboard navigation.

#### 🖼 Image Viewer
- **`image_viewer_interface.hpp`**: XnViewMP-inspired — ThumbnailCache (LRU disk cache), ImageInfo with full EXIF/IPTC/XMP, ImageHistogram (per-channel), ColorAdjustments (brightness/contrast/saturation/hue/exposure/highlights/shadows/sharpness/temperature/vibrance/clarity), SlideshowOptions with transitions, lossless JPEG rotate/flip via exiftran.
- **`gui/panels/ImageViewerPanel.qml`**: Full XnViewMP-style viewer — filmstrip, zoom controls, rotate/flip toolbar, slideshow with interval, Histogram panel, Color Adjustments panel, EXIF Metadata panel, multi-page support (TIFF/GIF/PDF), keyboard navigation.

#### 📖 Markdown Viewer
- **`markdown_viewer_interface.hpp`**: `MarkdownRenderer` using md4c + cmark — CommonMark + GFM tables/task-lists/autolinks/strikethrough, math (KaTeX), Mermaid diagrams, highlight.js syntax highlighting, footnotes, wikilinks, emoji, YAML frontmatter, word count, reading time. Export: HTML, PDF, DOCX via Pandoc.
- **`gui/panels/MarkdownViewerPanel.qml`**: Split-pane editor+preview with WebEngineView, TOC sidebar, theme switcher (Light/Dark/Sepia), all extension toggles, font/width controls, export menu.

### Version
- Bumped to **2.6.0**.



### Added
- **Advanced Media Analysis Engine**:
    - **Video Perceptual Hashing (`vhash`)**: Implemented an FFmpeg-backed video analyzer that extracts evenly-spaced frames, converts them to grayscale thumbnails, and generates 64-bit dHash values to identify visually similar or duplicate video files regardless of resolution, codec, or frame rate.
    - **Audio Fingerprinting (`afingerprint`)**: Implemented an FFmpeg + Chromaprint integration to generate AcoustID-compatible audio fingerprints. Capable of identifying identical audio content across differing formats (mp3 vs flac), bitrates, or volume levels.
    - New CLI commands: `fo_cli vhash <file>`, `fo_cli afingerprint <file>`, `--list-vhash`, `--list-ahash`.
- **vcpkg Dependencies**: Added `ffmpeg` and `chromaprint` to `vcpkg.json` for media analysis support.
- **Documentation**: Substantial updates to the project's vision, architecture, and roadmap based on deep-dive codebase analysis.

## [2.4.3] - 2026-04-02

### Added
- **OmniUI Explorer Enhancements**:
    - Windows 11 style tabbed title bar.
    - Functional address bar and search field skeletons.
    - Detailed file list view with Name, Size, Type, and Date Modified.
    - Navigation sidebar with "This PC", "OneDrive", etc.
    - Status bar showing item and selection counts.
- **Documentation**:
    - `docs/COMPETITIVE_ANALYSIS_DETAILED.md`: Analysis of Files, File Pilot, and OpenSalamander.
    - `EXPLORER_PLAN.md`: Detailed implementation roadmap for the shell replacement, including a new "Integrated Image Management Suite" category and an "Advanced Dual-Pane Operations (Salamander Parity)" category.

### Improved
- mica-style visual effects and drop shadows for the explorer window.
- Updated `HANDOFF.md` for session transition.
