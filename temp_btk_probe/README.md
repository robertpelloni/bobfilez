## BTK

### Introduction

BTK is a cross-platform C++ application framework descended from the CopperSpice/Qt lineage and being actively reworked into a broader systems foundation for large-scale software and operating-system development.

The immediate direction of this repository is:

* rebrand the project around BTK
* preserve the mature C++ framework core already present here
* study and systematically assimilate proven ideas from Qt 5/6, JUCE, Ultimate++, JavaFX, Dear ImGui, and BobUI
* evolve the architecture toward first-class multi-user, multi-focus, and collaborative interaction models

BTK is intended to become a foundational runtime for a new OS/application stack where multiple local and remote users can interact with software concurrently.

### Current Framework Surface

The repository currently contains substantial implementations for:

* BtkCore
* BtkGui
* BtkMultimedia
* BtkNetwork
* BtkOpenGL
* BtkSql
* BtkSvg
* BtkVulkan
* BtkWebKit
* BtkXml
* BtkXmlPatterns

It also vendors or integrates several supporting libraries under `src/annex/` and `src/3rdparty/`.

### Reference Submodules

The following repositories are now tracked as reference inputs for ongoing assimilation work:

* `external/juce` - JUCE upstream
* `external/ultimatepp` - Ultimate++ upstream
* `external/bobui-reference` - BobUI reference baseline

BobUI is included for reference analysis only. JUCE and Ultimate++ are being used as structured comparison sources for feature planning, architectural analysis, and future integration work.

### Build Notes

BTK continues to use CMake as its primary build system. This session adds BTK-branded package metadata and a `find_package(BTK)` entry point while preserving existing CopperSpice compatibility exports.

### Research and Planning

Initial assimilation research, comparison matrices, and migration planning live under:

* `docs/ai/requirements/`
* `docs/ai/design/`
* `docs/ai/planning/`
* `docs/ai/implementation/`
* `docs/ai/testing/`

### Status

This repository is in a transition phase from a CopperSpice-derived framework toward a BTK-branded platform with broader scope. The codebase already contains mature subsystems, but the larger parity goals with Qt 6+, JUCE, Ultimate++, JavaFX, Dear ImGui, and BobUI are a multi-phase effort and are being approached systematically.
