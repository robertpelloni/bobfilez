# filez Roadmap (Consolidated)

This roadmap synthesizes the full analysis, library evaluation, and your requirements into a phased implementation plan. All tasks support the goals of **extreme robustness, plugin architecture, cross-platform CLI-first design, and empirical benchmarking**.

---

## Current Status (v2.1.0)

### Completed
- ✅ **Core Engine**: C++20 static library (`fo_core`) with plugin registry.
- ✅ **CLI**: Feature-complete `fo_cli` with 15+ commands.
- ✅ **GUI**: Qt6-based `fo_gui` decoupled from core logic.
- ✅ **Refactoring**: Resolved CLI duplicate code issues and optimized structure.
- ✅ **Providers**:
    - Scanners: `std::filesystem`, `Win32`, `dirent`.
    - Hashers: `xxHash`, `SHA256`, `BLAKE3` (vcpkg).
    - Metadata: `TinyEXIF`, `Exiv2` (vcpkg).
    - OCR: `Tesseract` (vcpkg).
    - Perceptual Hash: `dHash`, `pHash`, `aHash`.
- ✅ **Database**: SQLite with migrations, repositories, and incremental scanning.
- ✅ **Advanced Features**: AI classification (ONNX), Rule-based organization, Batch operations.
- ✅ **Testing**: 59 unit/integration tests passing.
- ✅ **Documentation**: Comprehensive guides (User, Developer, API), Dashboards, and Submodule tracking.
- ✅ **Benchmarks**: Initial microbenchmarks implemented and passing (`Scanner`, `Hasher`).
- ✅ **Packaging**: Windows ZIP distribution script operational.
- ✅ **Submodules**: All 130+ submodules updated to latest upstream (v2.1.0).

### Pending
- 🔄 **Installers**: MSI (Windows) [Script Ready], AppImage (Linux), DMG (macOS).
- ✅ **CI/CD**: Automated Submodule Dashboard Updates.
- ✅ **Fuzz Testing**: Robustness testing infrastructure (sanity checks).
- ⏳ **Cloud Integration**: Optional S3/Drive support (Post-v1.0).

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

## Phase 7: Polish and Release (In Progress)

**Goal**: Documentation, packaging, and public launch.

### Tasks
1. **Documentation** ✅
   - [x] User Manual, Developer Guide, API Reference.
   - [x] Submodule and Feature Dashboards.

2. **Packaging**
   - [x] Portable ZIP scripts.
   - [x] Windows: MSI installer (WiX script added).
   - [ ] Linux: AppImage/Flatpak.
   - [ ] macOS: .dmg.

3. **Testing**
   - [x] Unit and Integration tests.
   - [ ] Fuzz testing.

4. **Java Port (In Progress)**
   - [x] Core/CLI/GUI logic.
   - [x] JNI Stubs.
   - [ ] Native Library Compilation (BLAKE3 submodule added).

5. **Release**
   - [x] Tag v2.0.2.
   - [x] Tag v2.1.0 (Submodule Sync).
   - [ ] Publish on GitHub Releases.

---

## Next Immediate Steps

1. **Java Native Build**: Compile `filez_native.dll` using the new build script.
2. **Installers**: Verify MSI creation on a machine with WiX installed.
3. **Fuzzing**: Set up libFuzzer for the scanner and parser.

