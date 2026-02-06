# Universal LLM Instructions

This document serves as the central instruction file for all AI agents (GitHub Copilot, Claude, Gemini, GPT, etc.) working on the filez project.

**Current Version:** See `VERSION.md` (Single Source of Truth)

---

## 1. Core Principles

- **Single Source of Truth**: `VERSION.md` contains the version string only.
- **Changelog Updates**: Every significant change must be recorded in `CHANGELOG.md`.
- **Documentation First**: Update documentation (`docs/`) alongside code changes.
- **Submodule Awareness**: 130+ submodules in `libs/`. Refer to `docs/SUBMODULES.md`.
- **Autonomy**: Proceed with implementation. Don't stop for confirmation unless blocked.
- **No Regressions**: Verify existing features work after changes.

---

## 2. Build Commands

### Quick Build (Windows)
```powershell
build.bat
```

### Manual Build
```powershell
cmake -S . -B build -G Ninja
cmake --build build
```

### Run Tests
```powershell
.\build\tests\fo_tests.exe                           # All tests
.\build\tests\fo_tests.exe --gtest_filter=*Name*     # Single test
```

### CLI Usage
```powershell
.\build\cli\fo_cli.exe --help
.\build\cli\fo_cli.exe --version
.\build\cli\fo_cli.exe scan --scanner=std /path/to/dir
```

---

## 3. Versioning Protocol

### Version Number Format
- Format: `MAJOR.MINOR.PATCH` (e.g., `1.4.0`)
- `VERSION.md` contains ONLY the version string (no other content).

### Version Update Procedure
1. Update `VERSION.md` with new version.
2. Update `CHANGELOG.md` with new entry at top.
3. Commit with message: `chore: bump version to X.Y.Z`
4. Push to remote.

### When to Increment
- **MAJOR**: Breaking changes, major feature releases.
- **MINOR**: New features, significant enhancements.
- **PATCH**: Bug fixes, minor improvements.

---

## 4. Coding Standards

### C++ Guidelines
- **Standard**: C++20 (`std::filesystem`, `std::chrono`, `std::optional`)
- **Headers**: Use `.hpp` extension, guard with `#pragma once`
- **Naming**: `snake_case` for variables/functions, `CamelCase` for classes
- **Includes**: Group by standard → external → internal, alphabetically

### Build System
- **CMake**: Minimum 3.16, use modern targets
- **Generator**: Ninja preferred
- **Dependencies**: vcpkg for external packages
- **Optional Features**: Guard with `FO_HAVE_*` defines (e.g., `FO_HAVE_TESSERACT`)

### Platform Code
- Guard with `#ifdef _WIN32` or `#ifdef __linux__`
- Provide portable defaults where possible
- Windows-specific: ADS cache, Win32 scanner

### Registry Pattern
```cpp
// Register a provider in static initialization
static auto reg = []() {
    Registry<IFileScanner>::instance().add("name", []() {
        return std::make_unique<MyScannerImpl>();
    });
    return true;
}();
```

---

## 5. Project Structure

```
filez/
├── core/                    # fo_core static library
│   ├── include/fo/core/     # Public headers
│   └── src/                 # Implementation
├── cli/                     # fo_cli executable (15+ commands)
├── gui/                     # fo_gui Qt6 application
├── tests/                   # Unit tests (GTest)
├── benchmarks/              # Performance benchmarks
├── docs/                    # Documentation
├── libs/                    # 130+ git submodules
├── vcpkg/                   # Package manager
├── VERSION.md               # Version (single line)
├── CHANGELOG.md             # Release history
└── AGENTS.md                # AI handoff log (concise)
```

---

## 6. Key Files Reference

| File | Purpose |
|------|---------|
| `VERSION.md` | Version string only |
| `CHANGELOG.md` | Release history with dates |
| `AGENTS.md` | Quick reference for AI agents |
| `docs/ROADMAP.md` | Feature roadmap |
| `docs/SUBMODULES.md` | Submodule dashboard |
| `docs/UNIVERSAL_LLM_INSTRUCTIONS.md` | This file |
| `vcpkg.json` | Package dependencies |

---

## 7. Handoff Protocol

When finishing a session, update `AGENTS.md` handoff log (in `docs/UNIVERSAL_LLM_INSTRUCTIONS.md` or separate):

```markdown
### Update: YYYY-MM-DD

**Author:** [Model Name]

**Scope:** [Brief description]

**Current Status:**
- [Bullet points of completed work]

**Next Steps:**
1. [Numbered list of recommendations]

**Handoff Note:**
[Summary for next AI agent]
```

---

## 8. Git Workflow

### Commit Messages
- `feat:` New feature
- `fix:` Bug fix
- `chore:` Maintenance (version bump, deps)
- `docs:` Documentation only
- `refactor:` Code restructuring
- `test:` Test additions/changes

### Branch Strategy
- `main`: Stable, deployable
- `feature/*`: Feature development
- Always merge to main when stable

### Submodule Commands
```powershell
# Initialize and update all submodules
git submodule update --init --recursive

# Update to latest upstream
git submodule foreach "git fetch origin && git checkout origin/HEAD 2>/dev/null || true"

# View status
git submodule status
```

---

## 9. CLI Commands Reference

| Command | Description |
|---------|-------------|
| `scan` | Scan directories for files |
| `duplicates` | Find duplicate files |
| `hash` | Compute file hashes |
| `metadata` | Extract EXIF metadata |
| `ocr` | Extract text from images |
| `similar` | Find visually similar images |
| `classify` | AI image classification |
| `organize` | Rule-based file organization |
| `delete-duplicates` | Remove duplicate files |
| `rename` | Pattern-based renaming |
| `export` | Export to JSON/CSV/HTML |
| `undo` | Undo last operation |
| `history` | View operation log |

---

## 10. Model-Specific Instructions

### GitHub Copilot
- See `.github/copilot-instructions.md`
- Use terminal for git operations and builds.

### Claude / Anthropic
- See `CLAUDE.md`
- Focus on architectural analysis and documentation.

### Gemini / Google
- See `GEMINI.md`
- Leverage large context window for comprehensive analysis.

### GPT / OpenAI
- See `GPT.md`
- Strong at code generation and refactoring.

---

## 11. Current Session Handoff Log

### Update: 2025-12-31 (Session 21)

**Author:** OpenCode Interpreter

**Scope:** Submodule Update, Documentation Refresh, Version Bump (v2.1.0).

**Current Status:**
- ✅ **Submodules**: Updated 130+ submodules to latest upstream commits (FFmpeg, ImageMagick, libheif, etc.).
- ✅ **Dashboard**: Regenerated `docs/SUBMODULES.md` with latest commit hashes and versions.
- ✅ **Documentation**: Updated `CHANGELOG.md` and `docs/LLM_INSTRUCTIONS.md` to reflect latest state.
- ✅ **Versioning**: Bumped to `2.1.0` in `VERSION.md`.

**Next Steps:**
1. **Packaging**: Create platform-specific installers (MSI, AppImage, DMG).
2. **Benchmarking**: Execute the benchmark harness on real datasets and publish results.
3. **Fuzzing**: Implement libFuzzer tests for robustness.
4. **Release**: Tag v2.1.0 and publish to GitHub.

**Handoff Note:**
The project is fully synced with upstream dependencies, documented, and versioned. The next phase focuses on stability (fuzzing/benchmarks) and distribution (packaging).

### Update: 2025-12-29 (Session 19)

**Author:** Gemini 2.0 Flash (Architect Mode)

**Scope:** Documentation overhaul, Roadmap update, and Project Structure clarification.

**Current Status:**
- ✅ **Roadmap**: Updated `docs/ROADMAP.md` to reflect v2.0.0 status (Feature Complete).
- ✅ **Project Structure**: Updated `docs/PROJECT_STRUCTURE.md` with detailed directory layout.
- ✅ **Submodules**: Created `docs/SUBMODULES.md` with categorized list of 130+ dependencies.
- ✅ **LLM Instructions**: Verified central instruction file and agent links.

**Next Steps:**
1. **Packaging**: Create platform-specific installers (MSI, AppImage, DMG).
2. **Benchmarking**: Execute the benchmark harness on real datasets and publish results.
3. **Fuzzing**: Implement libFuzzer tests for robustness.
4. **Release**: Tag v2.0.0 and publish to GitHub.

**Handoff Note:**
Documentation is now fully aligned with the v2.0.0 codebase. The roadmap clearly marks completed phases. The project structure is well-defined. The next major focus is on the "Polish and Release" phase, specifically creating installers and running comprehensive benchmarks.

### Update: 2025-12-28 (Session 20)

**Author:** OpenCode Interpreter

**Scope:** CLI Refactoring, Submodule Updates, Version Bump (v2.0.2).

**Current Status:**
- ✅ **Refactoring**: Resolved duplicate `main()` function in `cli/fo_cli.cpp` and removed temporary files (`fo_cli_new.cpp`, `fo_cli_corrupt.cpp`).
- ✅ **Submodules**: Updated `docs/SUBMODULES.md` dashboard with latest commit hashes from `git submodule status`.
- ✅ **Versioning**: Bumped version to `2.0.2` in `core/include/fo/core/version.hpp` and updated `CHANGELOG.md`.
- ✅ **Roadmap**: Updated `docs/ROADMAP.md` to mark v2.0.2 as current and documentation tasks as complete.
- ✅ **Build**: Verified `build.bat` runs successfully (ninja: no work to do).

**Next Steps:**
1. **Packaging**: Create installers (MSI, AppImage) as per Roadmap Phase 7.
2. **Benchmarking**: Run `benchmarks/fo_benchmarks.cpp` and analyze performance.
3. **Fuzzing**: Implement fuzz tests for robust input handling.

**Handoff Note:**
The codebase is clean, building, and documented. Submodule dashboard is current. We are ready to move into the final packaging and performance validation phase.
