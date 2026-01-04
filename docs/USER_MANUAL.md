# filez CLI User Manual

**Version**: 1.0  
**Last Updated**: December 2024

filez is a cross-platform file organization and deduplication engine with a plugin architecture. This manual covers the CLI tool (`fo_cli`), the primary user interface.

---

## Table of Contents

1. [Installation](#installation)
2. [Quick Start](#quick-start)
3. [Command Reference](#command-reference)
4. [Global Options](#global-options)
5. [Configuration](#configuration)
6. [Workflows](#workflows)
7. [Troubleshooting](#troubleshooting)

---

## Installation

### Requirements

- **CMake** 3.16+
- **C++20 compiler**: MSVC 2019+, GCC 10+, or Clang 12+
- **vcpkg** (optional): For advanced providers (Exiv2, BLAKE3, Tesseract)

### Windows

```powershell
# Clone the repository
git clone https://github.com/robertpelloni/filez.git
cd filez

# Quick build (recommended)
.\build.bat

# Or manual build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release

# Verify installation
.\build\cli\fo_cli.exe --version
```

### Linux / macOS

```bash
# Install dependencies (Ubuntu example)
sudo apt install build-essential cmake libsqlite3-dev

# Clone and build
git clone https://github.com/robertpelloni/filez.git
cd filez
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Verify installation
./build/cli/fo_cli --version
```

### Optional Dependencies

Install via vcpkg for additional features:

```bash
# Tesseract OCR
vcpkg install tesseract

# BLAKE3 hasher
vcpkg install blake3

# Exiv2 metadata
vcpkg install exiv2
```

---

## Quick Start

### 1. Scan a Directory

```bash
fo_cli scan /path/to/photos
```

### 2. Find Duplicates

```bash
fo_cli duplicates /path/to/photos
```

### 3. Preview Organization

```bash
fo_cli organize --rule="{year}/{month}/" --dry-run /path/to/photos
```

### 4. Execute Organization

```bash
fo_cli organize --rule="{year}/{month}/" /path/to/photos
```

### 5. Undo if Needed

```bash
fo_cli undo
```

---

## Command Reference

### scan

Scan directories and catalog files to the database.

```bash
fo_cli scan [options] <path>...
```

**Options:**

| Option | Description |
|--------|-------------|
| `--scanner=std\|win32\|dirent` | Scanner implementation (default: `std`) |
| `--ext=.jpg,.png` | Filter by file extensions |
| `--incremental` | Only scan new/modified files |
| `--prune` | Remove deleted files from database |
| `--follow-symlinks` | Follow symbolic links |

**Examples:**

```bash
# Scan photos only
fo_cli scan --ext=.jpg,.png,.heic /path/to/photos

# Incremental scan (fast re-scan)
fo_cli scan --incremental /path/to/photos

# Clean up deleted files from database
fo_cli scan --incremental --prune /path/to/photos

# Use Win32 API for faster scanning on Windows
fo_cli scan --scanner=win32 /path/to/photos
```

---

### duplicates

Find duplicate files based on size and hash.

```bash
fo_cli duplicates [options] <path>...
```

**Options:**

| Option | Description |
|--------|-------------|
| `--hasher=xxh3\|sha256\|blake3` | Hash algorithm (default: `xxh3`) |

**Examples:**

```bash
# Fast duplicate detection
fo_cli duplicates --hasher=xxh3 /path/to/photos

# Cryptographic verification (slower, more secure)
fo_cli duplicates --hasher=sha256 /path/to/photos

# BLAKE3 (fast + cryptographic)
fo_cli duplicates --hasher=blake3 /path/to/photos
```

**Hasher Comparison:**

| Algorithm | Speed | Security | Use Case |
|-----------|-------|----------|----------|
| `xxh3` | Fastest | Non-crypto | Quick deduplication |
| `sha256` | Slow | Cryptographic | High-assurance verification |
| `blake3` | Fast | Cryptographic | Best of both worlds |

---

### hash

Compute and display file hashes.

```bash
fo_cli hash [options] <file>...
```

**Examples:**

```bash
# Hash a single file
fo_cli hash photo.jpg

# Hash multiple files with BLAKE3
fo_cli hash --hasher=blake3 *.jpg
```

---

### metadata

Extract EXIF and other metadata from files.

```bash
fo_cli metadata [options] <path>...
```

**Options:**

| Option | Description |
|--------|-------------|
| `--provider=tinyexif\|exiv2` | Metadata provider (default: `tinyexif`) |

**Examples:**

```bash
# Extract EXIF from images
fo_cli metadata /path/to/photos

# Use Exiv2 for full IPTC/XMP support
fo_cli metadata --provider=exiv2 /path/to/photos

# JSON output for scripting
fo_cli metadata --format=json /path/to/photos
```

**Provider Comparison:**

| Provider | Speed | Features | Dependencies |
|----------|-------|----------|--------------|
| `tinyexif` | Fast | Basic EXIF | None |
| `exiv2` | Medium | Full EXIF/IPTC/XMP | Exiv2 library |

---

### ocr

Extract text from images using OCR.

```bash
fo_cli ocr [options] <path>...
```

**Options:**

| Option | Description |
|--------|-------------|
| `--provider=tesseract` | OCR engine |
| `--lang=eng` | Language code (default: `eng`) |

**Examples:**

```bash
# Extract text from scanned documents
fo_cli ocr /path/to/scans

# French language OCR
fo_cli ocr --lang=fra /path/to/documents

# Multiple languages
fo_cli ocr --lang=eng+fra+deu /path/to/documents
```

**Common Language Codes:**

| Code | Language |
|------|----------|
| `eng` | English |
| `fra` | French |
| `deu` | German |
| `spa` | Spanish |
| `jpn` | Japanese |
| `chi_sim` | Chinese (Simplified) |

---

### similar

Find visually similar images using perceptual hashing.

```bash
fo_cli similar [options] <image>
```

**Options:**

| Option | Description |
|--------|-------------|
| `--phash=dhash\|phash\|ahash` | Perceptual hash algorithm |
| `--threshold=N` | Similarity threshold (default: 10) |

**Examples:**

```bash
# Find images similar to a reference
fo_cli similar --phash=phash query.jpg

# Strict matching (lower threshold = more similar)
fo_cli similar --phash=dhash --threshold=5 query.jpg

# Loose matching (higher threshold = more tolerance)
fo_cli similar --phash=ahash --threshold=15 query.jpg
```

**Algorithm Comparison:**

| Algorithm | Description | Best For |
|-----------|-------------|----------|
| `dhash` | Difference hash | Fast, gradient-based |
| `phash` | Perceptual hash (DCT) | Most accurate |
| `ahash` | Average hash | Simple, fast |

---

### classify

Classify images using AI models.

```bash
fo_cli classify [options] <path>...
```

**Options:**

| Option | Description |
|--------|-------------|
| `--model=path` | Path to ONNX model file |

**Examples:**

```bash
# Classify images using a custom model
fo_cli classify --model=models/imagenet.onnx /path/to/photos
```

---

### organize

Move files based on rules (date, metadata, patterns).

```bash
fo_cli organize [options] <path>...
```

**Options:**

| Option | Description |
|--------|-------------|
| `--rule="{year}/{month}/"` | Organization template |
| `--rules=rules.yaml` | Load rules from YAML file |
| `--dry-run` | Preview changes without moving files |

**Template Variables:**

| Variable | Description | Example |
|----------|-------------|---------|
| `{year}` | 4-digit year | `2024` |
| `{month}` | 2-digit month | `12` |
| `{day}` | 2-digit day | `28` |
| `{name}` | Original filename (no extension) | `IMG_1234` |
| `{ext}` | File extension | `jpg` |
| `{camera}` | Camera make/model | `Canon EOS R5` |

**Examples:**

```bash
# Organize by year/month
fo_cli organize --rule="{year}/{month}/" /path/to/photos

# Preview first (recommended)
fo_cli organize --rule="{year}/{month}/" --dry-run /path/to/photos

# Organize by year/month/day
fo_cli organize --rule="{year}/{month}/{day}/" /path/to/photos

# Use a rules file for complex logic
fo_cli organize --rules=my-rules.yaml /path/to/photos
```

**Rules YAML Example:**

```yaml
# my-rules.yaml
rules:
  - match: "*.jpg"
    destination: "Photos/{year}/{month}/"
  - match: "*.mp4"
    destination: "Videos/{year}/"
  - match: "*.pdf"
    destination: "Documents/"
```

---

### delete-duplicates

Remove duplicate files based on a selection strategy.

```bash
fo_cli delete-duplicates [options]
```

**Options:**

| Option | Description |
|--------|-------------|
| `--keep=oldest\|newest\|shortest\|longest` | Which duplicate to keep |
| `--auto` | Delete without confirmation |
| `--dry-run` | Preview deletions |

**Examples:**

```bash
# Preview which files would be deleted (keep oldest)
fo_cli delete-duplicates --keep=oldest --dry-run

# Keep the file with the shortest path
fo_cli delete-duplicates --keep=shortest --dry-run

# Delete automatically (no prompts)
fo_cli delete-duplicates --keep=oldest --auto
```

**Strategy Comparison:**

| Strategy | Keeps | Deletes |
|----------|-------|---------|
| `oldest` | Oldest modification time | Newer copies |
| `newest` | Newest modification time | Older copies |
| `shortest` | Shortest file path | Longer paths |
| `longest` | Longest file path | Shorter paths |

---

### rename

Rename files based on patterns.

```bash
fo_cli rename [options] <path>...
```

**Options:**

| Option | Description |
|--------|-------------|
| `--pattern="{year}-{month}-{day}_{name}.{ext}"` | Rename template |
| `--dry-run` | Preview renames |

**Examples:**

```bash
# Rename with date prefix
fo_cli rename --pattern="{year}-{month}-{day}_{name}.{ext}" /path/to/photos

# Preview first
fo_cli rename --pattern="{year}_{name}.{ext}" --dry-run /path/to/photos

# Sequential numbering (if supported)
fo_cli rename --pattern="photo_{seq:04d}.{ext}" /path/to/photos
```

---

### export

Export scan results to various formats.

```bash
fo_cli export [options] <path>...
```

**Options:**

| Option | Description |
|--------|-------------|
| `--format=json\|csv\|html` | Output format |
| `--output=path` | Output file path |
| `--thumbnails` | Include thumbnails (HTML only) |

**Examples:**

```bash
# Export to JSON
fo_cli export --format=json --output=results.json /path/to/photos

# Export to CSV
fo_cli export --format=csv --output=results.csv /path/to/photos

# Export to HTML with thumbnails
fo_cli export --format=html --output=gallery.html --thumbnails /path/to/photos
```

---

### undo

Undo the last file operation.

```bash
fo_cli undo
```

Supported operations: move, rename, copy, delete.

---

### history

View the operation log.

```bash
fo_cli history
```

Shows recent file operations with timestamps and details.

---

## Global Options

These options apply to all commands:

| Option | Description |
|--------|-------------|
| `--db=path` | Database path (default: `fo.db`) |
| `--format=json` | Output in JSON format (for scripting) |
| `--version` | Show version information |
| `--help` | Show help message |

**Discovery Options:**

| Option | Description |
|--------|-------------|
| `--list-scanners` | List available scanner implementations |
| `--list-hashers` | List available hasher implementations |
| `--list-metadata` | List available metadata providers |
| `--list-ocr` | List available OCR providers |
| `--list-classifiers` | List available AI classifiers |
| `--list-phash` | List available perceptual hash algorithms |

---

## Configuration

### Database

filez stores all metadata in a SQLite database:

```bash
# Default location
fo.db

# Custom location
fo_cli scan --db=/path/to/custom.db /path/to/photos
```

The database contains:
- File catalog (path, size, modification time)
- File hashes (xxHash, SHA-256, BLAKE3)
- Duplicate groups
- EXIF metadata
- Operation history

### Environment Variables

| Variable | Description |
|----------|-------------|
| `FO_DB_PATH` | Default database path |
| `FO_SCANNER` | Default scanner implementation |
| `FO_HASHER` | Default hasher implementation |
| `TESSDATA_PREFIX` | Tesseract data directory (for OCR) |

---

## Workflows

### Photo Library Organization

```bash
# 1. Initial scan
fo_cli scan --ext=.jpg,.jpeg,.png,.heic,.raw /path/to/photos

# 2. Find duplicates
fo_cli duplicates --hasher=xxh3

# 3. Review duplicate report
fo_cli export --format=html --thumbnails --output=duplicates.html

# 4. Delete duplicates (preview first)
fo_cli delete-duplicates --keep=oldest --dry-run
fo_cli delete-duplicates --keep=oldest

# 5. Organize by date
fo_cli organize --rule="{year}/{month}/" --dry-run /path/to/photos
fo_cli organize --rule="{year}/{month}/" /path/to/photos
```

### Incremental Maintenance

```bash
# Quick re-scan (only new/modified files)
fo_cli scan --incremental /path/to/photos

# Clean up deleted files
fo_cli scan --incremental --prune /path/to/photos

# Find new duplicates
fo_cli duplicates
```

### Document Scanning Workflow

```bash
# 1. Scan documents
fo_cli scan --ext=.pdf,.tiff,.png /path/to/scans

# 2. OCR all documents
fo_cli ocr --lang=eng /path/to/scans

# 3. Export searchable index
fo_cli export --format=json --output=documents.json
```

### Finding Similar Photos

```bash
# Find images similar to a reference
fo_cli similar --phash=phash --threshold=8 reference.jpg

# Process in batch
for img in /path/to/photos/*.jpg; do
  fo_cli similar --phash=phash "$img"
done
```

---

## Troubleshooting

### Common Issues

#### "Scanner not found"

```
Error: Scanner 'win32' not available
```

**Solution:** Use `--list-scanners` to see available implementations:

```bash
fo_cli --list-scanners
# Use an available scanner
fo_cli scan --scanner=std /path
```

#### "Database locked"

```
Error: database is locked
```

**Solution:** Close other instances of fo_cli or applications using the database.

#### "Tesseract not found"

```
Error: OCR provider 'tesseract' not available
```

**Solution:** Install Tesseract and ensure it's in PATH:

```bash
# Windows (chocolatey)
choco install tesseract

# Ubuntu
sudo apt install tesseract-ocr

# Set TESSDATA_PREFIX if needed
export TESSDATA_PREFIX=/usr/share/tesseract-ocr/4.00/tessdata/
```

#### "No files found"

```
Scan complete: 0 files
```

**Solution:** Check file extensions filter:

```bash
# Scan all files (no filter)
fo_cli scan /path/to/files

# Or specify correct extensions
fo_cli scan --ext=.jpg,.jpeg,.JPG,.JPEG /path/to/files
```

### Performance Tips

1. **Use incremental scans** for large libraries:
   ```bash
   fo_cli scan --incremental /path
   ```

2. **Use xxh3 for speed**, blake3 for verification:
   ```bash
   fo_cli duplicates --hasher=xxh3  # Fast
   fo_cli duplicates --hasher=blake3  # Secure
   ```

3. **Use Win32 scanner on Windows** for faster directory traversal:
   ```bash
   fo_cli scan --scanner=win32 /path
   ```

4. **Filter by extension** to reduce scan time:
   ```bash
   fo_cli scan --ext=.jpg,.png /path
   ```

### Getting Help

```bash
# General help
fo_cli --help

# Command-specific help
fo_cli scan --help
fo_cli organize --help

# List available providers
fo_cli --list-scanners
fo_cli --list-hashers
fo_cli --list-metadata
```

### Debug Logging

For verbose output, check the log file:

```bash
# Windows
type debug.log

# Linux/macOS
cat debug.log
```

---

## See Also

- [README.md](../README.md) - Project overview
- [README_CLI.md](../README_CLI.md) - CLI quick reference
- [DATABASE_SCHEMA.md](DATABASE_SCHEMA.md) - Database structure
- [LIBRARY_EVALUATION.md](LIBRARY_EVALUATION.md) - Provider benchmarks
- [SCANNER_EVALUATION.md](SCANNER_EVALUATION.md) - Scanner comparison

---

## License

See [LICENSE.txt](../LICENSE.txt) for licensing information.
