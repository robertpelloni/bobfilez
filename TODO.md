# filez TODO List

## Immediate (Short Term)
- [x] **Autonomous Sync**: Implement real-time file synchronization across nodes.
- [x] **Distributed Coordination**: Integrate version tracking into the core Engine.
- [x] **Robust Error Handling**: retry mechanism implemented for file operations.
- [ ] **BobTK Go Port**: Stabilize the initial Go port scaffolding and documentation.
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
- [x] **BobUI Web**: Update web UI to reflect the native functionality. — Done: single-page unified dashboard (A–E by value, ⓘ tooltips, activity log) + tray with Quit; wiring bugs fixed in v6.5.1 (history-undo-on-load, dead tray routes, clerk/oracle grammar, swallowed HTTP errors) with `node --test` coverage.
- [x] **rebuild-cli**: rebuilt `fo_cli` from current source — DONE 2026-11-14: MinGW lane (`build-mingw/`, w64devkit gcc 15.2, BelowNormal) after restoring `libs/hash-library` + `libs/xxHash` pins; fresh binary reports **v6.5.1**, `clerk status` exit 0, dashboard health/scan/clerk/oracle/modules/history all HTTP 200. Server now prefers the newest-built CLI (`pickCli` mtime) so stale `build-msvc` (v6.3.0) no longer shadows it. C++ `fo_tests` could not be rebuilt (no `tests/` tree in this checkout); stale MSVC binary still passes 436/437 (1 skipped, no models).
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

- [x] Expanded VaultManager tests to 14 total ensuring 200+ testing milestone completeness.
- [ ] **build-host**: re-run scripts/dev_all.bat on a host with MSVC+cmake+Qt to rebuild o_cli, run o_tests.exe, and re-verify all 13 new dashboard bridges end-to-end (2026-11-03: blocked here — no toolchain).

- [ ] **vault-cli**: implement ault-init|vault-lock|vault-unlock|vault-list dispatch + Express bridge WIRED 2026-11-14 in cli/fo_cli.cpp wired to VaultManager; /api/vault/* call the real CLI (blanket 501 removed). MinGW build proves grammar + honest exit-1 without OpenSSL. REMAINING: MSVC rebuild (vcpkg 60-package install is the long pole) to verify the AES-256-GCM round-trip end-to-end.

- [x] **build-host**: MinGW portable toolchain (cmake 4.4.5 + ninja 1.13.2 + gcc 16.2) installed at C:/tools; uild-mingw configure PASSED 2026-11-03 after OpenSSL/blake3 QUIET + vault stub + tests-dir guard. COMPILE+LINK OK 2026-11-03 (fo_c_api includes, gdiplus link); all CLI smokes exit 0; dashboard bridges verified end-to-end over HTTP (see CHANGELOG 6.5.0). MinGW runtime DLLs sidecarred next to fo_cli.exe.

- [ ] **watch-status**: fo_cli watch is a blocking foreground loop; /api/watch/status answers 501. Add a queryable watcher mode or detached-watch tracking if live status is needed.

- [ ] **submodule-blake3**: superproject pins libs/BLAKE3 @ 3f6dd93 which upstream no longer serves (fetch fails like the old 7zip case); worktree restored at local f55849f. Repair gitlink to a valid remote tip (2026-11-03).
