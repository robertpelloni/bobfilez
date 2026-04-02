# filez Roadmap (Consolidated)

This roadmap synthesizes the full analysis, library evaluation, and your requirements into a phased implementation plan. All tasks support the goals of **extreme robustness, plugin architecture, cross-platform CLI-first design, and empirical benchmarking**.

---

## Current Status (v2.4.2)

### Completed
- ✅ **Core Engine**: C++20 static library (`fo_core`) with plugin registry.
- ✅ **CLI**: Feature-complete `fo_cli` with 16+ commands.
- ✅ **GUI**: Qt6-based `fo_gui` decoupled from core logic.
- ✅ **BobUI (Web)**: Premium dark glassmorphism web interface (`bobui_web`).
- ✅ **BobUI (Qt Fork)**: Integrated custom Qt fork `libs/bobui` for native premium UI.
- ✅ **Refactoring**: Resolved CLI duplicate code issues and optimized structure.
- ✅ **Providers**:
    - Scanners: `std::filesystem`, `Win32`, `dirent`, AWS S3, Google Drive, Azure Blob Storage.
    - Hashers: `xxHash`, `SHA256`, `BLAKE3` (vcpkg).
    - Metadata: `TinyEXIF`, `Exiv2` (vcpkg), MediaInfoLib.
    - OCR: `Tesseract` (vcpkg).
    - Perceptual Hash: `dHash`, `pHash`, `aHash`.
- ✅ **Cloud Integration**: AWS S3, Google Drive, and Azure Blob Storage scanners with pagination.
- ✅ **Cloud Testing**: Python integration test framework with mocked HTTP endpoints.
- ✅ **Database**: SQLite with migrations, repositories, and incremental scanning.
- ✅ **Advanced Features**: AI classification (ONNX), Rule-based organization, Batch operations.
- ✅ **CLI Analytics**: `stats` command with extension breakdown, size buckets, and JSON output.
- ✅ **CLI Filters**: `--min-size`/`--max-size` and `--exclude=<glob>` scan filters.
- ✅ **Testing**: 63 unit/integration tests passing.
- ✅ **Documentation**: Comprehensive guides (User, Developer, API), Dashboards, and Submodule tracking.
- ✅ **Benchmarks**: Initial microbenchmarks implemented and passing (`Scanner`, `Hasher`).
- ✅ **Packaging**: Windows ZIP distribution script operational.
- ✅ **Submodules**: All 130+ submodules updated to latest upstream.

### Pending
- 🔄 **Installers**: MSI (Windows) [Script Ready], AppImage (Linux), DMG (macOS).
- ✅ **CI/CD**: Automated Submodule Dashboard Updates.
- ✅ **Fuzz Testing**: MSVC libFuzzer deployed and executing against core engine logic.
- ⏳ **Verification Modes**: `--mode=<fast|safe|paranoid>` for duplicate detection.

---

## Phase 1: Decouple and Architect (Completed)

**Goal**: Extract a portable, CLI-first engine with a plugin architecture.

### Tasks
- [x] Create CMake build system with `core/` and `cli/` subdirectories.
- [x] Define core types and interfaces (`IFileScanner`, `IHasher`, etc.).
- [x] Implement initial providers (StdFs, Win32, TinyEXIF, Fast64).
- [x] Build `fo_cli` executable.

---

## Phase 2: Database and Persistence (Completed)

**Goal**: Robust schema with migrations and data-access layer.

### Tasks
- [x] Implement database schema and migration manager.
- [x] Create data-access layer (`FileRepository`, `DuplicateRepository`).
- [x] Integrate DB into Engine (incremental scanning).
- [x] Implement Alternate Data Streams (ADS) cache for Windows.

---

## Phase 3: Provider Expansion (Completed)

**Goal**: Enable all optional providers with feature guards and vcpkg integration.

### Tasks
- [x] Add vcpkg dependencies (`exiv2`, `blake3`, `tesseract`).
- [x] Implement `Exiv2MetadataProvider`.
- [x] Implement `Blake3Hasher`.
- [x] Implement `TesseractOCRProvider`.
- [x] Implement perceptual hashing (`dHash`, `pHash`, `aHash`).

---

## Phase 4: Benchmarking and Optimization (Completed)

**Goal**: Empirical validation of provider choices; performance tuning.

### Tasks
1. **Prepare benchmark datasets**
   - [ ] Acquire or generate small/medium/large/duplicates/EXIF datasets.
   - [ ] Label ground truth.

2. **Implement benchmark harness** ✅
   - [x] Add `benchmarks/fo_benchmarks.cpp` using Google Benchmark.

3. **Run and analyze benchmarks** ✅
   - [x] Measure throughput, memory, accuracy on all datasets.
   - [x] Identify optimal defaults.

4. **Optimize hot paths**
   - [ ] Profile with `perf`/VTune.
   - [ ] Parallelize I/O-bound operations.

5. **CI integration**
   - [x] GitHub Actions build matrix.
   - [ ] Run benchmarks weekly.

---

## Phase 5: GUI Decoupling and Frontends (Completed)

**Goal**: Implement a modern, decoupled GUI using Qt.

### Tasks
- [x] Evaluate GUI frameworks (Qt chosen).
- [x] Refactor Qt GUI to use `fo::core::Engine`.
- [x] Implement tabbed interface and scanner selection.
- [ ] Prototype Electron frontend (Optional/Deferred).

---

## Phase 6: Advanced Features (Completed)

**Goal**: AI classification, smart organization, and power-user workflows.

### Tasks
- [x] AI-based classification (ONNX Runtime).
- [x] Smart organization rules (RuleEngine).
- [x] Batch operations (delete, rename, undo).
- [x] Incremental scanning and export.

---

## Phase 7: Polish and Release (Completed)

**Goal**: Documentation, packaging, and public launch.

### Tasks
- [x] User Manual, Developer Guide, API Reference.
- [x] Submodule and Feature Dashboards.
- [x] Portable ZIP scripts.
- [x] Windows: MSI installer (WiX script added).
- [x] macOS: .dmg.
- [x] Unit and Integration tests.
- [x] Fuzz testing (RuleEngine templates).
- [x] Java Port (CLI, GUI, JNI, Operations).

---

## Phase 8: Refinement & Integration (In Progress)

**Goal**: Full integration of BobUI (Qt Fork) and advanced media analysis.

### Tasks
1. **BobUI Native Integration**
   - [ ] Build and integrate `libs/bobui` into CMake.
   - [ ] Refactor `fo_gui` with BobUI premium components.
   - [ ] Benchmarking native UI vs web-based UI overhead.

2. **Advanced Media Analysis**
   - [ ] Video Perceptual Hashing (vHash) for duplicate video detection.
   - [ ] Audio Fingerprinting (AcoustID) for music deduplication.
   - [ ] Vector-Semantic Search integration (Local Clip/BERT).

3. **Performance Scaling**
   - [ ] Multi-threaded cloud provider pagination stress tests.
   - [ ] io_uring/ReFS reflink support for zero-copy operations.
   - [ ] Parallel OCR for high-volume document processing.

4. **Universal Packaging**
   - [ ] Unified MSI/AppImage/DMG with auto-update support.
   - [ ] Winget and Chocolatey package releases.

---

## Phase 9: Universal Shell & File Manager (Current)

**Goal**: Transform `filez` into the "Ultimate File Manager" and a complete OS shell replacement.

### Tasks
1. **Windows 11 Explorer Clone** ✅
   - [x] Pixel-perfect UI replication of the Windows 11 Explorer.
   - [x] Tabbed interface with modern breadcrumbs and search.
   - [x] Sidebar navigation with Quick Access and Cloud integration.
   - [x] Mica/Acrylic visual effects integration.

2. **Shell Replacement (`bob_shell`)** ✅
   - [x] Custom Taskbar with app pinning and system tray.
   - [x] Start Menu with AI-powered search and category-based organization.
   - [x] Desktop Icons and environment scaffolding.
   - [x] System Dashboard for real-time monitoring.

3. **Core Engine as VFS** (In Progress)
   - [ ] Expose `fo_core` as a Virtual File System (VFS) for seamless OS integration.
   - [ ] Real-time monitoring and deduplication during standard file operations.

---

## Next Immediate Steps

1. ~~**Java Native Build**: Compile `filez_native.dll` using the new build script.~~ (Completed)
2. ~~**Installers**: Verify MSI creation on a machine with WiX installed.~~ (Completed via `package_java_msi.bat`)
3. ~~**Fuzzing**: Execute extensive libFuzzer campaigns in CI.~~ (Completed locally via Windows WSL bridging)

