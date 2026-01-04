# filez CLI Engine (prototype)

This is a cross-platform, console-first engine scaffold extracted from the monolithic Qt app. It scans directories, computes a fast prefilter hash, and groups potential duplicates by size + hash. It is designed for a plugin architecture so multiple providers (scanner, hasher, metadata, OCR, DB) can be swapped and benchmarked.

## Build (CMake)

Requirements: CMake 3.16+, a C++20 compiler.

```bash
# Configure and build (out-of-source)
cmake -S . -B build
cmake --build build --config Release

# Run
./build/cli/fo_cli --help
```

On Windows (PowerShell):

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
.\build\cli\Release\fo_cli.exe --help
```

## Usage

```
fo_cli <command> [options] [paths...]
```

### Commands

- `scan`: Scan directories and list files.
- `duplicates`: Find and list duplicate files (size + hash).
- `hash`: Compute and print file hashes.
- `metadata`: Extract and print metadata (EXIF, GPS).
- `ocr`: Extract text from images (requires Tesseract).
- `similar`: Find visually similar images (requires OpenCV).
- `classify`: Classify images using AI (requires ONNX Runtime).
- `organize`: Move files based on rules (e.g., date, tags).
- `rename`: Rename files based on patterns.
- `delete-duplicates`: Delete duplicate files based on strategy.
- `export`: Export scan results to JSON, CSV, or HTML format.
- `undo`: Undo the last file operation (move, rename, copy).
- `history`: Show operation history.

### Options

- `--scanner=<name>`: Select scanner (std, win32, dirent).
- `--hasher=<name>`: Select hasher (fast64, blake3).
- `--db=<path>`: Path to SQLite database (default: `fo.db`).
- `--ext=<.jpg,.png>`: Filter by extensions.
- `--follow-symlinks`: Follow directory symlinks.
- `--rule=<template>`: Organization rule (e.g., `/Photos/{year}/{month}`).
- `--rules=<file.yaml>`: Load organization rules from YAML file.
- `--pattern=<tmpl>`: Rename pattern (e.g., `{year}_{name}.{ext}`).
- `--keep=<strategy>`: Keep strategy for duplicates (`oldest`, `newest`, `shortest`, `longest`).
- `--dry-run`: Simulate operations without modifying files.
- `--incremental`: Perform an incremental scan (detect new/modified files).
- `--prune`: Remove deleted files from the database during scan.
- `--format=<fmt>`: Export format (`json`, `csv`, `html`).
- `--output=<path>`: Output file path for export command.
- `--thumbnails`: Include thumbnails in HTML export (images only).
- `--phash=<algo>`: Perceptual hash algorithm (`dhash`, `phash`, `ahash`).
- `--threshold=<N>`: Similarity threshold for `similar` command (default: 10).
- `--list-scanners`: List available scanners.
- `--list-hashers`: List available hashers.
- `--list-metadata`: List available metadata providers.
- `--list-ocr`: List available OCR providers.
- `--list-classifiers`: List available classifiers.
- `--list-phash`: List available perceptual hash algorithms.

### Examples

```bash
# Scan and find duplicates
fo_cli duplicates --db=my.db /path/to/photos

# Organize photos by date
fo_cli organize --rule="/Sorted/{year}/{month}" --dry-run /path/to/photos

# Rename files to include date
fo_cli rename --pattern="{year}-{month}-{day}_{name}.{ext}" /path/to/photos

# Delete duplicates, keeping the oldest file
fo_cli delete-duplicates --keep=oldest --dry-run

# Export scan results to HTML report
fo_cli export --format=html --output=report.html /path/to/photos

# Export with embedded thumbnails for images
fo_cli export --format=html --thumbnails --output=gallery.html /path/to/photos

# Export duplicates to JSON for processing
fo_cli export --format=json --output=results.json /path/to/photos

# Find similar images using pHash (DCT-based)
fo_cli similar --phash=phash --threshold=5 /path/to/query.jpg

# View operation history
fo_cli history

# Undo the last operation
fo_cli undo
```

## Notes

- This prototype intentionally avoids heavy external dependencies. Strong hashes (SHA-256, BLAKE3), EXIF readers (TinyEXIF/Exiv2), and OCR providers will be integrated as separate providers next.
- The plugin registry supports multiple implementations per feature; selection via CLI flags will be extended.

See `docs/SCANNER_EVALUATION.md` for analysis of scanner trade-offs on Windows and Linux.