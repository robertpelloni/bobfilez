# BobUI Qt Runtime Reality Check — 2026-04-05

## Summary
After restoring BobUI as the active native direction for bobfilez, this follow-up session tightened the technical picture around what BobUI is and is not on this machine.

The main conclusion is:

- **BobUI remains the desired native UI layer for bobfilez**, especially through `OmniUI/omnicore`
- but **BobUI in this repository is not, by itself, a full MSVC-ready Qt6 QML runtime provider**
- bobfilez's current native shell still needs a compatible **Qt6 Qml / Quick** runtime from a matching toolchain family

This session moved the project from a vague "missing Qt6Qml" diagnosis to a more precise and operational one.

## Key findings

### 1. The earlier BobUI corelib blocker was real and is now partially fixed
A small BobUI-side patch in:
- `libs/bobui/src/corelib/kernel/qtmochelpers.h`

fixed the earlier MSVC failure where BobUI stopped in:
- `qlocale.cpp`
- `qtmochelpers.h`
- `'result': undeclared identifier`

The change was intentionally narrow:
- introduced a local `result_ref` alias
- introduced a named `InputMetaTypes` alias inside the generic lambda
- preserved behavior while avoiding the MSVC lookup failure inside the templated lambda context

This was committed and pushed in the BobUI submodule history.

### 2. BobUI's in-place build now gets much farther on this host
After the `qtmochelpers.h` fix, the in-place BobUI build no longer dies at the earlier `QLocale` boundary.

It now progresses substantially farther into:
- `Gui`
- `Widgets`
- `OpenGL`
- `PrintSupport`
- `Test`

This proves the first blocker was not imaginary and that the patch materially improves BobUI's MSVC viability.

### 3. BobUI's current remaining in-place build stop is now different
The current in-place build stop is no longer the original `QLocale` failure.

The new visible boundary is an MSVC internal compiler error while building GUI documentation snippet content:
- `src/gui/doc/snippets/...`
- ultimately triggered in `src/corelib/itemmodels/qrangemodel_impl.h`
- compiler error: `fatal error C1001: Internal compiler error`

This is important because it changes the character of the problem:
- the original blocker was a BobUI source issue in the metaobject helper path
- the current blocker is now a **toolchain/compiler stability issue** in ancillary snippet targets

### 4. BobUI's own CI expects external Qt modules
BobUI's workflow file:
- `libs/bobui/.github/workflows/ci_cd.yml`

shows that BobUI CI installs external Qt modules such as:
- `qtdeclarative`
- `qtwebsockets`
- `qt5compat`
- `qtshadertools`
- `qtmultimedia`
- `qtsvg`

before building OmniUI examples/tests.

That is a crucial architecture clue.

It means the BobUI repo here should **not** be interpreted as a self-contained full Qt superbuild including all QML/Quick modules. Instead, the repo is closer to:
- a Qt-base-style core/framework tree
- plus BobUI's own `OmniUI` layer
- with some higher-level Qt modules expected to come from a separate Qt installation / module set

### 5. The local BobUI source tree does not include qtdeclarative in-tree
The local BobUI tree contains the expected qtbase-style directories under `src/`, but no in-tree `qtdeclarative` checkout.

At the same time, BobUI's `OmniUI/omnicore` clearly uses modern Qt QML/Quick APIs such as:
- `QQmlApplicationEngine`
- `QQmlEngine`
- `QQuickItem`
- `qmlRegisterType(...)`

So BobUI's Omni layer is QML/Quick-oriented, but the local BobUI tree alone is not enough to supply all those runtime modules as a self-contained package surface.

### 6. The host does have Qt6 QML packages — but not for the active MSVC lane
The user provided an installed Qt location at:
- `D:\Qt`

Inspection showed real Qt6 QML package configs at:
- `D:\Qt\6.11.0\mingw_64\lib\cmake\Qt6Qml\Qt6QmlConfig.cmake`
- plus Android kits

However, the visible desktop kit on this host is currently:
- **MinGW** (`mingw_64`)

and not an MSVC desktop kit.

bobfilez's current validated native build workflow is still MSVC-based.

So the blocker is now more precise than before:
- **the machine does have a Qt6 QML runtime**
- but **it is not an MSVC-matching Qt6 desktop kit**
- and the local BobUI tree still does not self-provide `Qt6Qml`

## Practical conclusion
The correct active mental model is now:

- **BobUI is the intended native UI layer**
- **OmniUI/omnicore remains worth integrating**
- bobfilez should still build against normal `Qt6::*` targets
- but the needed **Qt6 Qml / Quick runtime must come from a matching external Qt installation unless BobUI grows that module surface itself**

On this host specifically, the remaining native-GUI blocker is now:

1. local BobUI build tree does not export `Qt6Qml`
2. available external desktop Qt6 QML kit is currently **MinGW**, not **MSVC**
3. BobUI in-place MSVC build is improved but still eventually hits an MSVC internal compiler error in snippet-related targets

## Changes made in bobfilez to reflect this
### Build helper / discovery changes
Updated:
- `cmake/BobUIQtSetup.cmake`
- `scripts/build_bobui_gui.bat`
- `scripts/build_bobui_inplace.bat`
- `gui/CMakeLists.txt`
- `gui/omni/CMakeLists.txt`

Refinement:
- BobUI discovery now also acknowledges external Qt roots via:
  - `QT6_ROOT`
  - `QT_ROOT`
  - `QTDIR`
- messaging now reflects **BobUI Omni wiring + Qt6 package discovery**, not the earlier overconfident assumption that the local BobUI tree is always the full provider
- the BobUI in-place helper now explicitly disables tests/examples/benchmarks/doc-snippet intent at configure time to reduce noise from non-essential build parts

## Recommended next steps
1. **Obtain or install an MSVC-compatible Qt6 desktop kit with Qml/Quick**
   - for example an MSVC Qt 6.x desktop install that exports:
     - `Qt6Config.cmake`
     - `Qt6QmlConfig.cmake`
     - `Qt6QuickConfig.cmake`
     - `Qt6QuickControls2Config.cmake`

2. **Keep BobUI as the active Omni/UI layer rather than reverting to BTK again**
   - the BobUI direction remains strategically aligned with bobfilez's shell architecture
   - the missing piece is now runtime/toolchain alignment, not app architecture mismatch

3. **Treat local BobUI build improvement as worthwhile upstream progress**
   - the `qtmochelpers.h` patch should be kept because it removes a real MSVC source-level failure
   - if needed later, continue reducing the remaining in-place build noise around doc-snippet/compiler-ICE territory

4. **Only after an MSVC-compatible Qt6 QML kit is available, re-run the BobUI-backed bobfilez GUI probe**
   - at that point the next failures, if any, should be much more honest and integration-specific

## Bottom line
This session improved the BobUI story in two ways at once:
- it fixed a real BobUI-side MSVC source failure
- it clarified that the current blocker is no longer a vague "BobUI is broken" claim, but a specific combination of:
  - incomplete local BobUI QML package surface
  - external Qt module expectations
  - and current host toolchain mismatch (MinGW Qt available, MSVC Qt not yet available)
