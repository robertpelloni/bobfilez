# filez TODO List

## Immediate (Short Term)
- [x] **OmniUI Transition**: Transition the native GUI (`fo_gui`) to the **OmniUI** framework (Qt + JUCE + WASM).
- [x] **Windows 11 Explorer Clone**: Implement a high-fidelity Windows 11 Explorer UI in QML/TypeScript.
- [x] **`FileModel` Integration**: Bridge `fo_core` deduplication engine into the new Explorer UI.
- [x] **Mica/Acrylic Effect**: Implement authentic Windows 11 translucency and background blur in BobUI.
- [x] **Context Menus**: Implement pixel-perfect right-click menus for file operations.
- [x] **Taskbar Scaffolding**: Begin implementing the `bob_shell` taskbar with app pinning.
- [x] **Start Menu**: Design and implement the AI-powered Start Menu.
- [x] **Desktop Environment**: Implement Desktop Icons and a System Dashboard.
- [x] **Nexus Integration**: Implement the C++ master clock unification across all modules.
- [ ] **Ultimate++ Refactoring**: Begin refactoring and planning to utilize Ultimate++'s utility toolkit classes across the core.
- [ ] **BobUI Web**: Update web UI to reflect the native functionality.
- [x] **Vector-Semantic Search**: Integrate local Clip/BERT for AI-driven file discovery in Explorer.

## In Progress
- [x] **Advanced Media Analysis**: Video Perceptual Hashing and Audio Fingerprinting implemented.
- [x] **Real-time File Watcher (Shadow Sorter)**: Kernel-level directory monitoring with debounce.
- [ ] **Shell Replacement (`bob_shell`)**: Transforming the app into a full desktop environment.
- [ ] **Java 21 Port Refinement**: Update the Java version with latest CLI parity for `stats` and `--mode`.
- [ ] **Fuzz Testing Campaigns**: Continuous libFuzzer execution on the `RuleEngine`.
- [ ] **Packaging**: AppImage and DMG generation for each build.

## Bug Fixes
- [ ] Resolve any remaining linker issues with `vcpkg` and cloud SDKs in non-standard environments.
- [ ] Compile and configure `btk`, `bobui`, and `ultimatepp` successfully so they can be consumed by the frontend target apps.

## Low Priority
- [ ] Web GUI (`bobui_web`) polish and additional REST API endpoints.
- [x] Example `rules.yaml` files for various user personas (Photographer, SysAdmin).
