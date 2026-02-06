# Gemini Instructions

> **IMPORTANT**: Please refer to [docs/UNIVERSAL_LLM_INSTRUCTIONS.md](docs/UNIVERSAL_LLM_INSTRUCTIONS.md) for the universal instructions, versioning protocol, and coding standards.

## Gemini-Specific Guidelines

### Strengths
- **Large Context**: Utilize full context window for comprehensive analysis.
- **Multi-file**: Analyze relationships across the entire codebase.
- **Dependencies**: Track 100+ submodules in `libs/`.
- **Research**: Search for best practices and solutions.

### Workflow
1. Always read `VERSION.md` first to know current version.
2. Check `CHANGELOG.md` for recent changes.
3. Review `docs/ROADMAP.md` for pending features.
4. Update `AGENTS.md` handoff log when done.

### Version Updates
When updating the version:
1. Edit `VERSION.md` (single line with version string).
2. Add entry to `CHANGELOG.md` with date.
3. Commit: `git commit -m "chore: bump version to X.Y.Z"`
4. Push: `git push`

### Autonomy
- Proceed with implementation using available tools.
- Don't stop for confirmation unless blocked.
- Complete features, commit, and continue to next task.
- Fix errors encountered along the way.

