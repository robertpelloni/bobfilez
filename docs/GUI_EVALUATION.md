# GUI Library Evaluation

This document evaluates potential GUI frameworks for the filez project, considering the existing C++ codebase, performance requirements, and cross-platform goals.

## Requirements

1.  **C++ Integration**: The core engine (`fo_core`) is written in C++20. The GUI must interface with it efficiently.
2.  **Performance**: Must handle lists of 100,000+ files and render thumbnails without UI freezing.
3.  **Cross-Platform**: Windows, Linux, macOS.
4.  **Maintainability**: Should align with the project's goal of robustness and longevity.
5.  **Legacy Compatibility**: The project has an existing Qt codebase (`Openfilez.cpp`).

## Candidates

### 1. Qt (Widgets / QML)

*   **Language**: C++ (Widgets) or QML/JavaScript (Quick).
*   **License**: LGPLv3 / GPLv3 / Commercial.
*   **Pros**:
    *   **Native C++**: Seamless integration with `fo_core`.
    *   **Mature**: Industry standard for C++ desktop apps.
    *   **Virtual Scrolling**: `QListView`/`QTableView` handle millions of rows efficiently.
    *   **Existing Code**: The legacy `Openfilez.cpp` is already written in Qt Widgets.
    *   **Tools**: Qt Creator and Designer are excellent.
*   **Cons**:
    *   **Binary Size**: Large DLLs/shared libraries.
    *   **Complexity**: MOC (Meta-Object Compiler) adds build steps.
    *   **Licensing**: Must be careful with static linking (LGPL requirements).

### 2. Electron

*   **Language**: JavaScript/TypeScript (Frontend), Node.js (Backend).
*   **License**: MIT.
*   **Pros**:
    *   **Modern UI**: Easy to create beautiful, responsive layouts with CSS/React/Vue.
    *   **Ecosystem**: Access to npm packages.
    *   **Developer Pool**: Easier to find web developers than C++ Qt developers.
*   **Cons**:
    *   **Resource Usage**: High RAM consumption (Chromium instance).
    *   **Integration**: Requires an IPC bridge (Node.js C++ Addons or spawning `fo_cli` as a subprocess).
    *   **Performance**: Handling 100k items in the DOM requires careful virtualization (e.g., `react-window`).

### 3. wxWidgets

*   **Language**: C++.
*   **License**: wxWindows Library Licence (permissive).
*   **Pros**:
    *   **Native Look**: Uses the OS's native API (Win32, GTK, Cocoa).
    *   **Lightweight**: Smaller footprint than Qt.
*   **Cons**:
    *   **DX**: API feels dated (MFC-like).
    *   **Styling**: Harder to customize than Qt or Web.

### 4. Dear ImGui

*   **Language**: C++.
*   **License**: MIT.
*   **Pros**:
    *   **Performance**: Immediate mode, zero overhead state management.
    *   **Integration**: Trivial to link with C++.
*   **Cons**:
    *   **Non-Standard**: Looks like a game engine tool, not a native app.
    *   **Power Usage**: Constant redraw loop (unless carefully managed) drains battery.
    *   **Accessibility**: Poor support for screen readers.

### 5. Flutter

*   **Language**: Dart.
*   **License**: BSD-3-Clause.
*   **Pros**:
    *   **Performance**: Skia/Impeller rendering engine is fast.
    *   **Cross-Platform**: Single codebase for Mobile/Desktop/Web.
*   **Cons**:
    *   **Language Barrier**: Introduces Dart to a C++ project.
    *   **FFI**: Interop with C++ requires `dart:ffi` boilerplate.

## Recommendation: Qt (Qt Widgets)

**Verdict**: **Qt** is the best fit for filez.

**Reasoning**:
1.  **Inertia**: The project already has a significant amount of Qt code (`Openfilez.cpp`, `.ui` files). Porting this to a clean architecture (Model-View-Controller) is easier than rewriting the UI layer in a new language.
2.  **Performance**: Qt's `QAbstractItemModel` is specifically designed for the "millions of items" use case we have (file lists).
3.  **Integration**: We can link `fo_core` directly as a static library. No IPC overhead.
4.  **Build System**: CMake has first-class support for Qt (`find_package(Qt6)`).

### Implementation Plan

1.  **Decouple**: Continue extracting logic from `Openfilez.cpp` into `fo_core`.
2.  **Scaffold**: Create a new `gui/` subdirectory with a modern CMake setup.
3.  **Refactor**: Move the legacy `Openfilez` class into the new structure, stripping out the direct SQLite/Worker logic and replacing it with calls to `fo::core::Engine`.
4.  **Modernize**: Eventually migrate from `.ui` files to hand-coded layouts or QML if a custom look is desired, but Widgets are fine for a utility app.

## Alternative: Electron (Long Term)

If the project aims for a "cloud-native" or "remote-access" future (e.g., running the engine on a NAS and viewing on a laptop), a Web frontend (React) talking to a REST/gRPC API provided by `fo_cli` would be superior. However, for a standalone desktop utility, Qt remains the robust choice.
