# BTK Provider Probe Round 5 — BTK Declarative is a QDeclarative/Qt Quick 1 Era Surface, Not a QQml/QQuick Provider

## Summary
This session answered the next architectural question left open by Round 4:

> Even if BTK's Declarative stack were revived, would it actually match bobfilez's current native GUI API expectations?

The answer is now clearly **no, not directly**.

Round 5 confirms that the BTK tree currently exposes an older declarative stack centered on:
- `QDeclarativeEngine`
- `QDeclarativeComponent`
- `QDeclarativeView`
- `QDeclarativeItem`

At the same time, the BTK tree appears to provide **no actual `QQml*` or `QQuick*` API surface at all**:
- no `QQmlApplicationEngine`
- no `QQmlEngine`
- no `QQmlContext`
- no `QQuickItem`
- no `QQuickView`
- no `qqml*.h` headers
- no `qquick*.h` headers

This matters because bobfilez's active native GUI bootstrap is written against the newer QML API family:
- `QGuiApplication`
- `QQmlApplicationEngine`
- local `qmlRegisterType(...)`
- QML asset loading through the `QQml*` style engine path

So the mismatch is now proven to be deeper than:
- missing package exports
- missing top-level component enablement
- stale declarative-specific CMake
- missing QtScript-era support

It is also a **framework-generation mismatch**.

## Why this probe was necessary
Round 4 proved that BTK's `Declarative` path is stale/incomplete and tightly coupled to missing QtScript-era infrastructure.

However, there was still one more important risk of misinterpretation:
- perhaps reviving the BTK declarative module would still give bobfilez what it needs in the end

Round 5 was designed to verify whether the two sides even target the same declarative API generation.

## Evidence gathered

### 1. bobfilez's active GUI bootstrap is modern QQml-style
Inspected:
- `gui/omni/src/main.cpp`

Observed usage:
- `#include <QGuiApplication>`
- `#include <QQmlApplicationEngine>`
- `#include <QQmlContext>`
- `qmlRegisterType<...>(...)`
- `QQmlApplicationEngine engine;`

This establishes that bobfilez's current native GUI path expects a **QQml-family** runtime surface.

## 2. BTK declarative headers are explicitly QDeclarative-era
Inspected:
- `libs/btk/src/declarative/qml/qdeclarativeengine.h`

Observed:
- `class QDeclarativeEngine : public QObject`
- includes `QtScript/qscriptvalue.h`
- uses `QScriptEngine` and related QtScript-era types directly

This is not a QQml engine surface.

### 3. BTK tree contains no QQml/QQuick headers or class references
Searches over the BTK source tree found:
- **no `QQml*` class usage or includes**
- **no `QQuick*` class usage or includes**
- **no `qqml*.h` headers**
- **no `qquick*.h` headers**

This is strong negative evidence that the active BTK branch is not exposing the newer QML engine stack bobfilez currently uses.

### 4. BTK declarative tree remains heavily QtScript-coupled
A structural scan over `libs/btk/src/declarative` found:
- **61 files** with direct QtScript-related usage
- **24 files** referencing `QScriptDeclarativeClass`
- **23 files** still using `Q_DECLARE_METATYPE(...)`

Additional critical finding:
- files like `qscriptdeclarativeclass_p.h` are referenced from the declarative code, but the corresponding file does not appear to exist in the active BTK tree
- no `qscriptengine.h` header surface was found in the active BTK tree either

This reinforces the Round 4 conclusion that BTK declarative is not just disabled — it is internally incomplete relative to its own historical assumptions.

## Interpretation
The current BTK situation is now best described as three stacked gaps:

### Gap 1 — Package/component exposure gap
- BTK currently does not export `Declarative`
- so bobfilez cannot resolve the target it asks for today

### Gap 2 — Module build-readiness gap
- experimentally enabling `Declarative` exposes stale CMake and missing QtScript-era dependencies
- so the module is not currently buildable in the active branch

### Gap 3 — API-generation mismatch
- even a revived BTK declarative stack would still expose `QDeclarative*` era APIs
- bobfilez currently expects `QQml*` / modern QML engine APIs

That third point is new and important.

## Practical consequence for bobfilez
This means BTK is currently **not an honest drop-in replacement** for bobfilez's active native GUI architecture.

The gap is no longer just:
- “make BTK build”
- or “turn on Declarative”

It is now:
- **either port BTK declarative forward toward a `QQml*`/`QQuick*`-class provider surface**
- **or adapt bobfilez away from its current `QQmlApplicationEngine`-based architecture**
- **or pick a different framework direction for the native QML-style shell path**

## Recommended next steps
1. **Stop assuming a revived BTK Declarative module would automatically satisfy bobfilez**
   - Round 5 proves the API-generation mismatch directly

2. **If staying on BTK, explicitly define the required compatibility target**
   - decide whether the goal is:
     - legacy `QDeclarative*` compatibility only
     - or a real `QQml*` / `QQuick*`-class provider surface

3. **If staying on bobfilez's current GUI architecture, acknowledge the porting scope honestly**
   - this is likely no longer a small provider retarget
   - it is a framework adaptation project

4. **Use the quantified dependency footprint to estimate effort**
   - 61 QtScript-tied declarative files
   - 24 `QScriptDeclarativeClass`-tied files
   - 23 obsolete metatype macro sites
   - zero discovered `QQml*` / `QQuick*` source surface

## Bottom line
Round 5 upgrades the working conclusion again:

BTK's current native-provider story for bobfilez is blocked not only by missing exports and stale declarative internals, but also by a direct mismatch between:
- BTK's apparent **QDeclarative/Qt Quick 1-era** declarative model
- and bobfilez's current **QQml/modern QML-engine** native bootstrap

That makes the next step a strategic framework decision, not just another local build fix.
