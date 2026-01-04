---
name: file-organizer
description: Intelligently organizes your files and folders across your computer by understanding context, finding duplicates, suggesting better structures, and automating cleanup tasks. Reduces cognitive load and keeps your digital workspace tidy without manual effort.
---

# File Organizer

This skill acts as your personal organization assistant, helping you maintain a clean, logical file structure across your computer without the mental overhead of constant manual organization.

## When to Use This Skill

- Your Downloads folder is a chaotic mess
- You can't find files because they're scattered everywhere
- You have duplicate files taking up space
- Your folder structure doesn't make sense anymore
- You want to establish better organization habits
- You're starting a new project and need a good structure
- You're cleaning up before archiving old projects

## What This Skill Does

1. **Analyzes Current Structure**: Reviews your folders and files to understand what you have
2. **Finds Duplicates**: Identifies duplicate files across your system
3. **Suggests Organization**: Proposes logical folder structures based on your content
4. **Automates Cleanup**: Moves, renames, and organizes files with your approval
5. **Maintains Context**: Makes smart decisions based on file types, dates, and content
6. **Reduces Clutter**: Identifies old files you probably don't need anymore

## How to Use

### From Your Home Directory

```
cd ~
```

Then run Claude Code and ask for help:

```
Help me organize my Downloads folder
```

```
Find duplicate files in my Documents folder
```

```
Review my project directories and suggest improvements
```

### Specific Organization Tasks

```
Organize these downloads into proper folders based on what they are
```

```
Find duplicate files and help me decide which to keep
```

```
Clean up old files I haven't touched in 6+ months
```

```
Create a better folder structure for my [work/projects/photos/etc]
```

## Instructions

When a user requests file organization help:

1. **Understand the Scope**
   
   Ask clarifying questions:
   - Which directory needs organization? (Downloads, Documents, entire home folder?)
   - What's the main problem? (Can't find things, duplicates, too messy, no structure?)
   - Any files or folders to avoid? (Current projects, sensitive data?)
   - How aggressively to organize? (Conservative vs. comprehensive cleanup)

2. **Analyze Current State**
   
   Review the target directory:
   ```bash
   # Get overview of current structure
   ls -la [target_directory]
   
   # Check file types and sizes
   find [target_directory] -type f -exec file {} \; | head -20
   
   # Identify largest files
   du -sh [target_directory]/* | sort -rh | head -20
   
   # Count file types
   find [target_directory] -type f | sed 's/.*\.//' | sort | uniq -c | sort -rn
   ```
   
   Summarize findings:
   - Total files and folders
   - File type breakdown
   - Size distribution
   - Date ranges
   - Obvious organization issues

3. **Identify Organization Patterns**
   
   Based on the files, determine logical groupings:
   
   **By Type**:
   - Documents (PDFs, DOCX, TXT)
   - Images (JPG, PNG, SVG)
   - Videos (MP4, MOV)
   - Archives (ZIP, TAR, DMG)
   - Code/Projects (directories with code)
   - Spreadsheets (XLSX, CSV)
   - Presentations (PPTX, KEY)
   
   **By Purpose**:
   - Work vs. Personal
   - Active vs. Archive
   - Project-specific
   - Reference materials
   - Temporary/scratch files
   
   **By Date**:
   - Current year/month
   - Previous years
   - Very old (archive candidates)

4. **Find Duplicates**
   
   When requested, search for duplicates:
   ```bash
   # Find exact duplicates by hash
   find [directory] -type f -exec md5 {} \; | sort | uniq -d
   
   # Find files with same name
   find [directory] -type f -printf '%f\n' | sort | uniq -d
   
   # Find similar-sized files
   find [directory] -type f -printf '%s %p\n' | sort -n
   ```
   
   For each set of duplicates:
   - Show all file paths
   - Display sizes and modification dates
   - Recommend which to keep (usually newest or best-named)
   - **Important**: Always ask for confirmation before deleting

5. **Propose Organization Plan**
   
   Present a clear plan before making changes:
   
   ```markdown
   # Organization Plan for [Directory]
   
   ## Current State
   - X files across Y folders
   - [Size] total
   - File types: [breakdown]
   - Issues: [list problems]
   
   ## Proposed Structure
   
   ```
   [Directory]/
   ├── Work/
   │   ├── Projects/
   │   ├── Documents/
   │   └── Archive/
   ├── Personal/
   │   ├── Photos/
   │   ├── Documents/
   │   └── Media/
   └── Downloads/
       ├── To-Sort/
       └── Archive/
   ```
   
   ## Changes I'll Make
   
   1. **Create new folders**: [list]
   2. **Move files**:
      - X PDFs → Work/Documents/
      - Y images → Personal/Photos/
      - Z old files → Archive/
   3. **Rename files**: [any renaming patterns]
   4. **Delete**: [duplicates or trash files]
   
   ## Files Needing Your Decision
   
   - [List any files you're unsure about]
   
   Ready to proceed? (yes/no/modify)
   ```

6. **Execute Organization**
   
   After approval, organize systematically:
   
   ```bash
   # Create folder structure
   mkdir -p "path/to/new/folders"
   
   # Move files with clear logging
   mv "old/path/file.pdf" "new/path/file.pdf"
   
   # Rename files with consistent patterns
   # Example: "YYYY-MM-DD - Description.ext"
   ```
   
   **Important Rules**:
   - Always confirm before deleting anything
   - Log all moves for potential undo
   - Preserve original modification dates
   - Handle filename conflicts gracefully
   - Stop and ask if you encounter unexpected situations

7. **Provide Summary and Maintenance Tips**
   
   After organizing:
   
   ```markdown
   # Organization Complete! ✨
   
   ## What Changed
   
   - Created [X] new folders
   - Organized [Y] files
   - Freed [Z] GB by removing duplicates
   - Archived [W] old files
   
   ## New Structure
   
   [Show the new folder tree]
   
   ## Maintenance Tips
   
   To keep this organized:
   
   1. **Weekly**: Sort new downloads
   2. **Monthly**: Review and archive completed projects
   3. **Quarterly**: Check for new duplicates
   4. **Yearly**: Archive old files
   
   ## Quick Commands for You
   
   ```bash
   # Find files modified this week
   find . -type f -mtime -7
   
   # Sort downloads by type
   [custom command for their setup]
   
   # Find duplicates
   [custom command]
   ```
   
   Want to organize another folder?
   ```

## Examples

### Example 1: Organizing Downloads (From Justin Dielmann)

**User**: "My Downloads folder is a mess with 500+ files. Help me organize it."

**Process**:
1. Analyzes Downloads folder
2. Finds patterns: work docs, personal photos, installers, random PDFs
3. Proposes structure:
   - Downloads/
     - Work/
     - Personal/
     - Installers/ (DMG, PKG files)
     - Archive/
     - ToSort/ (things needing decisions)
4. Asks for confirmation
5. Moves files intelligently based on content and names
6. Results: 500 files → 5 organized folders

### Example 2: Finding and Removing Duplicates

**User**: "Find duplicate files in my Documents and help me decide which to keep."

**Output**:
```markdown
# Found 23 Sets of Duplicates (156 MB total)

## Duplicate Set 1: "proposal.pdf"
- `/Documents/proposal.pdf` (2.3 MB, modified: 2024-03-15)
- `/Documents/old/proposal.pdf` (2.3 MB, modified: 2024-03-15)
- `/Desktop/proposal.pdf` (2.3 MB, modified: 2024-03-10)

**Recommendation**: Keep `/Documents/proposal.pdf` (most recent in correct location)
Delete the other 2 copies?

[Continue for all duplicates...]
```

### Example 3: Restructuring Projects Folder

**User**: "Review my ~/Projects directory and suggest improvements."

**Output**:
```markdown
# Analysis of ~/Projects

## Current Structure Issues
- Mix of active and archived projects (3+ years old)
- No consistent naming convention
- Some projects at root, others in random subfolders
- Duplicate folders (project-name, project-name-old, project-name-v2)

## Proposed Structure

```
Projects/
├── Active/
│   ├── client-work/
│   ├── side-projects/
│   └── learning/
├── Archive/
│   ├── 2022/
│   ├── 2023/
│   └── 2024/
└── Templates/
```

## Specific Changes
1. Move 12 projects not touched since 2022 → Archive/
2. Consolidate 4 duplicate project folders
3. Rename projects to consistent format: "client-name-project-name"
4. Create Archive for old work

Want me to implement this?
```

### Example 4: Organizing Photos by Date

**User**: "Organize my photo folders by year and month."

**Output**: Creates structure like:
```
Photos/
├── 2023/
│   ├── 01-January/
│   ├── 02-February/
│   └── ...
├── 2024/
│   ├── 01-January/
│   └── ...
└── Unsorted/
```

Then moves photos based on EXIF data or file modification dates.

## Common Organization Tasks

### Downloads Cleanup
```
Organize my Downloads folder - move documents to Documents, 
images to Pictures, keep installers separate, and archive files 
older than 3 months.
```

### Project Organization
```
Review my Projects folder structure and help me separate active 
projects from old ones I should archive.
```

### Duplicate Removal
```
Find all duplicate files in my Documents folder and help me 
decide which ones to keep.
```

### Desktop Cleanup
```
My Desktop is covered in files. Help me organize everything into 
my Documents folder properly.
```

### Photo Organization
```
Organize all photos in this folder by date (year/month) based 
on when they were taken.
```

### Work/Personal Separation
```
Help me separate my work files from personal files across my 
Documents folder.
```

## Pro Tips

1. **Start Small**: Begin with one messy folder (like Downloads) to build trust
2. **Regular Maintenance**: Run weekly cleanup on Downloads
3. **Consistent Naming**: Use "YYYY-MM-DD - Description" format for important files
4. **Archive Aggressively**: Move old projects to Archive instead of deleting
5. **Keep Active Separate**: Maintain clear boundaries between active and archived work
6. **Trust the Process**: Let Claude handle the cognitive load of where things go

## Best Practices

### Folder Naming
- Use clear, descriptive names
- Avoid spaces (use hyphens or underscores)
- Be specific: "client-proposals" not "docs"
- Use prefixes for ordering: "01-current", "02-archive"

### File Naming
- Include dates: "2024-10-17-meeting-notes.md"
- Be descriptive: "q3-financial-report.xlsx"
- Avoid version numbers in names (use version control instead)
- Remove download artifacts: "document-final-v2 (1).pdf" → "document.pdf"

### When to Archive
- Projects not touched in 6+ months
- Completed work that might be referenced later
- Old versions after migration to new systems
- Files you're hesitant to delete (archive first)

## Related Use Cases

- Setting up organization for a new computer
- Preparing files for backup/archiving
- Cleaning up before storage cleanup
- Organizing shared team folders
- Structuring new project directories


## File Organizer Roadmap

### Current Architecture Observations
- **Monolithic worker** – the `Worker` class currently handles filesystem traversal, hashing, EXIF parsing, duplicate grouping, logging, and SQLite IO in one thread, which limits testability and portability.
- **Redundant scanning strategies** – four directory-walk implementations run sequentially (C++17 recursive iterator, manual stack recursion, `dirent`, Win32 APIs). Only one per platform should exist.
- **Ad hoc persistence** – `index.db` and `ignore.db` are opened without schema management, migrations, or a data-access layer; SQL lives inside worker logic.
- **UI limitations** – Qt Widgets UI is tightly coupled to the worker thread, making it difficult to port to non-desktop targets or ship headless/CLI builds.
- **Dependency sprawl** – dozens of vendored libs live under `libs/` with no documented ownership, licensing notes, or build story.
- **Testing & release gaps** – no unit tests, integration tests, or CI; build is VS solution only.

### Phase 0 – Architecture & Platform Preparation (2–3 weeks)
1. **Service decomposition**: split the current worker into scanning, metadata, hashing, duplicate analysis, logging, and persistence services with clear contracts.
2. **Filesystem abstraction**: create a single traversal interface with per-platform backends; remove redundant walkers.
3. **Persistence layer**: define SQLite schema (versioned migrations, WAL, indexes) and wrap DB access behind repository classes.
4. **Dependency audit**: inventory every project under `libs/`, record license, build instructions, current usage, and remove or archive unused code.
5. **UI/engine separation spike**: define a shared “core engine” API (CLI-first) that exposes scanning and automation without any Qt dependencies.

### Phase 1 – Core Feature Hardening (4–5 weeks)
1. **Hashing pipeline**: implement pluggable hash strategies (chunk hash, full-file, future perceptual hash) with regression tests verifying stability across OSes.
2. **Metadata fusion**: encapsulate filename, EXIF, filesystem timestamp heuristics into a scoring engine that retains raw signals for traceability.
3. **Duplicate review UX**: replace QTextEdit output with a data model supporting paging/filtering, preview panes, auto-select rules, and export.
4. **Structured logging & metrics**: emit timing and queue-depth metrics to identify hotspots; guard long-running tasks with cancellation hooks.
5. **UI modernization initiative**:
   - **Decouple core from UI** by exposing a gRPC/CLI/IPC boundary so any interface can drive the engine.
   - **Evaluate cross-platform shells** (GTK4, Dear ImGui + SDL, Electron/tauri) for future desktop UX, prioritizing permissive licensing and simple deployment.
   - **Prototype headless console UI** to validate the separation and enable server-mode scans.
   - Decision milestone: pick target UI stack or formally support multiple thin clients.

### Phase 2 – Feature Expansion (iterative sprints)
1. **Regex/scriptable renamer** with dry-run, undo, and preset support.
2. **OCR & AI classification** by leveraging vendored OCR libs (TinyOCR/Tesseract) and optional model adapters; run work asynchronously so dedupe scans remain responsive.
3. **ADS and metadata preservation** with platform-aware handlers (NTFS ADS, Linux xattrs) and user-configured policies.
4. **Pluggable source providers** for network shares, removable media, and cloud mounts respecting ignore rules and throttling.
5. **Automation & APIs**: expose CLI/REST endpoints so other apps (or future subagents) can orchestrate scans programmatically.

### Phase 3 – Quality, Tooling & Release
1. **Test harnesses**: introduce Catch2/GoogleTest suites plus fixture directories for regression testing duplicate detection, hashing, and metadata logic.
2. **CI/CD**: add GitHub Actions (Windows + Linux) to build, test, package artifacts, and run linting.
3. **Telemetry & crash reporting** (opt-in) to gather anonymized performance/error data.
4. **Documentation & onboarding**: publish architecture decision records, dependency build notes, contribution guide, and roadmap.

### Immediate Next Steps
1. Finalize module boundary diagram for engine vs. UI vs. persistence.
2. Draft the UI modernization RFC comparing GTK, Dear ImGui/SDL, and Electron/tauri with pros/cons for maintainability and multi-platform reach.
3. Produce dependency audit spreadsheet and schema migration plan.

---

## AI Multimodel Development Playbook

### Goals
- Support “heavy AI assisted development” with orchestrated multi-model subagents that can plan, code, review, and test within filez.
- Ensure the codebase, tooling, and documentation give agents clear, safe entry points and guardrails.

### Knowledge & Context Surfaces
1. **Canonical docs**: keep skill/rules (`FILE_ORGANIZER_SKILL.md`), roadmap, API references, and architecture diagrams up to date.
2. **Structured metadata**: maintain machine-readable descriptors (e.g., JSON/YAML) for modules, commands, and naming conventions so subagents can query them.
3. **Decision logs**: use ADRs and CHANGELOG to capture rationale and prevent re-litigating choices.

### Workflow Topology for Subagents
1. **Planner agent**: interprets tasks, consults roadmap, and creates execution plans with resource constraints.
2. **Builder agents** (per domain: core engine, UI, persistence, AI integrations) operate on isolated branches, referencing module-specific docs.
3. **Reviewer agent**: validates diffs against coding standards, tests, and architecture guidelines.
4. **Ops agent**: manages CI pipelines, dependency updates, and releases.

### Tooling & Infrastructure
- **Unified CLI** to run scans, tests, formatters, and packaging tasks (e.g., via CMake + vcpkg/conan). Provide sample commands in docs.
- **Sandboxed environments** (Docker/Dev Containers) mirroring CI to give AI agents reproducible setups.
- **API surface for orchestration**: provide stable scripts/commands (e.g., `fileorganizer scan --config config.json`) so subagents can automate workflows.
- **Observability hooks**: expose structured logs/metrics so agents can detect regressions.

### Guardrails
- Enforce code-formatting (clang-format), linting, and unit tests in CI gates.
- Require architectural review for changes touching the engine-core or UI boundary.
- Maintain security review checklist before integrating new third-party AI services.

### Execution Blueprint
1. **Bootstrap**: document the multi-agent workflow, required tools, and environment provisioning.
2. **Automate**: create scripts for plan generation, branch creation, and PR templating to guide autonomous work.
3. **Monitor**: set up dashboards (CI, code coverage, issue triage) so human overseers can supervise AI activity.
4. **Iterate**: collect metrics on agent performance (latency, defect rate) and refine prompts/tooling accordingly.

With the roadmap and playbook embedded here, future contributors—human or AI—have a single canonical reference that captures architectural priorities, UI modernization plans, and the orchestration model for multi-agent development.
