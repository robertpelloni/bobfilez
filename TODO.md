# filez TODO List

## Current Open (2026-11-24) - the live list; everything below is history

- [ ] **Owner: lean production checkout authorization** - ChatGPT/Gemini
  recommend authorizing prep of a lean checkout (12 build-referenced +
  5 framework `libs/` trees per `docs/SUBMODULE_INVENTORY.md`); no
  pruning or deletion without that call.
- [ ] **Owner: `upstream/master` decision** - unrelated histories
  (73/62, no merge base): keep separate / import selected / replace.
- [ ] **Owner/upstream: BobGUI build** - `libs/bobgui/meson.build` has
  a duplicate unclosed `project()` in the bobgui repo itself; fix
  there, then retry `scripts/build_bobgui_gui.bat` (GTK wrap chain next).
- [ ] **Muse roast pending** - Gemini + ChatGPT roasts received and
  triaged (v6.5.25-6.5.46 fix train); Muse's own roast still owed
  in the coordination thread.
- [x] Fuzz campaigns: lane fixed (v6.5.42); 14.5M-run RuleEngine
  campaign clean. Longer/multi-target campaigns remain open-ended.
- [x] Packaging: MSI parameterized on VERSION.md (v6.5.44); producing
  an actual .msi still needs WiX v3.11 installed.
- [x] **Legacy `/api/delete`** - raw unlink endpoint used only by the
  legacy React UI (no journal/audit). RETIRED v6.5.47: removed from
  `server.js`; deletion only via dry-run-first
  `/api/delete-duplicates`, pinned by `wiring.test.js`.

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
- [x] **build-host**: DONE 2026-11-16 (v6.5.7): `scripts/build_headless.bat` completes here and dashboard bridges were re-verified over HTTP; the surviving `fo_tests.exe` passes 436/437 but predates this source (no tracked `tests/` tree). Original request was to re-run scripts/dev_all.bat on a host with MSVC+cmake+Qt to rebuild o_cli, run o_tests.exe, and re-verify all 13 new dashboard bridges end-to-end (2026-11-03: blocked here — no toolchain).

- [x] **vault-cli**: implement ault-init|vault-lock|vault-unlock|vault-list DONE 2026-11-14 (v6.5.2): dispatch + Express bridge wired in cli/fo_cli.cpp to VaultManager; /api/vault/* call the real CLI (blanket 501 removed). AES-256-GCM round-trip PROVEN end-to-end: lean MSVC build (build-lean/, standalone OpenSSL 3.6.1 in build-lean-deps/, no manifest toolchain) — CLI vault-init/lock/list/unlock restored byte-identical content, and the HTTP smoke (init→lock→list→unlock) returned 200s with restored-match true. Note: lock_file consumes the source file. The full manifest build-msvc lane remains unbuilt (vcpkg aws-sdk-cpp extraction failed on disk-full/long-path); do NOT start a second build against that tree.

- [x] **build-host**: MinGW portable toolchain (cmake 4.4.5 + ninja 1.13.2 + gcc 16.2) installed at C:/tools; uild-mingw configure PASSED 2026-11-03 after OpenSSL/blake3 QUIET + vault stub + tests-dir guard. COMPILE+LINK OK 2026-11-03 (fo_c_api includes, gdiplus link); all CLI smokes exit 0; dashboard bridges verified end-to-end over HTTP (see CHANGELOG 6.5.0). MinGW runtime DLLs sidecarred next to fo_cli.exe.

- [x] **watch-status**: DONE 2026-11-14 (v6.5.3) — `fo_cli watch` publishes a heartbeat JSON (`%TEMP%/bobfilez_watch_status.json`, events + last event, refreshed per poll); new non-blocking `watch-status` query reports `watching:false` when the heartbeat is absent or >15s stale. `/api/watch/status` bridges to it (200, no more 501); verified live: file creation moved events 0→1 over CLI and HTTP. The last honest-501 bridge is closed.

- [x] **submodule-blake3**: DONE 2026-11-14 — index actually pins `8aa5145` (present locally); the worktree had a stray `3f6dd93` checkout. `git submodule update -- libs/BLAKE3` restored the recorded gitlink; status clean.
- [x] **providers-ocr-classify**: CLASSIFY DONE 2026-11-14 (v6.5.4) — staged official prebuilt ONNX Runtime 1.20.1 + OpenCV 4.10.0 into `build-lean-deps` (minimal hand-written `onnxruntimeConfig.cmake`; OpenCV ships its own), reconfigured `build-lean` (all three: OpenCV/ONNX/OpenSSL found). Root bug fixed: `classifier_onnx.cpp` used a static registrar in an unreferenced TU (linker dropped it from `fo_core.lib`); now explicit `register_classifier_onnx()` called from `register_all_providers()`. `--modules`: **Classifiers: onnx, Perceptual Hash: … opencv**. Classify verified CLI + HTTP 200 with MobileNetV2 (`model.onnx` must sit in the server CWD `bobui_web/`; labels.txt optional, indices shown otherwise; confidences are raw logits). OCR DONE 2026-11-14 (v6.5.5): Tesseract 5.5.2 vcpkg chain (15 pkgs, 40 min, concurrency 8) into `build-lean-deps` + `eng.traineddata`; implemented the real `recognize` in the providers/ Tesseract class (was a placeholder returning nullopt) with explicit `register_ocr_tesseract()`, and fixed its latent namespace-shadowing compile break (`::tesseract::`). `--modules`: **OCR: tesseract** — every provider row is now populated. OCR verified CLI + HTTP 200 ("Hello bobfilez OCR 2028" from a rendered image; set `TESSDATA_PREFIX` to the staged tessdata when starting the server).
- [x] **audit-query**: DONE 2026-11-16 (v6.5.8) — `make_audit_logger(DatabaseManager&)` factory, `fo_cli audit [--limit=N] [--format=json]` with chain verification, audit writes at applied move/delete/rename sites, `/api/audit` wired for real. Verified on an isolated DB from both MSVC lanes (identical hash, `verified: true`); wiring tests 14/14.
- [x] **media-analysis**: DONE 2026-11-17 (v6.5.9) — prebuilt FFmpeg shared release + source-built Chromaprint/FFTW staged in `build-lean-deps` (vcpkg FFmpeg source build failed twice: fork exhaustion, `sanm.c` optimizer spin); explicit registration calls added, `FFmpeg::swresample` linked, target-name alias for Chromaprint. Both lanes verified: frame hashes on MP4, 59 chunks on 10s WAV, HTTP 200s, tests 15/15.
- [x] **tests-tree**: DONE 2026-11-17 (v6.5.10) — recreated tracked `tests/` GTest tree (audit ledger, operation journal, provider registry; 11 tests, `ctest` 11/11 in the lean lane) with automatic runtime-DLL copying for gtest discovery. GTest comes from the vcpkg manifest prefix; lean configures need it on `CMAKE_PREFIX_PATH`.
- [ ] **upstream-history**: `main` and `upstream/master` have no merge base (73/62 divergent commits after fetch on 2026-11-16). Do not merge blindly; choose explicitly between keeping them separate, importing selected files, or replacing history with owner approval.
- [x] **qt-demo-alignment**: DONE 2026-11-17 (v6.5.14) — aligned `frontends/qt/src/main.cpp` with current core APIs (QLineEdit include, `QObject::connect`, `search_roots`/`SearchResult::path`, `get_all`, `IgnoreRepository::add`); builds with the MinGW Qt 6.11 kit in `build-qt-mingw/` and launch-verified offscreen. The web dashboard remains the primary UI.
- [x] **juce-demo-lane**: DONE 2026-11-17 (v6.5.15) - VS18 vcvars discovery + vcpkg applocal PowerShell pin in `scripts/build_juce_gui.bat`; `libs/JUCE` restored to recorded gitlink `762a87fc30`; `fo_juce_demo` links and launch-verified (`build-juce-demo/`). Fixed alongside: perceptual hashers (`dhash`/`phash`/`ahash`/`opencv`) now use explicit registration (static registrar was linker-dropped; JUCE-tree registry test caught it), `FO_HAVE_OPENCV` is PUBLIC, guarded registry test added. Lean ctest 22/22, JUCE-tree ctest 19/19, wiring 17/17.
- [ ] **bobgui-lane**: BLOCKED upstream - `libs/bobgui/meson.build` has a duplicate unclosed `project('bobgui', ...)` before `project('gtk', ...)` at its pinned commit, so `meson setup` fails parsing. Fix upstream in bobgui (do not edit submodule), then retry `scripts/build_bobgui_gui.bat`; GTK4 wrap chain (glib/pango/cairo) is the next hurdle.
