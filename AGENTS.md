# AGENTS.md

**Generated:** 2026-02-07 | **Commit:** - | **Branch:** main | **Version:** 2.2.1

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

## Current Status (v2.2.1)

- ✅ Fixed `ai-file-sorter` submodule clone error (invalid ref `c038d3e`).
- ✅ 15+ CLI commands (scan, duplicates, hash, metadata, ocr, classify, organize, etc.)
- ✅ 59 passing tests
- ✅ Qt6 GUI decoupled
- ✅ 130+ submodules synced

**Next Steps**: MSI/AppImage packaging, benchmark execution, fuzz testing.

## Handoff Protocol

Update this section when finishing a session:

---

## Handoff Protocol

Update this section when finishing a session:

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
