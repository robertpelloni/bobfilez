# BTK Bootstrap Validation

## Validation Scope
This session changed repository structure, CMake packaging metadata, and documentation. It did not attempt a full feature merge from BobUI/JUCE/U++.

## Validation Steps
- Confirm the new submodules exist and are tracked by `.gitmodules`.
- Attempt a low-surface CMake configure to validate the new BTK package/config generation path.
- Inspect generated git state for expected file additions.

## Validation Result
- Submodule registration succeeded.
- CMake configure was attempted with most optional components disabled, but the local environment does not currently expose a usable C/C++ compiler to CMake, so configure stopped before full package-generation validation.
- `git diff --check` reported only CRLF normalization warnings, not patch-content errors.

## Risks
- BTK package aliasing must not break existing CopperSpice package exports.
- A future broad `Cs*` rename can easily introduce ABI/API and include-path regressions.
- Existing optional modules still require deeper audit before enabling parity work.

## Additional Verified Checks
- `python scripts/generate_btk_api_inventory.py` completed successfully and generated both Markdown and JSON inventory artifacts.
- A downstream smoke example was prepared that consumes `<QtCore/BTKCore>` and uses `BtkString` plus `BtkSharedPointer` aliases.

## Required Follow-Up Tests
- `cmake -S . -B build-min -DWITH_GUI=OFF -DWITH_MULTIMEDIA=OFF -DWITH_NETWORK=OFF -DWITH_OPENGL=OFF -DWITH_SQL=OFF -DWITH_SVG=OFF -DWITH_VULKAN=OFF -DWITH_WEBKIT=OFF -DWITH_XMLPATTERNS=OFF`
- Verify `find_package(BTK)` from the sample at `docs/ai/testing/btk-package-smoke-example/` using both `BTK::Core` and `BTK::BtkCore` target forms.
- Verify `BTK_RESOURCES(...)`, `btk_copy_library(...)`, and `btk_copy_plugins(...)` in a downstream package test.
- Add CI coverage for BTK package config generation and alias validation.
