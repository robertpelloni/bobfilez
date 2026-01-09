# AGENTS.md

**Generated:** 2026-01-09 | **Commit:** 212ee4c5 | **Branch:** main | **Version:** 2.1.0

> Full guidelines: [docs/LLM_INSTRUCTIONS.md](docs/LLM_INSTRUCTIONS.md)

## Overview

C++20 file organization/deduplication engine with plugin architecture. CLI-first (`fo_cli`), optional Qt GUI, 130+ library submodules.

## Structure

```
filez/
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

## Current Status (v2.1.0)

- ✅ 15+ CLI commands (scan, duplicates, hash, metadata, ocr, classify, organize, etc.)
- ✅ 59 passing tests
- ✅ Qt6 GUI decoupled
- ✅ 130+ submodules synced

**Next Steps**: MSI/AppImage packaging, benchmark execution, fuzz testing.

## Handoff Protocol

Update this section when finishing a session:

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
