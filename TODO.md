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

- [ ] **vault-cli**: implement ault-init|vault-lock|vault-unlock|vault-list DONE 2026-11-14 (v6.5.2): dispatch + Express bridge wired in cli/fo_cli.cpp to VaultManager; /api/vault/* call the real CLI (blanket 501 removed). AES-256-GCM round-trip PROVEN end-to-end: lean MSVC build (build-lean/, standalone OpenSSL 3.6.1 in build-lean-deps/, no manifest toolchain) — CLI vault-init/lock/list/unlock restored byte-identical content, and the HTTP smoke (init→lock→list→unlock) returned 200s with restored-match true. Note: lock_file consumes the source file. The full manifest build-msvc lane remains unbuilt (vcpkg aws-sdk-cpp extraction failed on disk-full/long-path); do NOT start a second build against that tree.

- [x] **build-host**: MinGW portable toolchain (cmake 4.4.5 + ninja 1.13.2 + gcc 16.2) installed at C:/tools; uild-mingw configure PASSED 2026-11-03 after OpenSSL/blake3 QUIET + vault stub + tests-dir guard. COMPILE+LINK OK 2026-11-03 (fo_c_api includes, gdiplus link); all CLI smokes exit 0; dashboard bridges verified end-to-end over HTTP (see CHANGELOG 6.5.0). MinGW runtime DLLs sidecarred next to fo_cli.exe.

- [x] **watch-status**: DONE 2026-11-14 (v6.5.3) — `fo_cli watch` publishes a heartbeat JSON (`%TEMP%/bobfilez_watch_status.json`, events + last event, refreshed per poll); new non-blocking `watch-status` query reports `watching:false` when the heartbeat is absent or >15s stale. `/api/watch/status` bridges to it (200, no more 501); verified live: file creation moved events 0→1 over CLI and HTTP. The last honest-501 bridge is closed.

- [x] **submodule-blake3**: DONE 2026-11-14 — index actually pins `8aa5145` (present locally); the worktree had a stray `3f6dd93` checkout. `git submodule update -- libs/BLAKE3` restored the recorded gitlink; status clean.
- [ ] **providers-ocr-classify**: ATTEMPTED 2026-11-14 — onnxruntime vcpkg chain (abseil→protobuf→onnx→onnxruntime) failed at abseil with no compiler error in logs; disk had crashed to 88 MB mid-build (host-shared consumption), the likely write-failure cause. Needs a host window with >8 GB free; until then Media & AI cards keep badging the missing providers (by design). Tesseract/OpenCV not attempted (larger). Retry command (classic mode, BelowNormal): `vcpkg install onnxruntime --triplet x64-windows --x-install-root=build-lean-deps` from a non-manifest cwd, then reconfigure `build-lean` and check `--modules`.
