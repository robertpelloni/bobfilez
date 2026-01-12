# bobfilez

**An extremely robust, cross-platform file organization and deduplication engine with plugin architecture and empirical benchmarking.**

*Part of the bob software ecosystem by Robert Pelloni.*

bobfilez is built for power users who need:
- **Accurate duplicate detection** with multiple verification strategies
- **Flexible metadata extraction** (EXIF, filename parsing, OCR)
- **Perceptual image hashing** for near-duplicate photos
- **Blazing-fast hashing** with multiple algorithms (xxHash, SHA-256, BLAKE3)
- **CLI-first architecture** with optional Qt/Electron GUIs
- **Complete control** over scanning, hashing, and organization workflows

Unlike other tools that crash, misidentify duplicates, or sacrifice speed for features, bobfilez lets you **choose your trade-offs** via swappable providers and detailed benchmarking.

---

## Features

### Core Capabilities

✅ **Multiple directory scanners** (std::filesystem, dirent, Win32)—benchmarked for speed and portability  
✅ **Fast prefilter hashing** (XXH3, sampled chunks) + **strong verification** (SHA-256, BLAKE3)  
✅ **Exact duplicate detection** with size + hash + optional byte-compare  
✅ **Perceptual hashing** (dHash, pHash) for near-duplicate images  
✅ **EXIF/metadata extraction** (TinyEXIF, Exiv2, ExifTool)—read dates, GPS, camera info  
✅ **Filename date parsing** via regex (YYYY-MM-DD, YYYYMMDD, and many more patterns)  
✅ **OCR support** (Tesseract, PaddleOCR)—extract text from images and PDFs  
✅ **AI image classification** (planned)—tag and search photos by content  
✅ **SQLite database** with migrations—persistent metadata, incremental scans  
✅ **Alternate Data Streams** (Windows NTFS)—optional fast-hash caching without DB bloat  

### Unique Strengths

- **Plugin architecture**: Swap scanners, hashers, and metadata providers at runtime via CLI flags
- **Benchmarking harness**: Empirical comparison of all implementations on real datasets
- **Date fusion**: Combine EXIF, filename, and mtime dates; choose most reliable automatically
- **Duplicate verification modes**: Fast (hash-only), safe (byte-compare), paranoid (SHA-256)
- **Incremental scanning**: Only re-hash changed files; track scan history in DB
- **Cross-platform**: Windows, Linux, macOS; CMake + vcpkg for reproducible builds

---

## Use Cases

### You Need This If...

- 📂 You have giant messy folders with years of accumulated files  
- 🖼️ You have duplicate images in different resolutions/names/dates  
- 🗓️ You can't figure out the real date a photo was taken (EXIF vs. filename vs. mtime)  
- 🔍 Existing dedupe tools are too slow, crash, or misidentify duplicates  
- 🔒 You want 100% certainty that duplicates are exact matches (byte-by-byte verification)  
- 📋 You want to keep all filenames, dates, and metadata from duplicates "just in case"  
- 🚀 You need to rename/organize ridiculous amounts of files without GUI lag  
- 🔎 You want to OCR images/PDFs and search them like Google Photos—**locally, no cloud**  
- 🤖 You want AI image classification (objects, scenes) **locally, no cloud**  

---

## Quick Start

### CLI (Recommended)

```bash
# Build from source (requires CMake 3.16+, C++20 compiler)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Scan a directory and find duplicates
./build/cli/fo_cli --scanner=std --hasher=xxh3 --ext=.jpg,.png /path/to/photos

# Use strong hash for verification
./build/cli/fo_cli --hasher=blake3 /path/to/photos

# Extract metadata
./build/cli/fo_cli metadata --provider=tinyexif /path/to/photos
```

See [`README_CLI.md`](README_CLI.md) for detailed CLI usage.

### GUI (Legacy Qt App)

The original Qt GUI (`Openfilez`) is being refactored to use the CLI engine as a backend. For now, build with:

```bash
# Open in Visual Studio (Windows)
# or Qt Creator (cross-platform)
Openfilez.sln / Openfilez.pro
```

GUI decoupling roadmap: see [`docs/ROADMAP.md`](docs/ROADMAP.md).

---

## Documentation

- **[Library Evaluation](docs/LIBRARY_EVALUATION.md)**: Detailed comparison of all providers (speed, accuracy, license)
- **[Scanner Evaluation](docs/SCANNER_EVALUATION.md)**: Platform-specific scanner findings and benchmark protocol
- **[Database Schema](docs/DATABASE_SCHEMA.md)**: SQLite tables, migrations, and queries
- **[Benchmarking Plan](docs/BENCHMARKING_PLAN.md)**: Datasets, metrics, and CI integration
 - **[Benchmarking Runbook](docs/BENCHMARKING_RUNBOOK.md)**: How to build and run the scanner benchmark
- **[Roadmap](docs/ROADMAP.md)**: Phased development plan (20 weeks to v1.0)
- **[CLI Guide](README_CLI.md)**: Command-line usage and examples
 - **[AI Handoff Log](docs/AI_HANDOFF.md)**: Running status for future AI agents
 - **[Contributing (AI)](CONTRIBUTING_AI.md)**: Checklist for autonomous agents

---

## Architecture

filez uses a **plugin architecture** where:
- **Core engine** (`fo_core`) is a portable C++20 library with provider interfaces
- **Providers** (scanners, hashers, metadata, OCR) are swappable implementations
- **CLI** (`fo_cli`) is the primary user interface; GUIs are thin clients
- **Database** (SQLite3) persists metadata, hashes, and duplicate groups
- **Benchmarks** (`fo_benchmarks`) empirically validate provider performance

```
┌─────────────────────────────────────────────┐
│              GUI Frontends                  │
│   (Qt, Electron, GTK, Web, etc.)            │
└──────────────┬──────────────────────────────┘
               │ IPC (JSON-RPC, WebSocket)
┌──────────────▼──────────────────────────────┐
│              CLI Engine (fo_cli)            │
│  Commands: scan, duplicates, hash, metadata │
└──────────────┬──────────────────────────────┘
               │ Links to
┌──────────────▼──────────────────────────────┐
│           Core Library (fo_core)            │
│  ┌─────────────────────────────────────┐   │
│  │ Interfaces: IFileScanner, IHasher,  │   │
│  │ IMetadataProvider, IOCRProvider     │   │
│  └─────────────────────────────────────┘   │
│  ┌─────────────────────────────────────┐   │
│  │ Providers: StdFsScanner, XXHasher,  │   │
│  │ TinyExifMetadata, TesseractOCR, ... │   │
│  └─────────────────────────────────────┘   │
│  ┌─────────────────────────────────────┐   │
│  │ Engine: scan(), find_duplicates(),  │   │
│  │ extract_metadata(), ocr(), ...      │   │
│  └─────────────────────────────────────┘   │
└──────────────┬──────────────────────────────┘
               │ Persists to
┌──────────────▼──────────────────────────────┐
│           Database (SQLite3)                │
│  Tables: files, file_dates, duplicate_groups│
└─────────────────────────────────────────────┘
```

---

## Building from Source

### Requirements

- **CMake** 3.16+
- **C++20 compiler** (MSVC 2019+, GCC 10+, Clang 12+)
- **vcpkg** (optional, for Exiv2/BLAKE3/Tesseract)

### Linux / macOS

```bash
# Install dependencies (example: Ubuntu)
sudo apt install build-essential cmake libsqlite3-dev

# Optional: vcpkg for advanced providers
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.sh && ./vcpkg integrate install

# Build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Run
./build/cli/fo_cli --help
```

### Windows

```powershell
# Install vcpkg (if not already)
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg; .\bootstrap-vcpkg.bat; .\vcpkg integrate install

# Build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

# Run
.\build\cli\Release\fo_cli.exe --help
```

---

## Provider Selection

Choose implementations via CLI flags:

```bash
# Fast prefilter (default)
fo_cli --hasher=xxh3 /path

# Strong cryptographic hash
fo_cli --hasher=blake3 /path

# Full EXIF/IPTC/XMP metadata
fo_cli metadata --provider=exiv2 /path

# OCR with Tesseract
fo_cli ocr --provider=tesseract --lang=eng /path
```

See [`docs/LIBRARY_EVALUATION.md`](docs/LIBRARY_EVALUATION.md) for benchmarks and recommendations.

---

## FAQ

**Q: Why not just use rmlint / jdupes / dupeGuru?**  
A: They're excellent but have trade-offs. filez offers:
- More verification modes (hash-only, byte-compare, crypto)
- Metadata fusion (combine EXIF/filename/mtime dates intelligently)
- Plugin architecture (benchmark and choose your own providers)
- OCR and AI classification for advanced workflows

**Q: Is this faster than [tool X]?**  
A: Depends on your workload. We benchmark everything—see [`docs/BENCHMARKING_PLAN.md`](docs/BENCHMARKING_PLAN.md). For large files, xxHash + sampled chunks is fastest. For strong verification, BLAKE3 beats SHA-256 by 3–4×.

**Q: What about radare2 in the libs folder?**  
A: It's a binary analysis framework (reverse engineering). Not needed for file organization—safe to remove unless you plan executable signature detection. See [`docs/LIBRARY_EVALUATION.md`](docs/LIBRARY_EVALUATION.md#8-radare2-reverse-engineering-framework).

**Q: Windows-only or cross-platform?**  
A: **Cross-platform from the start.** Core engine uses portable C++20 and std::filesystem. Windows-specific features (Alternate Data Streams, Win32 FindFirstFile) are optional optimizations.

**Q: License?**  
A: GPL-compatible (exact license TBD; Exiv2/Tesseract are GPLv2/Apache 2.0). Permissive for core; optional providers may have stricter terms.

---

## Contributing

See [`docs/ROADMAP.md`](docs/ROADMAP.md) for current priorities. Contributions welcome:
- New providers (hashers, metadata, OCR)
- Benchmark datasets (labeled duplicates, ground truth)
- GUI frontends (Electron, GTK, etc.)
- Documentation and tutorials

---

## License

TBD (likely GPLv2+ to match Exiv2). Core library may be dual-licensed (GPL + commercial) in the future.

---

## Acknowledgments

- **TinyEXIF** (Seacave): Fast, lightweight EXIF reader
- **hash-library** (Stephan Brumme): SHA-256, MD5, CRC32, Keccak
- **xxHash** (Yann Collet): Blazing-fast non-cryptographic hashing
- Inspired by: rmlint, jdupes, dupeGuru, fclones, Czkawka

---

## Contact

**Author**: Robert Pelloni  
**Email**: (TBD)  
**Websites**: [robertpelloni.com](http://robertpelloni.com), [bobsgame.com](http://bobsgame.com), [fwber.me](http://fwber.me)

---

**Status**: 🚧 **Active Development** — CLI prototype ready; GUI refactor and benchmarking in progress.








