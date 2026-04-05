# BTK Provider Probe Round 4 — Experimental Declarative Enablement Confirms Stale Module and Missing QtScript Dependency

## Summary
This session pushed the BTK investigation one level deeper than Round 3.

Round 3 established that:
- BTK now builds successfully on this MSVC host for the currently enabled module set
- bobfilez's native GUI still cannot configure because the active BTK package surface does not export `Declarative`

Round 4 asked the next honest question:

> What happens if `Declarative` is experimentally re-enabled in BTK's top-level build?

The answer is important:
- the current BTK branch does **not** merely omit `Declarative` from the top-level component list
- the `src/declarative` module itself is also **stale and incomplete** relative to the active BTK/CopperSpice build system
- after minimal CMake modernization just to get configuration moving, the experimental build fails in the declarative code on:
  - obsolete declarative-era metatype declarations (`Q_DECLARE_METATYPE` diagnostics)
  - and, more decisively, a missing QtScript-era dependency:
    - `QtScript/qscriptvalue.h`

This means the blocker is stronger than a missing list entry.
The current BTK line does not yet provide a viable path to the QML/Declarative stack bobfilez's `QQmlApplicationEngine`-based GUI requires.

## Why this session was necessary
Round 3 showed that the top-level BTK build excludes `Declarative` by omitting it from:
- `CS_OPTIONAL_COMPONENTS`

That left two possibilities:
1. `Declarative` was accidentally omitted but otherwise still basically works
2. `Declarative` was omitted because the module is not actually build-ready in the current BTK line

Round 4 was designed to distinguish those cases.

## Starting state
Before the experiment:
- BTK normal module set builds successfully on this host via `scripts/build_btk_inplace.bat`
- bobfilez's BTK-backed GUI probe still fails at:
  - missing component/target: `Declarative`
- bobfilez's native entrypoint remains genuinely QML-based:
  - `gui/omni/src/main.cpp` uses `QGuiApplication`
  - `QQmlApplicationEngine`
  - `qmlRegisterType(...)`

So the missing Declarative/QML capability is still a hard blocker, not an optional enhancement.

## Experimental BTK changes
The experiment intentionally made only the smallest changes needed to probe deeper.

### 1. Temporarily re-enabled Declarative in the BTK top-level component list
Temporary change in `libs/btk/CMakeLists.txt`:
- added `Declarative` to `CS_OPTIONAL_COMPONENTS`

This allowed the top-level BTK build to attempt `add_subdirectory(src/declarative)`.

### 2. Fixed the first stale Declarative CMake command
The first configure failure was:
- unknown CMake command `target_add_definitions`

Temporary change in `libs/btk/src/declarative/CMakeLists.txt`:
- replaced the obsolete helper with modern `target_compile_definitions(...)`

### 3. Fixed a second stale Declarative build-system mismatch
The next configure failure came from:
- `function_generate_resources("${DECLARATIVE_SOURCES}")`

The active BTK macro expects a **target name**, not a source list.

Temporary modernization in `libs/btk/src/declarative/CMakeLists.txt`:
- added `target_sources(CsDeclarative ...)`
- normalized compile/link flag handling to the same general shape used by live BTK modules
- changed resource generation to:
  - `function_generate_resources(CsDeclarative)`

These changes were intentionally treated as an experiment, not as a production-ready upstream declarative revival.

## Result of the experiment
After the minimal CMake modernization above, BTK configuration succeeded with `Declarative` enabled and the build advanced into actual declarative compilation.

That exposed the more meaningful failure boundary.

### Observed compile failures
The experimental build failed in declarative compilation with errors including:

- obsolete metatype macro diagnostics:
  - `Q_DECLARE_METATYPE(TYPE) is obsolete, use CS_DECLARE_METATYPE(TYPE)`
- and a fatal missing include:
  - `QtScript/qscriptvalue.h: No such file or directory`

Observed failure path included:
- `src/declarative/graphicsitems/qdeclarativerepeater.cpp`
- `src/declarative/qml/qdeclarativelist.h`
- generated / public declarative headers such as `qdeclarativecomponent.h`

## Additional structural evidence gathered
Independent inspection of the BTK source tree shows:
- `src/declarative/` is large and real
- it contains many `QDeclarative*` sources
- it depends broadly on old QtScript-era APIs such as:
  - `QScriptEngine`
  - `QScriptValue`
  - `QScriptContext`
  - `QScriptEngineAgent`
  - headers under `<QtScript/...>`

However, the current BTK tree does **not** appear to provide a real `QtScript` module/header surface in the expected form.

Examples:
- no obvious `src/script/` module exists in the current BTK top-level component graph
- no matching `QtScript/qscriptvalue.h` header surface is present for the active build
- `src/declarative/CMakeLists.txt` still advertises package dependencies including:
  - `CsScript`

This is strong evidence that the current `Declarative` directory is not aligned with the rest of the active BTK branch.

## Important correction made during this session
While performing the declarative experiment on top of the newer upstream BTK tip, a regression in `src/gui/kernel/qapplication_cs.cpp` reappeared around property-name handling.

Specifically, newer upstream BTK code had drifted back to invalid calls like:
- `QString::fromLatin1(btkOwnerPropertyName)`
- where `btkOwnerPropertyName` was already a `QString`

That broke the normal BTK MSVC build again.

A real BTK-side fix was applied and preserved:
- restored direct `QString` property/setProperty usage in `src/gui/kernel/qapplication_cs.cpp`

After reverting the temporary declarative experiment changes, BTK returned to a successful normal-module MSVC build on this host.

The corrected BTK commit for the active branch was pushed to:
- branch: `pi/msvc-focus-fixes-20260405`

## Practical conclusion
Round 4 upgrades the conclusion from Round 3.

### Round 3 conclusion
- BTK currently does not export `Declarative`
- bobfilez therefore cannot resolve the QML/Declarative layer it needs

### Round 4 conclusion
- BTK currently does not export `Declarative`
- and when `Declarative` is experimentally re-enabled, the module is still not viable in the current BTK line because it immediately hits:
  - stale declarative-specific CMake integration
  - obsolete declarative metatype usage
  - and missing QtScript-era headers / module surface

So the active blocker is **not just omission**.
It is a deeper framework-readiness gap.

## Implication for bobfilez
This matters because bobfilez's current native GUI path is not a generic widget-only GUI.
It is explicitly QML-based:
- `QQmlApplicationEngine`
- QML assets under `gui/omni/assets/`
- local `qmlRegisterType(...)` bridges

Therefore, the current BTK line is still not an honest drop-in native provider for bobfilez's active GUI architecture.

## Recommended next steps
1. **Do not classify the current problem as a simple top-level component omission**
   - Round 4 proved the issue runs deeper than `CS_OPTIONAL_COMPONENTS`

2. **Treat BTK Declarative revival as a substantial upstream project**
   - it likely requires:
     - additional CMake modernization
     - declarative header/source modernization
     - restoration or replacement of the old QtScript/CsScript dependency chain

3. **Keep bobfilez's BTK consumer boundary explicit and honest**
   - the current failure should remain framed as:
     - missing required Declarative/QML capability in the active BTK provider

4. **Evaluate strategic options explicitly**
   - either:
     - invest in upstream BTK Declarative/QtScript revival work
     - reduce bobfilez's dependency on QML/QQmlApplicationEngine over time
     - or revisit framework assumptions if full QML capability remains mandatory
