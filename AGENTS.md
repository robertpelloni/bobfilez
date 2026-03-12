# AGENTS.md

**Generated:** 2026-03-10 | **Commit:** - | **Branch:** main | **Version:** 2.4.1

> Full guidelines: [docs/UNIVERSAL_LLM_INSTRUCTIONS.md](docs/UNIVERSAL_LLM_INSTRUCTIONS.md)

## Overview

**bobfilez** - C++20 file organization/deduplication engine with plugin architecture. CLI-first (`fo_cli`), optional Qt GUI, 130+ library submodules. Part of the bob software ecosystem.

## Structure

```
bobfilez/
├── core/           # fo_core static library (interfaces, providers, engine, DB)
├── cli/            # fo_cli executable (15+ commands)
├── gui/            # fo_gui Qt6 application (optional)
├── tests/          # GTest unit/integration tests (59 tests)
├── benchmarks/     # Google Benchmark harness
├── libs/           # 130+ git submodules (DO NOT modify directly)
├── docs/           # Documentation (ROADMAP, SUBMODULES, LLM_INSTRUCTIONS)
├── scripts/        # Build/packaging scripts
├── wix/            # MSI installer templates
└── vcpkg/          # Package manager submodule
```

## Build Commands

```powershell
build.bat                                    # Quick build (Windows)
cmake -S . -B build -G Ninja && cmake --build build  # Manual
.\build\tests\fo_tests.exe                   # All tests
.\build\tests\fo_tests.exe --gtest_filter=*Name*     # Single test
.\build\cli\fo_cli.exe --help                # CLI usage
```

## Where to Look

| Task | Location | Notes |
|------|----------|-------|
| Add scanner/hasher/provider | `core/src/`, `core/include/fo/core/` | Use Registry pattern |
| Add CLI command | `cli/cmd_*.cpp` | Follow existing cmd_ pattern |
| Add test | `tests/test_*.cpp` | GTest fixtures |
| Update dependencies | `vcpkg.json` | Then rebuild |
| Submodule info | `docs/SUBMODULES.md` | Auto-generated dashboard |

## Code Patterns

**Registry Pattern** (providers):
```cpp
static auto reg = []() {
    Registry<IFileScanner>::instance().add("name", []() {
        return std::make_unique<MyScannerImpl>();
    });
    return true;
}();
```

**Feature Guards**: `#ifdef FO_HAVE_TESSERACT`, `#ifdef FO_HAVE_BLAKE3`, etc.

**Platform Guards**: `#ifdef _WIN32` for Windows-specific code.

## Anti-Patterns (NEVER)

- ❌ `as any`, `@ts-ignore` equivalents — no type suppression
- ❌ Modify files in `libs/` directly — they're submodules
- ❌ Hardcode version — read from `VERSION.md`
- ❌ Skip tests after changes — always verify with `fo_tests.exe`
- ❌ Commit without conventional prefix — use `feat:`, `fix:`, `chore:`, `docs:`

## Conventions

- **C++20**: `std::filesystem`, `std::optional`, `std::chrono`
- **Naming**: `snake_case` functions/variables, `CamelCase` classes
- **Headers**: `.hpp`, `#pragma once`, includes grouped (std → external → internal)
- **Versioning**: `VERSION.md` = single source of truth, SemVer

## Version Update Protocol

1. Edit `VERSION.md` (single line)
2. Add entry to `CHANGELOG.md`
3. Commit: `git commit -m "chore: bump version to X.Y.Z"`
4. Push

## Submodule Commands

```powershell
git submodule update --init --recursive      # Initialize all
git submodule status                         # Check status
python scripts/generate_dashboard.py         # Update dashboard
```

## Current Status (v2.4.1)

- ✅ **Web GUI (BobUI)**: Premium web interface wrapping `fo_cli.exe` with Express.js backend. Features a Dashboard, Smart Clean wizard, real-time stats, drag-and-drop, and dark glassmorphism design.
- ✅ Cloud storage providers: AWS S3, Google Drive, and Azure Blob Storage scanners
- ✅ 16+ CLI commands (scan, duplicates, hash, metadata, ocr, classify, organize, stats, lint, ignore, etc.)
- ✅ `--threads=<N>` parallel hashing for `hash` and `duplicates --mode=safe` commands
- ✅ `--mode=<fast|safe|paranoid>` three-tier duplicate verification
- ✅ `--min-size`/`--max-size`, `--exclude`, `--no-recursive`, `--count`, `--prune`, `--time`, `--verbose` flags
- ✅ `ignore add/remove/list` for managing ignore rules
- ✅ Zero MSVC warnings (ctime_s/localtime_s on Windows)
- ✅ 63 passing tests
- ✅ 130+ submodules synced

**Next Steps**: MSI/AppImage packaging, cloud pagination stress tests.

## Handoff Protocol

Update this section when finishing a session:

---

### Update: 2026-03-10 (Session 10)
**Author:** Antigravity

**Scope:** BobUI v2.4.0 & v2.4.1 (Web GUI for bobfilez)

**Delivered:**
- ✅ v2.4.0: `bobui/server.js` Express backend wrapping `fo_cli.exe` with REST API endpoints (`/api/duplicates`, `/api/scan`, etc.).
- ✅ v2.4.0: Initial BobUI HTML/JS frontend with dark glassmorphism design (dupeGuru style).
- ✅ v2.4.1: Premium UI upgrade - Dashboard home with animated hero and quick stats.
- ✅ v2.4.1: Smart Clean 4-step wizard with animated progress bars and auto-mark strategies.
- ✅ v2.4.1: Drag-and-drop folder support and file type emoji icons.

---

### Update: 2026-03-09 (Session 9)
**Author:** Antigravity

**Scope:** v2.3.5→v2.3.9 (Parallel Hashing, Timing, Verbose, Ignore Management, Count, Prune)

**Delivered:**
- ✅ v2.3.5: `--threads=<N>` parallel hashing for `hash` command with per-thread `IHasher` instances
- ✅ v2.3.6: `--time` elapsed display, universal `apply_filters` across all 8 scan-based commands
- ✅ v2.3.7: `--verbose` progress output, parallel strong hash for `duplicates --mode=safe`
- ✅ v2.3.8: `ignore add/remove/list` CLI, zero MSVC warnings (ctime_s/localtime_s)
- ✅ v2.3.9: `--count` flag for scan/duplicates, `--prune` documented in help

---

### Update: 2026-03-09 (Session 8)
**Author:** Antigravity

**Scope:** Verification Modes, No-Recursive Flag, Threads Scaffolding

**Status:**
- ✅ **`--mode=<fast|safe|paranoid>`**: Three-tier duplicate verification with strong hash and byte-by-byte comparison.
- ✅ **`--no-recursive`**: CLI-level depth filter using canonical path comparison. Works with all scanner backends.
- ✅ **`--threads=<N>`**: Argument parsed and validated, scaffolded for future parallel hashing.
- ✅ **Roadmap**: Updated `docs/ROADMAP.md` from stale v2.2.0 to current v2.3.2.
- ✅ **Version Bump**: Updated to `2.3.4` and documented in `CHANGELOG.md`.

**Next Steps:**
1. Wire `--threads` into parallel hashing implementation.
2. MSI/AppImage packaging.
3. Cloud provider pagination stress tests.

---

### Update: 2026-03-09 (Session 7)
**Author:** Antigravity

**Scope:** CLI Enhancements — Stats, Size Filters, Exclude Patterns

**Status:**
- ✅ **`stats` command**: File count, total size, extension breakdown (top 20), size bucket distribution, oldest/newest file tracking. Supports `--format=json`.
- ✅ **`--min-size`/`--max-size`**: Post-scan file size filters with K/M/G suffix parsing.
- ✅ **`--exclude=<glob>`**: Repeatable glob pattern exclusion filter for filenames.
- ✅ **Helper functions**: `parse_size_string()`, `glob_match()`, `format_human_size()` added to `fo_cli.cpp`.
- ✅ **Roadmap**: Updated `docs/ROADMAP.md` from v2.2.0 to v2.3.2.
- ✅ **Version Bump**: Updated to `2.3.2` and documented in `CHANGELOG.md`.

**Next Steps:**
1. Implement `--mode=<fast|safe|paranoid>` verification modes for duplicate detection.
2. Cloud provider pagination stress tests.
3. MSI/AppImage packaging.

---

### Update: 2026-03-09 (Session 6)
**Author:** Antigravity

**Scope:** Cloud Provider Integration Testing & AWS SDK Fixes

**Status:**
- ✅ **AzureBlobScanner**: Resolved `Azure::DateTime` → `std::chrono::file_clock` conversion via `PosixTimeConverter::DateTimeToPosixTime`.
- ✅ **Cloud Registration**: Replaced anonymous static registrars with explicit `register_scanner_*()` functions to fix MSVC linker stripping.
- ✅ **AWS SDK Lifecycle**: Added `Aws::InitAPI`/`Aws::ShutdownAPI` RAII in `fo_cli` main. Fixed `FO_HAVE_S3` CMake visibility (`PRIVATE` → `PUBLIC`).
- ✅ **S3 Path-Style**: Forced path-style addressing and mock credentials for custom endpoints.
- ✅ **Integration Tests**: `tests/mock_cloud_scanners.py` — all 3 scanners (GDrive, S3, Azure) pass with mocked HTTP server.
- ✅ **Version Bump**: Updated to `2.3.1` and documented in `CHANGELOG.md`.

**Next Steps:**
1. Add pagination stress tests for large (1000+) object sets.
2. MSI/AppImage packaging.
3. Consider Azure Blob Storage authentication via `DefaultAzureCredential`.

---

### Update: 2026-03-07 (Session 5)
**Author:** Antigravity (Claude)

**Scope:** Cloud Storage Providers (AWS S3, Google Drive)

**Status:**
- ✅ **S3Scanner**: Implemented AWS S3 file scanner using `aws-sdk-cpp`. Lists objects from S3 buckets with metadata extraction.
- ✅ **GDriveScanner**: Implemented Google Drive scanner using `cpr` HTTP client and the Drive REST API v3.
- ✅ **CLI Integration**: Added `--s3-bucket=` and `--gdrive-token=` flags. Environment variable injection for provider factories.
- ✅ **Core Refactor**: `FileInfo::uri` (string) replaces `std::filesystem::path` to support cloud URIs.
- ✅ **Dependencies**: Added `aws-sdk-cpp[s3]` and `cpr` to `vcpkg.json`.
- ✅ **Version Bump**: Updated to `2.3.0` and documented in `CHANGELOG.md`.
- ✅ **Tests**: All 63 tests passing.

**Next Steps:**
1. Add pagination for large S3 buckets and Drive file lists.
2. Consider Azure Blob Storage provider.
3. Add integration tests for cloud scanners (mocked endpoints).

---

### Update: 2026-03-07 (Session 4)
**Author:** Sisyphus (Gemini)

**Scope:** Linux Packaging and Fuzz Testing

**Status:**
- ✅ **Linux AppImage**: Created `scripts/package_appimage.sh` for portable deployment on Linux distributions using `appimagetool` and `linuxdeploy`.
- ✅ **macOS DMG**: Created `scripts/package_macos.sh` incorporating CMake compilation, App Bundle construction, and Ad-Hoc Code Signing.
- ✅ **Fuzz Testing**: Authored `scripts/build_fuzzer.sh` for Linux and validated existing rules engines using LibFuzzer locally via `build_fuzzer.bat` under WSL.
- ✅ **Version Bump**: Updated to `2.2.5` and documented in `CHANGELOG.md`.
- ✅ **Java Port Packaging**: 
  - Compiled a functional cross-platform standalone executable jar using `./gradlew :cli:fatJar`.
  - Authored a WiX toolchain definition (`wix/filez_java.wxs`) resolving native JNI bindings alongside the flat-jar format.
  - Wrote a batch runner to automate this pipeline (`scripts/package_java_msi.bat`).

**Next Steps:**
1. **Verification**: Confirm the generated AppImage and DMG behave correctly on diverse Linux and macOS environments.
2. **Maintenance**: Evaluate the necessity for code signing certificates.
3. **Execution**: Verify that a host with the WiX Toolset can successfully compile the template to `.msi`.

---

### Update: 2026-02-07 (Session 3)
**Author:** Sisyphus (Gemini)

**Scope:** Submodule Fix and Version Bump

**Status:**
- ✅ **Submodule Fix**: Rolled back `ai-file-sorter` to `f29262a` to resolve cloning issues on proxied/mirrored environments (like Jules's).
- ✅ **Dashboard**: Regenerated `docs/SUBMODULES.md` with the updated submodule state.
- ✅ **Version Bump**: Updated to `2.2.4` and documented in `CHANGELOG.md`.

**Next Steps:**
1. **Verification**: Confirm with Jules if the clone now works.
2. **Maintenance**: Continue with packaging and distribution tasks.

---

### Update: 2026-02-07 (Session 2)
**Author:** Sisyphus (Gemini)

**Scope:** Java Port Feature Completion

**Status:**
- ✅ **Java Features**: Implemented near-full parity with C++ CLI.
  - `undo`, `history`: Operation tracking and rollback.
  - `metadata`: Using `metadata-extractor` (EXIF/IPTC/XMP).
  - `ocr`: Using `tess4j` (Tesseract wrapper).
  - `similar`: Perceptual hashing (dHash/aHash) with pure Java implementation.
  - `export`: JSON/CSV/HTML export.
  - `stats`: File statistics.
  - `rename`, `delete-duplicates`: Batch operations.
- ✅ **Infrastructure**: Added `libs/BLAKE3` submodule and native build script.

**Next Steps:**
1. **Native Build**: Run `filez-java/native/compile_native_win.bat` (requires MSVC).
2. **Packaging**: Build MSI and Java fat JARs.
3. **AI Classification**: Implement `classify` command in Java (deferred, requires ONNX).

**Handoff Note:**
The Java port is now feature-complete for all core file organization tasks (deduplication, metadata, OCR, perceptual hash, organization). Only AI object classification is missing compared to the C++ version.

---

### Update: 2026-01-12
**Author:** Sisyphus (Claude)

**Scope:** Java 21 port - complete migration of filez to Java with JNI support

**Status:**
- ✅ Created `filez-java/` - full Java 21 port with multi-module Gradle structure
- ✅ Core types as Java records: `FileInfo`, `Hashes`, `DuplicateGroup`, `ScanResult`, etc.
- ✅ Interfaces: `FileScanner`, `Hasher`, `MetadataProvider`, `OCRProvider`, `Classifier`
- ✅ Registry pattern for provider management (same as C++ version)
- ✅ Implementations: `NioFileScanner`, `JavaHasher` (SHA-256, MD5, XXH64)
- ✅ Database layer: `DatabaseManager`, `FileRepository`, `DuplicateRepository` (SQLite)
- ✅ CLI with picocli: `scan`, `hash`, `duplicates`, `metadata`, `organize` commands
- ✅ JNI stubs: `Blake3Jni`, `XxHash64Jni`, `NativeHasher` with auto-fallback
- ✅ 30 JUnit 5 tests (all passing): `NioFileScannerTest`, `JavaHasherTest`, `FileRepositoryTest`
- ✅ Run scripts: `filez.bat`, `filez.sh` with memory flags
- ✅ Committed and pushed: `1bfa2831`

**Project Structure:**
```
filez-java/
├── core/           # Types, interfaces, implementations, database
├── cli/            # picocli commands (FilezApp entry point)
├── native/         # JNI wrappers (com.filez.jni package)
├── build/          # Compiled classes
└── *.jar           # Dependencies (picocli, sqlite-jdbc, junit, slf4j)
```

**Next:**
1. Build native library (`filez_native.dll`/`.so`) with BLAKE3/XXHash64 JNI implementations
2. Add Gradle wrapper (`gradlew`) for proper build automation
3. Create fat JAR for single-file distribution
4. Add more tests for CLI commands and DuplicateRepository
5. Consider GraalVM native-image for startup performance

**Handoff Note:**
Java port is functional and tested. CLI works: `filez scan`, `filez duplicates`, etc. JNI stubs ready for native implementation. The C++ version remains the production codebase; Java port is for cross-platform distribution and JVM ecosystem integration.

---

### Update: 2026-01-09
**Author:** Sisyphus (Claude)

**Scope:** Documentation overhaul, VISION.md creation, codebase audit

**Status:**
- ✅ Created comprehensive `docs/VISION.md` (413 lines) documenting project goals, architecture, feature taxonomy, CLI reference, and success metrics
- ✅ Added structured AGENTS.md files for `cli/`, `core/`, `tests/` subdirectories
- ✅ Updated root AGENTS.md with improved structure and current status
- ✅ Deleted stale `feat/filesystem-lint` branch (already merged to main)
- ✅ Full codebase audit: no disabled tests, no FIXME/HACK comments, 2 minor TODOs (non-blocking)
- ✅ All 59 tests remain passing, 76 source files across core/cli/tests

**Code Health:**
- Minor: 6 files have include ordering (internal before std) - cosmetic only
- TODOs: `metadata_exiv2.cpp` (date parsing), `ocr_tesseract.cpp` (bounding boxes) - enhancement areas

**Next:**
1. MSI installer (WiX templates ready in `wix/`)
2. AppImage/DMG for Linux/macOS
3. Run benchmark suite on real datasets
4. Implement fuzz tests (`tests/fuzz/` scaffolding exists)
5. Tag and publish GitHub Release for v2.1.0

**Handoff Note:**
Project is feature-complete at v2.1.0. Documentation is now comprehensive with VISION.md capturing the full product vision. Ready for packaging and release phase.
