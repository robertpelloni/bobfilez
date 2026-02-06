# GitHub Copilot Instructions

> **IMPORTANT**: Please refer to [docs/UNIVERSAL_LLM_INSTRUCTIONS.md](../docs/UNIVERSAL_LLM_INSTRUCTIONS.md) for the universal instructions, versioning protocol, and coding standards.

## Copilot-Specific Guidelines

### Strengths
- **Inline Completion**: Context-aware code suggestions.
- **Terminal Commands**: Use `run_in_terminal` for builds and git operations.
- **Quick Fixes**: Rapid iteration on compiler errors.
- **Patterns**: Infer patterns from existing code.

### Workflow
1. Always read `VERSION.md` first to know current version.
2. Check `CHANGELOG.md` for recent changes.
3. Review `docs/ROADMAP.md` for pending features.
4. Update `AGENTS.md` handoff log when done.

### Build Commands
```powershell
# Quick build (Windows)
build.bat

# Manual build
cmake -S . -B build -G Ninja && cmake --build build

# Run tests
.\build\tests\fo_tests.exe

# Run single test
.\build\tests\fo_tests.exe --gtest_filter=*TestName*

# CLI usage
.\build\cli\fo_cli.exe --help
```

### Version Updates
When updating the version:
1. Edit `VERSION.md` (single line with version string).
2. Add entry to `CHANGELOG.md` with date.
3. Commit: `git commit -m "chore: bump version to X.Y.Z"`
4. Push: `git push`

### Code Style
- **C++20** with `std::filesystem`, `std::chrono`, `std::optional`
- **Naming**: `snake_case` for functions/variables, `CamelCase` for classes
- **Headers**: `.hpp` extension, `#pragma once`
- **Platform code**: Guard with `#ifdef _WIN32`
- **Optional deps**: Guard with `FO_HAVE_*` defines

### Autonomy
- Proceed with implementation using available tools.
- Don't stop for confirmation unless blocked.
- Complete features, commit, and continue to next task.
- Fix errors encountered along the way.
