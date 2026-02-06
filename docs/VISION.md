# filez Vision Document

**Author:** Robert Pelloni  
**Version:** 2.1.0  
**Last Updated:** 2026-01-09

---

## Executive Summary

**filez** is an extremely robust, cross-platform file organization and deduplication engine designed for power users who demand accuracy, speed, and complete control over their digital assets.

The core philosophy: **You choose your trade-offs.** Unlike monolithic tools that make assumptions, filez exposes every decision point—hashing algorithms, verification modes, metadata sources, organization rules—through a plugin architecture with empirical benchmarking.

---

## The Problem

### The Digital Mess Reality

Modern users accumulate massive, chaotic file collections:
- **Duplicate explosion**: Same photo in 5 folders, 3 resolutions, 2 names
- **Metadata chaos**: EXIF says 2019, filename says 2021, mtime says yesterday
- **Tool failures**: Existing dedupe tools crash on large datasets, misidentify duplicates, or sacrifice speed for features
- **Cloud lock-in**: Google Photos/iCloud do magic—but your data is hostage
- **No verification**: "Trust me, it's a duplicate" isn't good enough for irreplaceable files

### Why Existing Tools Fail

| Tool | Problem |
|------|---------|
| **rmlint** | Fast but Unix-only, no metadata fusion |
| **jdupes** | Good but no perceptual hashing, no OCR |
| **dupeGuru** | GUI-only, slow on large datasets |
| **fclones** | Rust rewrite of rmlint, same limitations |
| **Czkawka** | Modern but limited provider choices |
| **Cloud services** | Privacy concerns, vendor lock-in, no local control |

---

## The Solution

### Core Design Principles

1. **CLI-First Architecture**
   - The CLI (`fo_cli`) is the primary interface—scriptable, composable, fast
   - GUIs (Qt, Electron, Web) are thin clients that call the CLI or link the core library
   - No GUI-specific logic in the engine

2. **Plugin Architecture**
   - Every component is swappable: scanners, hashers, metadata providers, OCR engines
   - Runtime selection via CLI flags (`--hasher=blake3`, `--scanner=win32`)
   - New providers can be added without modifying core code

3. **Empirical Benchmarking**
   - Don't guess—measure. Every provider is benchmarked on real datasets
   - Users can run benchmarks on their own data to find optimal settings
   - Published results guide default choices

4. **Extreme Robustness**
   - Multiple verification modes: hash-only (fast), byte-compare (safe), crypto (paranoid)
   - Never delete without explicit confirmation
   - Full undo/history for all destructive operations
   - Graceful handling of permission errors, symlinks, special files

5. **Cross-Platform from Day One**
   - C++20 with `std::filesystem` for portability
   - Platform-specific optimizations are optional enhancements (Win32 scanner, NTFS ADS)
   - Same CLI commands work everywhere

---

## Feature Taxonomy

### Tier 1: Core Capabilities (Implemented)

| Feature | Description | Providers |
|---------|-------------|-----------|
| **Directory Scanning** | Enumerate files with filtering (extensions, size, date) | `std::filesystem`, `Win32`, `dirent` |
| **Fast Hashing** | Prefilter with non-crypto hashes | `xxHash` (XXH3, XXH64), `Fast64` |
| **Strong Hashing** | Verification with crypto hashes | `SHA-256`, `BLAKE3`, `MD5` |
| **Exact Duplicate Detection** | Size + hash + optional byte-compare | Built-in engine |
| **Metadata Extraction** | EXIF, IPTC, XMP from images | `TinyEXIF`, `Exiv2`, `ExifTool` |
| **Filename Date Parsing** | Extract dates from filenames via regex | Built-in patterns |
| **Date Fusion** | Combine EXIF/filename/mtime intelligently | Configurable priority |
| **Database Persistence** | SQLite with migrations, incremental scans | `SQLite3` |
| **Incremental Scanning** | Only re-hash changed files | mtime/size tracking |

### Tier 2: Advanced Capabilities (Implemented)

| Feature | Description | Providers |
|---------|-------------|-----------|
| **Perceptual Hashing** | Find visually similar images | `dHash`, `pHash`, `aHash` |
| **OCR** | Extract text from images/PDFs | `Tesseract`, `PaddleOCR` |
| **AI Classification** | Tag images by content (objects, scenes) | `ONNX Runtime` (local models) |
| **Rule-Based Organization** | Move/rename files based on metadata rules | `RuleEngine` |
| **Batch Operations** | Delete, rename, move with undo | Transaction log |
| **Export** | JSON, CSV, HTML reports | Built-in formatters |
| **Alternate Data Streams** | Windows NTFS hash caching | Win32 API |

### Tier 3: Future Capabilities (Planned)

| Feature | Description | Status |
|---------|-------------|--------|
| **Video Deduplication** | Perceptual hashing for video content | Research |
| **Audio Fingerprinting** | Identify duplicate audio files | Research |
| **Cloud Integration** | Optional S3/GCS/Azure sync | Post-v1.0 |
| **Network Scanning** | SMB/NFS share support | Post-v1.0 |
| **Web Interface** | Browser-based GUI | Post-v1.0 |
| **Mobile Companion** | iOS/Android for photo import | Post-v1.0 |

---

## Architecture

### System Layers

```
┌─────────────────────────────────────────────────────────────────┐
│                        USER INTERFACES                          │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐            │
│  │   CLI   │  │   Qt6   │  │Electron │  │   Web   │            │
│  │(fo_cli) │  │(fo_gui) │  │ (future)│  │ (future)│            │
│  └────┬────┘  └────┬────┘  └────┬────┘  └────┬────┘            │
│       │            │            │            │                  │
│       └────────────┴─────┬──────┴────────────┘                  │
│                          │                                      │
│                    IPC / Direct Link                            │
└──────────────────────────┼──────────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────────┐
│                      CORE ENGINE (fo_core)                      │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │                     Provider Registry                      │ │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐      │ │
│  │  │ Scanners │ │ Hashers  │ │ Metadata │ │   OCR    │      │ │
│  │  └──────────┘ └──────────┘ └──────────┘ └──────────┘      │ │
│  └────────────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │                        Engine                              │ │
│  │  scan() | find_duplicates() | extract_metadata() | ocr()  │ │
│  │  classify() | organize() | delete() | rename() | undo()   │ │
│  └────────────────────────────────────────────────────────────┘ │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │                    Data Access Layer                       │ │
│  │  FileRepository | DuplicateRepository | HistoryRepository  │ │
│  └────────────────────────────────────────────────────────────┘ │
└──────────────────────────┼──────────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────────┐
│                      PERSISTENCE LAYER                          │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐ │
│  │     SQLite      │  │   ADS Cache     │  │   File System   │ │
│  │   (metadata)    │  │ (Windows NTFS)  │  │   (operations)  │ │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

### Provider Interface Pattern

Every provider type follows the same pattern:

```cpp
// Interface definition
class IHasher {
public:
    virtual ~IHasher() = default;
    virtual std::string name() const = 0;
    virtual std::string hash_file(const fs::path& path) = 0;
    virtual std::string hash_bytes(std::span<const uint8_t> data) = 0;
};

// Registration (static initialization)
static auto reg = []() {
    Registry<IHasher>::instance().add("blake3", []() {
        return std::make_unique<Blake3Hasher>();
    });
    return true;
}();

// Runtime selection
auto hasher = Registry<IHasher>::instance().create("blake3");
```

### Database Schema (Core Tables)

| Table | Purpose |
|-------|---------|
| `files` | File metadata (path, size, mtime, scan_id) |
| `file_hashes` | Hash values by algorithm (file_id, algorithm, hash) |
| `file_dates` | Extracted dates by source (file_id, source, date) |
| `duplicate_groups` | Groups of identical files |
| `duplicate_members` | Files in each group |
| `operations` | History log for undo |
| `rules` | Organization rules |
| `scans` | Scan session metadata |

---

## CLI Command Reference

### Scanning & Discovery

```bash
fo_cli scan [OPTIONS] <PATHS>...
    --scanner=<std|win32|dirent>    # Scanner implementation
    --ext=<.jpg,.png,...>           # Filter by extension
    --min-size=<bytes>              # Minimum file size
    --max-size=<bytes>              # Maximum file size
    --exclude=<pattern>             # Exclude patterns
    --recursive / --no-recursive    # Subdirectory handling
```

### Duplicate Detection

```bash
fo_cli duplicates [OPTIONS] <PATHS>...
    --hasher=<xxh3|sha256|blake3>   # Hash algorithm
    --mode=<fast|safe|paranoid>     # Verification mode
    --min-size=<bytes>              # Ignore small files
    --output=<json|csv|table>       # Output format
```

### Metadata & Analysis

```bash
fo_cli metadata [OPTIONS] <PATHS>...
    --provider=<tinyexif|exiv2|exiftool>
    --fields=<date,gps,camera,...>

fo_cli ocr [OPTIONS] <PATHS>...
    --provider=<tesseract|paddleocr>
    --lang=<eng|jpn|...>

fo_cli classify [OPTIONS] <PATHS>...
    --model=<resnet|mobilenet|custom>
    --threshold=<0.0-1.0>

fo_cli similar [OPTIONS] <PATHS>...
    --algorithm=<dhash|phash|ahash>
    --threshold=<0-64>              # Hamming distance
```

### Organization & Operations

```bash
fo_cli organize [OPTIONS] <PATHS>...
    --rules=<rules.json>            # Rule file
    --dry-run                       # Preview only
    --confirm                       # Interactive confirmation

fo_cli rename [OPTIONS] <PATHS>...
    --pattern=<template>            # e.g., "{date}_{original}"
    --dry-run

fo_cli delete-duplicates [OPTIONS]
    --keep=<first|newest|oldest|largest>
    --dry-run
    --confirm

fo_cli undo                         # Undo last operation
fo_cli history                      # View operation log
```

### Export & Reporting

```bash
fo_cli export [OPTIONS]
    --format=<json|csv|html>
    --output=<file>
    --include=<duplicates|metadata|all>
```

---

## Verification Modes

| Mode | Speed | Safety | Use Case |
|------|-------|--------|----------|
| **Fast** | ★★★★★ | ★★☆☆☆ | Quick scan, size + fast hash only |
| **Safe** | ★★★☆☆ | ★★★★☆ | Default: size + fast hash + strong hash |
| **Paranoid** | ★☆☆☆☆ | ★★★★★ | Byte-by-byte comparison after hash match |

---

## Date Fusion Strategy

filez combines multiple date sources to determine the "true" date of a file:

| Source | Priority | Reliability |
|--------|----------|-------------|
| EXIF DateTimeOriginal | 1 (highest) | Camera-set, most reliable |
| EXIF CreateDate | 2 | Usually same as above |
| Filename pattern | 3 | User-controlled, often accurate |
| EXIF ModifyDate | 4 | Can be changed by editing |
| File mtime | 5 (lowest) | Easily corrupted by copies |

Configurable via `--date-priority` flag.

---

## Performance Targets

| Operation | Target | Notes |
|-----------|--------|-------|
| Scan 100K files | < 10 seconds | std::filesystem on SSD |
| Hash 1GB file (xxHash) | < 1 second | ~1.5 GB/s |
| Hash 1GB file (BLAKE3) | < 2 seconds | ~700 MB/s |
| Find duplicates in 100K files | < 30 seconds | With size prefilter |
| Extract EXIF (TinyEXIF) | > 10K files/sec | Header-only parsing |
| OCR single image | < 2 seconds | Tesseract, English |

---

## Security Considerations

1. **No Network by Default**: filez never phones home, uploads data, or requires internet
2. **Local Processing**: All AI/OCR runs locally—no cloud APIs
3. **Permission Preservation**: Operations preserve original permissions where possible
4. **Safe Defaults**: Destructive operations require explicit `--confirm` or `--force`
5. **Audit Trail**: All operations logged in database for forensic review

---

## Target Users

### Primary Personas

1. **Digital Hoarder**
   - 500GB+ of accumulated photos, downloads, documents
   - Wants to reclaim disk space without losing anything important
   - Needs confidence that "duplicates" are truly identical

2. **Photographer**
   - Thousands of RAW + JPEG pairs
   - Needs perceptual hashing for near-duplicates
   - Wants date-based organization with EXIF priority

3. **System Administrator**
   - Managing shared drives with years of accumulated cruft
   - Needs scriptable CLI for automation
   - Requires detailed reports for stakeholders

4. **Data Archivist**
   - Long-term preservation concerns
   - Wants cryptographic verification (BLAKE3/SHA-256)
   - Needs export to standard formats

---

## Non-Goals (Explicit Exclusions)

- **Real-time sync**: filez is batch-oriented, not a sync daemon
- **Cloud-first**: Cloud integration is optional, never required
- **Mobile-first**: Desktop CLI is primary; mobile is companion at best
- **Proprietary formats**: All data stored in open formats (SQLite, JSON)
- **Subscription model**: One-time tool, no ongoing fees

---

## Success Metrics

| Metric | Target |
|--------|--------|
| False positive rate (duplicates) | < 0.001% with Safe mode |
| False negative rate (duplicates) | < 0.01% |
| Crash rate on large datasets | 0% (graceful degradation) |
| CLI command coverage | 100% of engine functionality |
| Cross-platform parity | All features on Windows/Linux/macOS |
| Test coverage | > 80% of core engine |

---

## Roadmap Summary

| Phase | Status | Description |
|-------|--------|-------------|
| 1. Core Architecture | ✅ Complete | CLI, plugin registry, basic providers |
| 2. Database Layer | ✅ Complete | SQLite, migrations, repositories |
| 3. Provider Expansion | ✅ Complete | BLAKE3, Exiv2, Tesseract, perceptual hash |
| 4. Benchmarking | ✅ Complete | Google Benchmark integration |
| 5. GUI Decoupling | ✅ Complete | Qt6 frontend |
| 6. Advanced Features | ✅ Complete | AI classification, rule engine |
| 7. Polish & Release | ✅ Complete | Installers, fuzzing, v1.0 launch |
| 8. Distribution & Maintenance | 🔄 In Progress | MSI/AppImage installers, Chocolatey/Winget packages |

---

## Appendix: Technology Stack

| Component | Technology | Rationale |
|-----------|------------|-----------|
| Language | C++20 | Performance, `std::filesystem`, wide library support |
| Build | CMake + Ninja | Cross-platform, fast incremental builds |
| Dependencies | vcpkg | Reproducible builds, Windows support |
| Database | SQLite3 | Single-file, zero-config, battle-tested |
| Testing | Google Test | Industry standard, good CMake integration |
| Benchmarking | Google Benchmark | Microbenchmark standard |
| GUI | Qt6 | Cross-platform, mature, good C++ integration |
| Hashing | xxHash, BLAKE3 | Best-in-class performance |
| Metadata | TinyEXIF, Exiv2 | Lightweight vs. comprehensive options |
| OCR | Tesseract | Open source standard |
| AI | ONNX Runtime | Local inference, model portability |

---

## Contact

**Author**: Robert Pelloni  
**Websites**: [robertpelloni.com](http://robertpelloni.com) | [bobsgame.com](http://bobsgame.com) | [fwber.me](http://fwber.me)

---

*This document represents the complete vision for filez. All implementation decisions should align with these principles and goals.*
