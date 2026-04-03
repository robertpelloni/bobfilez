# Changelog

## [4.7.0] - 2026-04-03

### Added — The "OmniMount" Release

#### 💽 OmniMount VFS (Dokany/FUSE Integration)
- **`omni_mount_interface.hpp` & `omni_mount.cpp`**: Implemented a kernel-level bridge using Dokany (Windows) and FUSE (Linux).
- Allows bobfilez Virtual File Systems (Live Folders, Semantic Searches, Cloud Drives, and Data Swarms) to be mounted as native OS drive letters (e.g., `Z:\`).
- **`OmniMountPanel.qml`**: A comprehensive UI for managing active virtual mounts, configuring drive letters, and setting Read/Write or Read-Only permissions.
- You can now run a semantic search for "Invoices from 2023", mount the result as `Z:\`, and open those scattered files natively in any third-party application like Microsoft Word or Excel.

### Version
- Bumped to **4.7.0**.

## [4.6.0] - 2026-04-03

### Added — The "OmniOracle & Time Machine" Release

#### 🤖 OmniOracle AI Copilot (Local RAG)
- **`omni_oracle_interface.hpp` & `omni_oracle.cpp`**: Implemented a local Retrieval-Augmented Generation (RAG) engine.
- Users can now chat directly with their filesystem. OmniOracle uses the `DocumentEmbedder` (BERT) to read PDFs/DOCXs, summarizes contents, and suggests automated `OmniFlow` actions.
- **`OraclePanel.qml`**: A sleek chat interface integrated into the OmniShell Taskbar, complete with markdown support, file citation chips, and actionable trigger buttons.

#### ⏳ OmniTimeMachine (Continuous Data Protection)
- **`time_machine.cpp`**: Captures binary deltas (using librsync/zstd concepts) as files are modified, allowing instantaneous rewind without duplicating full file sizes.
- **`TimeMachinePanel.qml`**: A visual timeline scrubber that lets users review modification history and restore old states.

### Version
- Bumped to **4.6.0**.

## [4.4.0] - 2026-04-03

### Added — The "Visual Automation" Release

#### 🌊 OmniFlow Visual Automation Engine
- **`omniflow_engine.cpp`**: Implemented a robust C++ execution engine for custom file workflows.
- **`OmniFlowPanel.qml`**: A stunning Node-based Visual Editor (similar to Unreal Blueprints or Apple Shortcuts).
- **Node Types**: Users can drag, drop, and connect `Triggers` (Folder Watcher, Timers), `Conditions` (Regex, AI Semantic Tags), and `Actions` (Convert, Encrypt, OCR, Move).

#### 🕸️ OmniGraph Knowledge Database
- **Semantic Mapping**: Shifted from pure folder hierarchies to a Personal Knowledge Graph for files (`omni_graph.cpp`). Files are linked via semantic relationships (`DEPICTS`, `LOCATED_IN`, `MENTIONS`).
- **`OmniGraphPanel.qml`**: Interactive force-directed graph visualization of your file relationships.

### Version
- Bumped to **4.4.0**.

## [3.0.0] - 2026-04-03

### Added — The "Next-Gen Data Custodian" Release

#### 🖥️ OmniShell: Windows 11 Shell Replacement
- **`Taskbar.qml` & `StartMenu.qml`**: Fully functional shell UI with Acrylic/Mica effects, system tray, and AI-powered recommendations.
- **`DesktopIcons.qml`**: Grid-based desktop management system.
- **Multi-Window Coordinator**: `main.qml` now manages separate process-like windows for all sub-tools.

#### ⚡ High-Performance & Security
- **Zero-Copy Sorter**: Integrated ReFS Block Cloning (Windows) and Reflink (Linux) for near-instant file duplication on the same volume.
- **Data Topology Map**: SequoiaView-style Tree-map visualization for disk usage analysis.
- **PII Sentinel**: Autonomous scanning for Social Security Numbers, Credit Cards, and API keys.
- **Forensic Audit Ledger**: Immutable, append-only operation log with tamper-detection hashes (Migration 5).

#### 🧹 Autonomous Cleanup
- **Digital Rot Agent**: AI identifying obsolete installers, temporary caches, and forgotten data.
- **Visual Discovery**: Tinder-style "swipe" interface for near-duplicate photo cleanup.

#### 🕰️ Unification & Parity
- **Nexus Master Clock**: Unified scheduler for priority-based task arbitration across all engines.
- **Java 21 Parity**: Brought the Java project up to v3.0 architectural parity with new interfaces and command features.

### Version
- Bumped to **3.0.0**.

## [2.9.0] - 2026-04-03

### Added — Vector Semantic Search & Backend Finalization

#### 🧠 Vector Semantic Search
- **`clip_search_interface.hpp` & `clip_search_engine.cpp`**: Implemented an offline natural-language-to-image search engine using OpenAI's CLIP model.
- Uses ONNX Runtime C++ API to run `clip-image` and `clip-text` models entirely locally for complete privacy.
- Parses user queries via BPE Tokenizer, converts them to 512-dimensional float vectors, and computes L2-normalized cosine similarity against cached image vectors.
- **UI Integration**: Added a new "Semantic Search" mode to `SearchPanel.qml` featuring a dynamic threshold slider and a GridView thumbnail layout for image results with relevance score badges.

#### ⚙️ Core Engines Implemented
- **`batch_rename_engine.cpp`**: Fully implemented the `apply()` methods for all 13 Batch Rename rules (Replace, Insert, Delete, Trim, Case, Number, DateTime, Metadata, Extension, Truncate, Sanitize, Hash, Transliterate, RegexGroups) using `std::regex` and string manipulation.
- **`search_engine.cpp`**: Fully implemented the core recursive folder traversal and content search loops, featuring multi-threading and PCRE2-compatible (via `<regex>`) text grepping.
- **`enhanced_fileops.cpp`**: Implemented `EnhancedCopyEngine::copy_single_enhanced()` with FastCopy-style custom buffering, TeraCopy-style checksum verification, and error-recovery callbacks.
- **`advanced_archive_manager.cpp`**: Implemented the `AdvancedArchiveManager` bindings mapped directly to the `7z` command-line executable for in-archive browsing, extraction, modification, testing, and volume splitting.
- **`hex_editor.cpp`**: Implemented the `HexBuffer` class backing the Hex Editor panel, using OS-level memory mapping (`MapViewOfFile` on Windows, `mmap` on Linux) to instantly load arbitrarily large files without RAM exhaustion. Added `DataInterpretation` parser for 30+ types.
- **`markdown_viewer.cpp`**: Implemented the `MarkdownRenderer::render()` function wrapping the fast `md4c` C API to convert Markdown AST to HTML, injecting CSS templating and external libraries (KaTeX, Mermaid, highlight.js) for the QML `WebEngineView`.
- **`conversion_engine.cpp`**: Added the `CalibreConverter` for ebooks and `GhostscriptConverter` for PDFs.
- **`FileModel`**: Implemented C++ context menu actions (`openFile`, `deleteFile`) using `QDesktopServices` and `<filesystem>`.

### Version
- Bumped to **2.8.0**.

## [2.7.0] - 2026-04-03

### Added — Enhanced File Operations (TeraCopy/FastCopy/UltraCopier/SuperCopier parity)

#### 📎 `EnhancedCopyEngine` (C++)
- **`enhanced_fileops_interface.hpp`**: `EnhancedCopyOptions` extends `CopyMoveOptions` with:
  - **FastCopy I/O tuning**: separate read/write buffer sizes (up to 512 MiB), dedicated read/write thread pools, `FILE_FLAG_NO_BUFFERING`, `FILE_FLAG_WRITE_THROUGH`, sequential hint, smart same-drive detection.
  - **TeraCopy verification**: per-file checksum after transfer (xxHash64/MD5/SHA-1/SHA-256/CRC-32), abort on mismatch.
  - **TeraCopy error handling**: `ErrorHandlerCb` callback per file error (`FileError` with type/source/dest/message), per-error decision: Skip/SkipAll/Retry/RetryAll/Overwrite/OverwriteAll/RenameAuto/RenameAll/Abort.
  - **TeraCopy post-actions**: open dest, eject drive, shutdown, sleep, logoff, play sound.
  - **TeraCopy favorites**: save source+dest pairs for 1-click operations.
  - **TeraCopy log**: per-job log file with per-file status, written to configurable directory.
  - **UltraCopier job queue**: multi-job deque, pause/resume/cancel individual jobs, drag reorder (`move_job_up`/`move_job_down`), save/load queue to disk.
  - **SuperCopier stats**: `TransferStats` with instantaneous/average/peak speed, ETA, elapsed, speed history ring-buffer (60 samples for graph).
  - **FastCopy estimate mode**: calculate total size and file count without copying.
  - **NTFS preservation**: ACL, ADS (Alternate Data Streams), compressed flag, encrypted flag.
  - **Date-based filtering**: `only_newer_than`, `only_modified_before`.
  - **Free-space check**: abort before starting if destination has insufficient space.

#### 🗃️ `AdvancedArchiveManager` (C++)
- Extends `ArchiveEngine` with **in-archive editing**:
  - `browse()`: list archive contents as virtual filesystem tree.
  - `extract_entries()`: extract specific files/folders only.
  - `add_to_archive()`: add files to existing archive at any path.
  - `delete_entries()`: remove entries (rebuilds archive).
  - `rename_entry()`: rename path inside archive.
  - `update_entry()`: replace entry with new file from disk.
  - `convert()`: convert between archive formats without full extract.
  - `test()`: verify all CRCs, return list of corrupt entries.
  - `repair_zip()`: attempt to recover corrupt ZIP archives.
  - `search_in_archives()`: find files by pattern without extracting.
  - `benchmark()`: compare compression ratio/speed across all algorithms.
  - `info()`: detailed archive metadata (format, ratio, solid, multivolume).
  - `create_multivolume()`: split archives across volumes.
  - `merge_volumes()`: reassemble split archives.

#### 🎨 `EnhancedFileOpsPanel.qml`
- **Copy/Move Tab** (TeraCopy+FastCopy+UltraCopier+SuperCopier combined):
  - Job queue panel: add multiple jobs, drag-reorder, pause/resume/cancel individual jobs, global pause/cancel.
  - Favorites panel: quick-add source+dest pairs.
  - Per-file list with status tabs (All / OK / Skip / Error / Pending), right-click error recovery menu (Retry/Skip/SkipAll/Overwrite/Rename).
  - Real-time stats display: current speed, peak speed, average speed, ETA, files done, data done, elapsed, failed.
  - **Live speed graph** (SuperCopier-style): Canvas-drawn 60-second bandwidth chart with fill gradient.
  - Per-job mini progress bars in queue.
  - Options panel: error handling, verification algorithm, I/O tuning (buffer sizes, threads, no-cache, write-through, smart mode), NTFS options, file filters, post-action.
- **Archive Tab** (7-Zip parity + in-archive browser/editor):
  - Create archive: format, method, level, dictionary, word size, threads, solid mode, SFX, AES-256 encryption with filename encryption, volume splitting, comment, delete-after.
  - Operations panel: Test Integrity, Repair ZIP, Convert Format, Benchmark, Merge Volumes.
  - **In-archive browser**: breadcrumb navigation inside archive, hierarchical file tree with Name/Size/Packed/Ratio/Modified/CRC/Method columns.
  - Right-click context menu: Extract to.../Extract here/Open/Rename/Delete/Replace with file/Copy path/Properties.
  - Add files to existing archive, delete entries, rename entries, replace entries — all via UI.
  - Extract controls: destination, keep structure, overwrite, Extract All/Extract Selection.
  - Progress bar during all archive operations.

### Version
- Bumped to **2.7.0**.



### Added — Comprehensive File Management Suite

#### 🔄 File Format Conversion Engine
- **`conversion_interface.hpp`**: Universal converter interface with `IConverter` abstract base, `ConversionEngine` orchestrator, and `ConversionResult` tracking.
- **`conversion_engine.cpp`**: Three fully-wired converter backends:
  - **FFmpegConverter**: Video (mp4/mkv/avi/webm/mov/gif/flv) ↔ all codecs (H.264/H.265/VP9/AV1), audio (mp3/flac/aac/ogg/opus/wav), with CRF, bitrate, preset, resolution, trim, and thread options.
  - **ImageMagickConverter**: 200+ image formats (png/jpg/webp/avif/heic/tiff/bmp/ico/psd/raw/jp2/jxl/exr) with quality, resize, colorspace, EXIF-strip, PDF page extraction.
  - **PandocConverter**: Documents (md/rst/html/docx/odt/epub/latex/pdf) with TOC, templates, CSS, numbered headings, PDF engine choice.
- **`gui/panels/BatchConvertPanel.qml`**: Full-featured batch conversion UI with source file list, output format picker, per-backend options panels (video/image/document), output directory picker, collision policy, thread slider, live progress, error log.
- **New submodules**: `pandoc`, `ghostpdl`, `poppler`, `LibRaw`, `libde265`, `calibre`, `wkhtmltopdf`, `Magick.NET`, `pcre2`, `re2`, `ripgrep`, `the_silver_searcher`, `sigil`, `md4c`, `cmark`, `highlightjs`, `rapidjson`.

#### 📝 Batch File Renaming Engine
- **`batch_rename_interface.hpp`**: Complete rename rule system with 13 rule types: Replace, Insert, Delete, Trim, Case (9 modes), Number (counter with padding), DateTime, EXIF/Metadata, Extension, Truncate, Sanitize, Hash, Transliterate, RegexGroups.
- **`BatchRenameEngine`**: Chain-based rule application, preview generation with conflict detection, execute with undo support, preset save/load.
- **`gui/panels/BatchRenamePanel.qml`**: Split-pane UI — left rule chain builder with per-rule configuration panels, right live preview table (original→new with conflict coloring). Built-in preset library.

#### 🔍 Search & Find-in-Files Engine
- **`search_interface.hpp`**: Comprehensive `SearchOptions` covering Everything+AgentRansack+grepWin+dnGrep feature sets: filename/content modes, PCRE2/wildcard/fuzzy match, size/date/attribute filters, `.gitignore` awareness, archive search, multi-threaded parallel search, find-and-replace in files with backup/undo.
- **`gui/panels/SearchPanel.qml`**: Three-mode tabbed UI (Filename Search / Content Search / Find & Replace), all filter options, results list with expandable content match lines, sort/group controls, export CSV.

#### 📦 Batch File Operations
- **`fileops_interface.hpp`**: Five operation engines:
  - **FileCopyMoveEngine**: TeraCopy-style copy/move with verify (xxHash/MD5/SHA-256), collision policy, throttle, retry, hardlink/symlink support.
  - **FolderSyncEngine**: FreeFileSync-style mirror/update/two-way/custom sync with versioning, sync database, real-time watch, delta detection.
  - **FileDiffEngine**: WinMerge-style file/folder diff (Myers/Patience/Histogram algorithms), side-by-side or unified output, HTML export.
  - **BackupEngine**: Areca/Duplicati-style full/incremental/differential backup with Zstd/LZ4/Brotli/7z compression, AES-256/ChaCha20 encryption, volume splitting, rsync delta.
  - **ArchiveEngine**: 7-Zip-style create/extract for zip/7z/tar.*/brotli/lz4/zstd with solid mode, SFX, volume split, password.
- **`gui/panels/FileOpsPanel.qml`**: Five-tab comprehensive panel (Copy/Move / Sync / Diff / Backup / Archive).
- **New submodules**: `libarchive`, `librsync`, `zstd`, `lz4`, `brotli`, `dtl-diff`, `p7zip`, `7zip`.

#### 🔬 Hex Editor
- **`hex_editor_interface.hpp`**: Full `HexBuffer` with mmap-backed sparse overlay (edit any file without full copy), undo/redo, insert/delete bytes, PCRE2/hex-pattern/ASCII/UTF-16 search with `??` wildcards.
- **`DataInterpretation`**: Interprets selection as int8/16/32/64 (LE+BE), uint variants, float32/64, ASCII, UTF-8, UTF-16, GUID, Windows FILETIME, Unix timestamp, CRC-32/CRC-16, binary, octal.
- **`gui/panels/HexEditorPanel.qml`**: Full hex view with offset column, hex bytes, ASCII panel, configurable columns (8/16/32), Data Inspector sidebar, search bar with mode selector, Go-to-offset, keyboard navigation.

#### 🖼 Image Viewer
- **`image_viewer_interface.hpp`**: XnViewMP-inspired — ThumbnailCache (LRU disk cache), ImageInfo with full EXIF/IPTC/XMP, ImageHistogram (per-channel), ColorAdjustments (brightness/contrast/saturation/hue/exposure/highlights/shadows/sharpness/temperature/vibrance/clarity), SlideshowOptions with transitions, lossless JPEG rotate/flip via exiftran.
- **`gui/panels/ImageViewerPanel.qml`**: Full XnViewMP-style viewer — filmstrip, zoom controls, rotate/flip toolbar, slideshow with interval, Histogram panel, Color Adjustments panel, EXIF Metadata panel, multi-page support (TIFF/GIF/PDF), keyboard navigation.

#### 📖 Markdown Viewer
- **`markdown_viewer_interface.hpp`**: `MarkdownRenderer` using md4c + cmark — CommonMark + GFM tables/task-lists/autolinks/strikethrough, math (KaTeX), Mermaid diagrams, highlight.js syntax highlighting, footnotes, wikilinks, emoji, YAML frontmatter, word count, reading time. Export: HTML, PDF, DOCX via Pandoc.
- **`gui/panels/MarkdownViewerPanel.qml`**: Split-pane editor+preview with WebEngineView, TOC sidebar, theme switcher (Light/Dark/Sepia), all extension toggles, font/width controls, export menu.

### Version
- Bumped to **2.6.0**.



### Added
- **Advanced Media Analysis Engine**:
    - **Video Perceptual Hashing (`vhash`)**: Implemented an FFmpeg-backed video analyzer that extracts evenly-spaced frames, converts them to grayscale thumbnails, and generates 64-bit dHash values to identify visually similar or duplicate video files regardless of resolution, codec, or frame rate.
    - **Audio Fingerprinting (`afingerprint`)**: Implemented an FFmpeg + Chromaprint integration to generate AcoustID-compatible audio fingerprints. Capable of identifying identical audio content across differing formats (mp3 vs flac), bitrates, or volume levels.
    - New CLI commands: `fo_cli vhash <file>`, `fo_cli afingerprint <file>`, `--list-vhash`, `--list-ahash`.
- **vcpkg Dependencies**: Added `ffmpeg` and `chromaprint` to `vcpkg.json` for media analysis support.
- **Documentation**: Substantial updates to the project's vision, architecture, and roadmap based on deep-dive codebase analysis.

## [2.4.3] - 2026-04-02

### Added
- **OmniUI Explorer Enhancements**:
    - Windows 11 style tabbed title bar.
    - Functional address bar and search field skeletons.
    - Detailed file list view with Name, Size, Type, and Date Modified.
    - Navigation sidebar with "This PC", "OneDrive", etc.
    - Status bar showing item and selection counts.
- **Documentation**:
    - `docs/COMPETITIVE_ANALYSIS_DETAILED.md`: Analysis of Files, File Pilot, and OpenSalamander.
    - `EXPLORER_PLAN.md`: Detailed implementation roadmap for the shell replacement, including a new "Integrated Image Management Suite" category and an "Advanced Dual-Pane Operations (Salamander Parity)" category.

### Improved
- mica-style visual effects and drop shadows for the explorer window.
- Updated `HANDOFF.md` for session transition.
