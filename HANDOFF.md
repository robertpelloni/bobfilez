# Session Handoff - v6.5.4 (Classify Provider Live)

## What landed (2026-11-14, cron continuation)
- **Classify live**: prebuilt ONNX Runtime 1.20.1 + OpenCV 4.10.0 staged in
  `build-lean-deps` (vcpkg source chain abandoned at protobuf — ~10 obj/15min;
  abseil needed `VCPKG_MAX_CONCURRENCY=4`). Fixed the real registration bug:
  `classifier_onnx.cpp` static registrar was linker-dropped; now explicit
  `register_classifier_onnx()` in `register_all_providers()`. Verified:
  `--modules` shows `Classifiers: onnx`, Perceptual gains `opencv`; Classify
  CLI + HTTP 200 (MobileNetV2 in `bobui_web/model.onnx` at runtime — not
  committed; labels.txt optional).
- Version **6.5.4**. Tray gained watcher-aware status (`7859d44bc`).

## Next
1. OCR: Tesseract vcpkg chain (smaller than OpenCV was) or prebuilt.
2. Full manifest `build-msvc` rebuild only with ample disk + short vcpkg path.
3. Softmax in classifier postprocess (confidences are raw logits today).

---
# Session Handoff - v6.5.3 (Queryable Watcher — Last 501 Closed)

## What landed (2026-11-14, cron continuation)
- **Queryable watcher**: `watch` publishes a temp-dir heartbeat JSON every
  poll (events + last event); new `watch-status` CLI query + `/api/watch/status`
  bridge report fresh/stale/absent honestly (200, no 501). Live-verified:
  file creation moved events 0→1. **No honest-501 bridges remain.**
- Version **6.5.3**. Lean lane (`build-lean/`) is the verified MSVC build;
  server picks it as newest. Tests 9/9, dashboard smoke all 200.

## Next
1. Provider builds for OCR/Classifiers (Media cards badge their absence).
2. Full manifest `build-msvc` rebuild only with ample disk + short vcpkg path.
3. Tray custodian badge (IDEAS §6) — duplicate-space in the tray tooltip.

---
# Session Handoff - v6.5.2 (Vault Proof + Lean MSVC Lane)

## What landed (2026-11-14, cron continuation)
- **Vault proven end-to-end**: standalone OpenSSL 3.6.1 (`build-lean-deps/`,
  vcpkg classic mode, 1.5 h BelowNormal build) + lean MSVC lane (`build-lean/`,
  no manifest toolchain; all heavy deps are `find_package(QUIET)`-optional).
  CLI and HTTP init→lock→list→unlock round-trips restore byte-identical
  content (`restored-match: true`). Fixed MSVC `std::max` macro break
  (`NOMINMAX` in `enhanced_fileops.cpp`). Server CLI candidates now include
  `build-lean/cli/fo_cli.exe`; OpenSSL DLLs sidecarred beside it.
- Version **6.5.2** (`VERSION.md` + `version.hpp`, binary banner verified).
- Disk discipline learned: full manifest rebuild wipes `vcpkg_installed`
  and needs 60 packages + >10 GB; purged regenerable `vcpkg/buildtrees`
  (kept `openssl`) recovering ~7 GB. Never run two builds on one tree.

## Next
1. `watch-status` queryable watcher (last honest-501 bridge).
2. Provider builds for OCR/Classifiers (Media cards already badge absence).
3. Full manifest `build-msvc` rebuild only with ample disk + short vcpkg path.

---
# Session Handoff - v6.5.1 (Dashboard Wiring Fixes + Script Archive + Governance)

## What landed (2026-11-14, Muse Code / muse-wonder-1.0)
- **Dashboard wiring fixes** (`bobui_web/`): History primary button no longer
  fires `/api/undo` on load; `api()` surfaces HTTP errors (501 vault/watch
  now render as errors, not OK); Clerk bridge runs real `clerk status`
  grammar (card relabeled *OmniClerk status* — `clerk start` blocks forever,
  terminal-only); Oracle forwards the bare question; Search forwards
  wildcard/case/whole-word/invert/max-results; tray polls `/api/health` +
  quits via `/api/quit` (was dead `/health` + `/quit`); `start-server.bat`
  defaults `PORT=3131`.
- **Durable tests**: `bobui_web/test/wiring.test.js` (`node --test test/`,
  6 tests incl. live health + honest-501) wired into `scripts/dev_all.bat`.
- **Scripts cleanup (nondestructive)**: 20 root `patch_*.py`/`fix_*.py`
  one-shots `git mv`'d to `scripts/archive/root-helpers/` (see
  `scripts/README.md`). Canonical entrypoints unchanged.
- **Governance**: created missing `VISION.md`; refreshed `ROADMAP.md`
  (status v6.5.1), `TODO.md` (+`rebuild-cli`), `MEMORY.md`, `DEPLOY.md`
  (unified page + tray + tests), `IDEAS.md` (§6 pivots), `CHANGELOG.md`.
  Version: `VERSION.md` + `version.hpp` → **6.5.1**.

## Verified this session
- `node --test test/` in `bobui_web`: 6/6 pass (live server on :3199).
- `build-msvc/cli/fo_cli.exe --help` exit 0 but banner **v6.3.0** (stale);
  `--modules`: Scanners gdrive/std/win32/dirent/s3/azure, Hashers
  fast64/sha256/xxhash/blake3/dhash, Metadata tinyexif, **OCR/Classifiers
  empty**. `clerk`/`oracle` = Unknown command on this binary (source in
  `cli/fo_cli.cpp:2005/2027` has the dispatch — rebuild required).

## Open / next (in priority order)
1. ~~`rebuild-cli`~~ DONE same session: MinGW lane rebuilt (`build-mingw/`,
   w64devkit gcc 15.2, BelowNormal) after restoring `libs/hash-library` +
   `libs/xxHash` pins; fresh CLI = v6.5.1, live smoke health/scan/clerk/
   oracle/modules/history all 200, wiring tests 7/7. Server now picks the
   newest-built CLI by mtime. `fo_tests` not rebuilt (no `tests/` tree in
   this checkout); stale MSVC binary passes 436/437.
2. `vault-cli`, `watch-status`, `submodule-blake3` (pre-existing TODO items).
3. Do NOT stage `libs/*` gitlink drift (160+ submodules show modified);
   commit explicit paths only.

---
# Session Handoff - v6.5.0 (MinGW Lane + Exit-Crash Fix + Verified Bridges)

## What landed (2026-11-03, this turn)
- Portable no-admin toolchain at `C:/tools` (cmake 4.4.5, ninja 1.13.2, winlibs gcc 16.2).
  `scripts/build_headless_mingw.bat` configures `build-mingw/`; full compile+link OK.
- `build-mingw/cli/fo_cli.exe` runs; every CLI invocation previously crashed at exit
  (`0xC0000374`) — root cause was `SwarmHub` use-after-destroy during static teardown
  (Engine `get_shared` autodev chain). Fixed by leaking the hub singleton on purpose
  (`core/src/swarm_engine.cpp`). Latent on MSVC too.
- `lint` CLI was implemented but never dispatched — wired with `--format=json`.
- `bobui_web/server.js`: `extractJson` payload extractor (service chatter on stdout),
  `build-mingw` CLI candidate, organize/delete-duplicates arg fixes, watch 501.
- HTTP-verified end-to-end (node server, this host): scan/duplicates/stats/lint/count/
  organize/delete-duplicates/scrub/undo/hash/metadata/export/search/history/ignore/
  flow-list/modules = 200; vault + watch/status = honest 501.

## Open (not started)
- `vault-cli` (needs OpenSSL/MSVC host), `watch-status` query mode, `submodule-blake3`
  gitlink repair, `tests/` dir absent (FO_BUILD_TESTS defaults ON against nothing),
  model-dependent bridges unverified (ocr/similar/vhash/afingerprint/classify/oracle).
- Do NOT stage `libs/*` gitlink drift or `build-mingw/` (now gitignored); `fo.db`
  reverted (test artifact, tracked).

## Next
- Bump tested: VERSION.md + version.hpp = 6.5.0, CHANGELOG + TODO updated here.
- `git add` explicit paths only, commit `feat:`/`fix:` prefix, push.

---# Session Handoff - v6.3.4 (Autonomous Coordination & Feedback)

## Executive Summary
This session focused on maturing the **Autonomous Ecosystem** (Phase 10 of the Roadmap) by transitioning from a singleton-heavy architecture to an explicit **Dependency Injection (DI)** model for coordination services (`INexus`, `ISwarmEngine`, `IAutonomousSyncService`). This was critical to enable reliable integration testing where multiple `Engine` instances coexist without interfering with each other's state or database connections.

## Completed Milestones
- **Architectural Refactoring (Ownership & DI)**: Services are now created as `shared_ptr` within the `Engine` and wired together using explicit setters (`set_swarm_engine`, `set_sync_service`).
- **Registry Enhancements**: `Registry<T>` now supports `get_shared` and `clear_shared_instances` to manage singleton lifecycle during test teardown.
- **Distributed Swarm Simulation**: The `SwarmHub` (process-local routing) allows independent nodes to broadcast file update notices, simulating a cluster environment.
- **Autonomous Protocol Logic**:
    - Mandatory checkpointing (automatic `upload_file`) before task execution in `AutonomousDevProtocolImpl`.
    - Human-in-the-loop feedback mechanism (`provide_feedback`) for manual approval/rejection of autonomous tasks.
- **UI Integration**: Wired `activeSwarmPeers` and `lastCheckpointId` to `AutonomousMonitoringPanel.qml` through `FileModel`.
- **Integration Test Stability**: All 22+ tests passing. Resolved DB lock contention and race conditions in file watchers by introducing stabilization delays and explicit DB pathing.

## Technical Notes
- **DB Isolation**: Each test node now has its own `nodeN_sync.db` to prevent cross-contamination.
- **Watcher Debounce**: CI environments required increasing watcher debounce to 500ms-1000ms to avoid overlapping event bursts during rapid file writes.
- **Swarm Peer Discovery**: Currently simulated via `SwarmHub`. Real-world UDP/mDNS discovery is scaffolded in `swarm_engine.cpp` but not yet fully activated in the simulation layer.

## Outstanding Items (ROADMAP.md)
- [ ] Transition `bob_shell` to use the new `AutonomousMonitoringPanel`.
- [ ] Implement `io_uring` support for zero-copy file operations in `enhanced_fileops.cpp`.
- [ ] Full BTK native surface refactoring for the Explorer UI.

## Environment & Build
- **Branch**: `main`
- **Version**: `6.3.4`
- **Build Status**: Verified passing with `cmake -DFO_BUILD_GUI=OFF -DFO_BUILD_OMNI=OFF ..` and `./tests/fo_tests`.

## 2026-11-03 autonomous session (v6.4.0)
- Unified dashboard + 13 API bridges + tray + script archive; server smoke-tested (health/unified/ocr-error/quit). Build impossible on this host (no MSVC/cmake/Qt). Pushed to origin/main. Next model: run scripts/dev_all.bat on a real dev host, then close TODO build-host.
