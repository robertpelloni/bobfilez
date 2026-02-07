# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.2.1] - 2026-02-07

### Fixed
- **Submodules**: Updated `ai-file-sorter` submodule to a valid commit (`95a23a5`) after upstream force-push broke historical references.
- **Submodule Dashboard**: Regenerated `docs/SUBMODULES.md` with updated submodule states.

## [2.2.0] - 2026-02-05

### Added
- **MSI Installer**: Added WiX-based MSI installer generation (`scripts/package_msi.bat`).
- **Universal LLM Instructions**: Consolidated all agent instructions into `docs/UNIVERSAL_LLM_INSTRUCTIONS.md`.

### Changed
- **Documentation**: Renamed `docs/LLM_INSTRUCTIONS.md` to `docs/UNIVERSAL_LLM_INSTRUCTIONS.md` and updated all references.
- **Vision**: Updated `docs/VISION.md` to reflect Phase 7 completion and new Distribution phase.


## [2.1.0] - 2025-12-31

### Changed
- **Submodules**: Updated all 130+ submodules to their latest upstream versions, including major updates for FFmpeg, ImageMagick, libheif, and OpenCV.
- **Project Structure**: Re-analyzed and documented project structure in `docs/SUBMODULES.md`.
- **Dashboard**: Regenerated submodule dashboard with latest version information.

### Fixed
- **Submodule Consistency**: Resolved potential conflicts by merging upstream changes and ensuring all submodules are synchronized.

## [2.0.2] - 2025-12-29

### Fixed
- **Packaging**: Fixed `scripts/package.bat` to correctly use `PowerShell -ExecutionPolicy Bypass`.
- **Fuzz Testing**: Fixed build issues with fuzz tests missing standard headers in some environments.
- **CI/CD**: Added robust GitHub Actions release workflow (`.github/workflows/release.yml`).

## [2.0.1] - 2025-12-29

### Changed
- **Submodule Dashboard**: Regenerated `docs/SUBMODULES.md` with latest submodule status.
- **Versioning**: Bumped version to 2.0.1.

## [2.0.0] - 2025-12-28

### Added
- **Portable Packaging**: Scripts for creating distributable ZIP archives.
    - `scripts/package.bat`: Windows packaging script.
    - `scripts/package.sh`: Linux/macOS packaging script.
    - Generates `filez-{version}-{platform}.zip` with CLI, docs, and license.
- **Phase 7 Completion**: All documentation and packaging infrastructure in place.

### Changed
- **Major Version Bump**: v2.0.0 marks feature-complete status for CLI tool.
    - 15+ CLI commands
    - 59 passing tests
    - Comprehensive documentation (User Manual, Developer Guide, API Reference)
    - Portable packaging scripts

## [1.9.0] - 2025-12-28

### Added
- **API Reference Documentation**: Doxygen configuration and API guide.
    - `docs/Doxyfile`: Doxygen configuration for HTML API docs.
    - `docs/API_REFERENCE.md`: Developer guide to core interfaces and classes.
    - Documents IFileScanner, IHasher, IMetadataProvider, Engine, Registry, Database.
    - Usage examples for each component.

### Documentation
- Complete Phase 7 documentation tasks (User Manual, Developer Guide, API Reference).

## [1.8.0] - 2025-12-28

### Added
- **Integration Tests**: End-to-end workflow tests in `tests/test_integration.cpp`.
    - Scan directory and verify file count.
    - Find duplicates with identical content.
    - Export to JSON and verify structure.
    - Incremental scan only processes new files.
    - Extension filtering tests.
    - Duplicate group size verification.
    - Export to file creates valid JSON.
- **Test Suite**: Now 59 tests total across 7 test suites (all passing).

### Changed
- **Roadmap**: Updated to reflect documentation and testing progress.

## [1.7.0] - 2025-12-28

### Added
- **Database Unit Tests**: Comprehensive test suite in `tests/test_database.cpp`.
    - DatabaseTest (5 tests): open/close, in-memory DB, migrations, execute, query.
    - FileRepositoryTest (13 tests): CRUD operations, hashes, tags, paths.
    - Total test count increased from 34 to 52 (all passing).

### Changed
- **Test Coverage**: Expanded test coverage across all major components.

## [1.6.0] - 2025-12-28

### Added
- **Developer Guide**: Comprehensive documentation at `docs/DEVELOPER_GUIDE.md`.
    - Prerequisites and build instructions for all platforms.
    - Project architecture and component overview.
    - Registry pattern explanation with code examples.
    - Step-by-step guide for adding new providers.
    - Testing guide with Google Test patterns.
    - Code style conventions and contributing guidelines.

## [1.5.0] - 2025-12-28

### Added
- **User Manual**: Comprehensive documentation at `docs/USER_MANUAL.md`.
    - Installation instructions for Windows/Linux/macOS.
    - Quick start guide with 5-step workflow.
    - Detailed command reference with examples and comparison tables.
    - Common workflows: photo organization, incremental scans, OCR.
    - Troubleshooting section with solutions.
- **RuleEngine Unit Tests**: 12 new tests in `tests/test_rule_engine.cpp`.
    - Template variable expansion tests ({year}, {month}, {day}, {name}, {ext}, {parent}).
    - Rule matching with conditions and tag filtering.
    - Destination path handling and first-matching-rule behavior.

### Fixed
- **Test API Updates**: Fixed outdated API calls in test files (`list()` → `names()`, `full()` → `strong()`).

### Changed
- **Test Suite**: Now 34 tests total across 4 test suites (all passing).

## [1.4.0] - 2025-12-28

### Added
- **Submodule Dashboard Overhaul**: Comprehensive documentation of 130+ submodules with categories.
    - Organized by category: Image/Media, Metadata, Hashing, Database, Compression, Networking, etc.
    - Full version info, commit hashes, and repository URLs.
    - Project structure documentation with component descriptions.

### Changed
- **Merged Feature Branches**: Consolidated `feature/project-analysis-and-refactoring` into main.
- **Updated All Submodules**: Synced 130+ submodules to latest upstream commits.
    - FFmpeg updated to n8.1-dev
    - ImageMagick updated to 7.1.2-12
    - libheif updated to v1.20.1+
    - mpv updated to v0.41.0+
    - sqlite updated to 3.51.0+
    - raylib updated to 5.5+
    - radare2 updated to 6.0.7+
    - vlc updated to 4.0.0-dev

### Documentation
- **AGENTS.md**: Streamlined to 24-line actionable reference.
- **docs/SUBMODULES.md**: Complete rewrite with categorized submodule listing.
- **docs/LLM_INSTRUCTIONS.md**: Referenced as central AI instruction source.

## [1.3.0] - 2025-12-28

### Added
- **Thumbnail Support for HTML Export**: Added thumbnail generation for image files in HTML reports.
    - New `--thumbnails` CLI option for the `export` command.
    - Generates base64-encoded JPEG thumbnails embedded directly in HTML.
    - Uses Windows GDI+ for high-quality thumbnail generation.
    - Supports common image formats: JPEG, PNG, BMP, GIF, TIFF.
    - Thumbnails are 150x150 pixels by default, maintaining aspect ratio.

## [1.2.0] - 2025-12-28

### Added
- **ADS Cache for Windows**: Implemented Alternate Data Streams (ADS) caching for NTFS.
    - New `--use-ads-cache` CLI option to enable hash caching in file streams.
    - Stores hash values directly in the file's ADS (e.g., `file.jpg:fo_cache`).
    - Automatically invalidates cache when file mtime changes.
    - Supports multiple hash types per file.
    - Only works on NTFS/ReFS volumes; gracefully falls back on other filesystems.

## [1.1.0] - 2025-12-28

### Added
- **JSON Output for All Commands**: Extended `--format=json` support to all CLI commands:
    - `similar` - JSON output with query, hash, algorithm, threshold, and matches.
    - `classify` - JSON output with file paths and classification results.
    - `organize` - JSON output with dry_run flag, total files, and move operations.
    - `delete-duplicates` - JSON output with strategy, groups, kept/deleted counts, and results.
    - `rename` - JSON output with pattern, total files, and rename operations.
    - `undo` - JSON output with success status and operation details.
    - `history` - JSON output with operation records (already added in v0.6.1).

## [1.0.0] - 2025-12-28

### Added
- **v1.0.0 Release**: First major stable release with comprehensive feature set.
- **Complete CLI**: 15+ commands for file organization, duplicate detection, and image analysis.
- **Submodule Dashboard**: Documentation for 100+ library dependencies with versions.
- **Project Structure**: Comprehensive documentation of the codebase layout.

## [0.9.0] - 2025-12-28

### Added
- **Perceptual Hash Algorithms**: Added multiple perceptual hash algorithms for image similarity detection.
    - `dhash` (difference hash) - fast, gradient-based.
    - `phash` (DCT-based perceptual hash) - robust to scaling and minor edits.
    - `ahash` (average hash) - simple and fast.
- **CLI Enhancements**:
    - New `--phash=<algo>` option to select perceptual hash algorithm.
    - New `--list-phash` option to list available algorithms.

## [0.8.0] - 2025-12-28

### Added
- **Undo Support**: All file operations (move, rename, delete) are now logged to the database.
    - New `undo` command to reverse the last operation.
    - New `history` command to view operation history.
    - Database migration V3 adds `operation_log` table.
- **OperationRepository**: New repository class for managing operation history.

## [0.7.0] - 2025-12-28

### Added
- **Qt GUI**: Complete GUI refactoring with fo_core Engine integration.
    - Scan directories with visual progress.
    - Find duplicates with tabular display.
    - Export results to HTML/JSON/CSV.
    - Scanner and hasher selection dropdowns.
- **Unit Tests**: Added tests for Exporter, Hasher, and Scanner modules.

## [0.6.1] - 2025-12-28

### Added
- **JSON Output Mode**: `--format=json` now works with `scan`, `duplicates`, `hash`, and `metadata` commands.
- **Public Escape Functions**: `Exporter::json_escape`, `csv_escape`, `html_escape` are now public static methods.

## [0.6.0] - 2025-12-28

### Added
- **Export Feature**: New `export` command for exporting scan results.
    - Supports JSON, CSV, and HTML formats (`--format=json|csv|html`).
    - Includes file statistics, duplicates summary, and full file listing.
    - Output to stdout or file (`--output=<path>`).
- **Export Module**: Added `core/include/fo/core/export.hpp` and `core/src/export.cpp`.
- **CLI**: Added `--output=<path>` option for export file destination.

### Changed
- Updated `print_usage()` to document the new export command.

## [0.5.1] - 2025-12-27

### Added
- **Submodule Dashboard**: Automated generation of `docs/SUBMODULES.md` listing all submodules, versions, and build info.
- **Documentation**: Added project structure explanation to the dashboard.

### Changed
- **Submodules**: Updated all submodules to their latest upstream versions.
- **Build**: Verified build stability with updated dependencies.

## [0.5.0] - 2025-12-27

### Added
- **Smart Organization**: Implemented `RuleEngine` for organizing files based on metadata templates (e.g., `/Photos/{year}/{month}/`).
- **YAML Rules**: Added support for loading organization rules from YAML files (`--rules=rules.yaml`).
- **Batch Operations**:
    - `delete-duplicates`: Bulk deletion of duplicates with strategies (`oldest`, `newest`, `shortest`, `longest`).
    - `rename`: Bulk renaming using patterns (e.g., `{year}-{month}-{day}_{name}.{ext}`).
- **CLI Commands**: Added `organize`, `delete-duplicates`, and `rename` commands.
- **Dependencies**: Added `yaml-cpp` to `vcpkg.json`.

## [0.4.0] - 2025-12-25

### Added
- **AI Classification**: Implemented `OnnxRuntimeClassifier` for image tagging using ONNX models.
- **Perceptual Hashing**: Implemented `OpencvPerceptualHasher` (dHash) for finding visually similar images.
- **Benchmarking**: Added Google Benchmark harness (`benchmarks/fo_benchmarks.cpp`) for scanners and hashers.
- **CLI Commands**: Added `classify` (AI tagging) and `similar` (perceptual hash search) commands.
- **CLI Commands**: Added `download-models` command (stub) to help users acquire AI models.
- **Database**: Added `tags` and `file_tags` tables (Migration 2) for storing AI classification results.
- **Persistence**: `classify` command now saves tags to the database.
- **Dependencies**: Added `opencv`, `onnxruntime`, and `benchmark` to `vcpkg.json`.

## [0.3.0] - 2025-12-25

### Added
- **CI/CD**: Added GitHub Actions workflow for Windows and Linux builds.
- **Dependencies**: Added `vcpkg.json` for managing Tesseract, BLAKE3, Exiv2, and SQLite3.
- **Providers**: Implemented `Blake3Hasher` and `Exiv2MetadataProvider` (feature-guarded).
- **Database Layer**: Implemented SQLite integration with `DatabaseManager`, `FileRepository`, and `DuplicateRepository`.
- **Persistence**: Scanned files and duplicate groups are now persisted to `fo.db` (configurable via `--db`).
- **Schema**: Initial V1 schema with `files`, `file_hashes`, `duplicate_groups`, and `duplicate_members` tables.
- **CLI**: Added `--db=<path>` option.
- **CLI Commands**: Implemented `hash`, `metadata`, and `ocr` commands.
- **OCR**: Implemented `TesseractOCRProvider` (requires `FO_HAVE_TESSERACT` build flag).
- **Versioning**: Added `--version` flag to CLI; build system now reads version from `VERSION.md`.
- **Repositories**: Added `IgnoreRepository` and `ScanSessionRepository`.
- **Submodules**: Fixed missing `vcpkg` submodule mapping.

## [0.2.0] - 2025-12-25

### Added
- **Submodule Dashboard**: Created `docs/SUBMODULES.md` listing all submodules and their versions.
- **Versioning**: Introduced `VERSION.md` as the single source of truth for the project version.
- **LLM Instructions**: Centralized AI agent instructions in `docs/LLM_INSTRUCTIONS.md`.

### Changed
- Updated `AGENTS.md` to reference the new universal instructions.
- Updated submodules to their latest local state.

### Fixed
- Addressed missing submodule mapping for `vcpkg`.

## [0.1.0] - 2025-11-14

### Added
- Initial release of filez CLI.
- Scanners: `std`, `win32`, `dirent`.
- Basic benchmarking tools.
