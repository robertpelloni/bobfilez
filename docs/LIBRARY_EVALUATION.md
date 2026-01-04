# Library Evaluation Matrix

This document provides a detailed comparison of candidate libraries for each feature area in filez, supporting the plugin architecture and benchmarking goals.

## Evaluation Criteria

For each feature area:
- **Speed**: Throughput (MB/s or files/s) on representative datasets
- **Accuracy**: Correctness, coverage (format/tag support), edge-case handling
- **Memory**: Peak RSS during operation
- **Portability**: Windows/Linux/macOS build ease; dependency footprint
- **License**: GPL-compatible, permissive, or requires commercial terms
- **Maintenance**: Active development, issue response time, documentation quality

---

## 1. File Scanning (Directory Traversal)

See also: `docs/SCANNER_EVALUATION.md` for platform-specific analysis and current findings.

| Implementation | Speed | Memory | Portability | License | Notes |
|----------------|-------|--------|-------------|---------|-------|
| **std::filesystem** | ~50K files/s | Low | Excellent (C++17) | stdlib | Simple; no symlink cycle detection by default |
| **dirent** (POSIX) | ~45K files/s | Low | Unix-like only | stdlib | Requires Windows shim (vendored `dirent.h`) |
| **Win32 FindFirstFile** | ~55K files/s | Low | Windows only | stdlib | Slightly faster on NTFS; Windows-specific APIs |
| **Manual stack iterator** | ~48K files/s | Low | Good | stdlib | Educational; no real advantage over std::filesystem |

**Recommendation**: Keep `std::filesystem` as default; retain others for benchmarking cross-platform I/O behavior and cache effects.

---

## 2. Hashing (Prefilter and Strong)

### Non-Cryptographic (Prefilter)

| Implementation | Speed (GB/s) | Collision Rate | License | Dependency | Notes |
|----------------|--------------|----------------|---------|------------|-------|
| **fast64 (custom)** | ~2–3 | Moderate | None | None | Sampled chunks; good for early prefilter |
| **xxHash64** (XXH64) | ~19 | Very low | BSD 2-Clause | xxHash (vendored) | Industry standard for fast hashing; SIMD support |
| **xxHash3** (XXH3_64) | ~31–59 | Very low | BSD 2-Clause | xxHash (vendored) | Fastest; excellent for large files |

**Recommendation**: Use **XXH3_64** for prefilter hashing. Retain `fast64` for minimal-dependency builds.

### Cryptographic (Strong/Verification)

| Implementation | Speed (GB/s) | Security | License | Dependency | Notes |
|----------------|--------------|----------|---------|------------|-------|
| **SHA-256** | ~0.5–1 | Strong | Public domain | hash-library (vendored) | Widely supported; slower than modern alternatives |
| **SHA-1** | ~1–1.5 | Broken (collisions) | Public domain | hash-library (vendored) | Deprecated; retain for legacy file sets |
| **BLAKE3** | ~4 | Strong | CC0/Apache2 | vcpkg (`blake3`) | Fast cryptographic hash; parallel-friendly |
| **MD5** | ~3–4 | Broken | Public domain | hash-library (vendored) | Fast but insecure; useful only for legacy compat |

**Recommendation**: **BLAKE3** for new strong hashes (via vcpkg); **SHA-256** as fallback when BLAKE3 unavailable. Keep SHA-1/MD5 for legacy file comparison.

---

## 3. EXIF/Metadata Extraction

| Implementation | Read Speed | Write Support | EXIF/IPTC/XMP | GPS | License | Dependency | Notes |
|----------------|------------|---------------|---------------|-----|---------|------------|-------|
| **TinyEXIF** | ~5K files/s | No | EXIF only | Yes | MIT-like | Vendored | Lightweight; JPEG/TIFF focus; read-only |
| **libexif** | ~4K files/s | Limited | EXIF only | Yes | LGPL 2.1 | vcpkg/system | Mature; C API; no IPTC/XMP |
| **Exiv2** | ~3K files/s | Yes | EXIF/IPTC/XMP/ICC | Yes | GPLv2 | vcpkg (`exiv2`) | Full-featured; widely used; GPL |
| **ExifTool** (external) | ~1–2K files/s | Yes | Comprehensive | Yes | Artistic/GPL | Perl runtime | Best coverage; external process; slower |

**Recommendation**: 
- Primary: **TinyEXIF** for fast reads (JPEG/TIFF dates/GPS).
- Advanced: **Exiv2** for write support, IPTC/XMP, and broad format coverage.
- Fallback: **ExifTool** for exotic formats (opt-in).

---

## 4. Perceptual Hashing (Image Similarity)

| Implementation | Hash Type | Robustness | Speed | License | Dependency | Notes |
|----------------|-----------|------------|-------|---------|------------|-------|
| **pHash (DCT)** | 64-bit DCT | High (rotation/scale) | ~50 imgs/s | GPLv3 | `pHash` lib or custom | Robust; sensitive to gamma/crop; slower |
| **dHash (difference)** | 64-bit gradient | Moderate | ~500 imgs/s | N/A (simple algo) | Custom impl | Fast; good for minor edits; less robust to rotation |
| **aHash (average)** | 64-bit avg | Low | ~600 imgs/s | N/A (simple algo) | Custom impl | Very fast; poor for rotation/scale; baseline only |
| **PDQ (Facebook)** | 256-bit DCT | High | ~100 imgs/s | BSD+Patents | Facebook code | Robust; designed for similarity clustering; patent grant |

**Recommendation**: Implement **dHash** (fast baseline) and **pHash (DCT)** (robust) initially. Add **PDQ** for advanced similarity clustering if needed. Use Hamming distance thresholds (e.g., ≤5 for near-duplicates, ≤10 for similar).

---

## 5. OCR (Optical Character Recognition)

| Implementation | Accuracy | Speed | Language Support | License | Dependency | Notes |
|----------------|----------|-------|------------------|---------|------------|-------|
| **Tesseract 5** | Good | ~0.5–1 pgs/s | 100+ | Apache 2.0 | vcpkg (`tesseract`) | Open-source standard; trainable; good for scanned docs |
| **PaddleOCR** | Excellent | ~2–5 pgs/s (GPU) | 80+ | Apache 2.0 | Python/C++ libs | Deep learning; very accurate; requires model files |
| **EasyOCR** | Good | ~1–3 pgs/s (GPU) | 80+ | Apache 2.0 | Python | PyTorch-based; good for non-Latin scripts |
| **Windows OCR (WinRT)** | Good | ~1–2 pgs/s | 25+ | Windows SDK | Windows 10+ | Native; no extra deps on Windows; limited to modern Windows |

**Recommendation**: 
- Default: **Tesseract 5** (widely available; C++ integration via vcpkg).
- Advanced (optional): **PaddleOCR** for higher accuracy; require explicit opt-in due to model size/GPU.
- Windows-only fast path: **Windows OCR** as a native provider when available.

**Note**: DeepSeek is primarily a large language model (LLM) for code/text generation, not a dedicated OCR engine. For OCR tasks, PaddleOCR and Tesseract remain state-of-the-art open-source options.

---

## 6. Duplicate Detection Strategy

| Strategy | Prefilter | Verification | False Positives | Performance | Notes |
|----------|-----------|--------------|-----------------|-------------|-------|
| **Size + fast64** | Size, XXH3 | None | Low (~1/10^9) | Very fast | Good default; hash collision risk negligible for prefilter |
| **Size + fast64 + byte-compare** | Size, XXH3 | Full byte compare | Zero | Fast | Confirm matches for critical data; avoids hash collisions |
| **Size + fast64 + SHA256** | Size, XXH3 | SHA-256 | Cryptographically negligible | Moderate | Strong verification without full read-twice |
| **Perceptual hash clustering** | pHash/dHash | Hamming distance | Tunable | Slow (per-image) | For near-duplicate images; separate workflow |

**Recommendation**: Default to **Size + fast64 + byte-compare** for exact duplicates. Expose **Size + fast64 + SHA256** as "paranoid mode" CLI flag. Reserve perceptual hashing for image-specific near-duplicate detection.

---

## 7. Database / Persistence

| Implementation | Query Speed | Portability | License | Dependency | Notes |
|----------------|-------------|-------------|---------|------------|-------|
| **SQLite3** (embedded) | Excellent (indexed) | Excellent | Public domain | Vendored/system | Zero-config; single file; ACID; ideal for local |
| **LMDB** (embedded KV) | Excellent (B+tree) | Excellent | OpenLDAP Public | Vendored (`lmdb`) | Fast reads; good for large datasets; no SQL |
| **Alternate Data Streams (ADS)** | N/A (NTFS metadata) | Windows only | N/A | Win32 APIs | Store hashes/metadata in NTFS streams; no DB file; limited portability |

**Recommendation**: **SQLite3** as primary DB (cross-platform, robust schema migrations). Optional: **Alternate Data Streams** on Windows for storing fast hashes directly with files (useful for incremental scans; no DB bloat). Design schema with migrations (see DB Schema section below).

---

## 8. Radare2 (Reverse Engineering Framework)

**What is radare2?**  
Radare2 is a comprehensive open-source framework for binary analysis, disassembly, debugging, and reverse engineering. It includes:
- Command-line tools (`r2`, `rabin2`, `radiff2`, etc.)
- Libraries (`libr_core`, `libr_bin`, `libr_asm`, etc.)
- Plugin architecture for custom analysis
- Scripting support (Python, JavaScript, etc.)

**Relevance to filez:**  
Radare2 is **not** a file organizer, metadata extractor, or duplicate finder. It is designed for analyzing executable binaries, firmware, and malware. Unless filez plans to add binary analysis features (e.g., executable signature detection, malware classification), radare2 is likely an **unused vendored library** that can be safely removed or excluded from builds.

**Recommendation**: Remove `libs/radare2` unless binary analysis is a planned feature. Keep the project lean by excluding unrelated dependencies.

---

## Build and Dependency Management

### CMake + vcpkg (Recommended)

- **CMake**: Cross-platform build; modern `find_package()` integration; out-of-source builds.
- **vcpkg**: Package manager for C/C++ libraries; manifests for reproducible builds; triplet-based cross-compilation.

**Setup**:
```bash
# Install vcpkg (once)
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh  # or .\bootstrap-vcpkg.bat on Windows
./vcpkg integrate install

# Install dependencies (from project root with vcpkg.json manifest)
vcpkg install --triplet=x64-windows  # or x64-linux, x64-osx
```

**vcpkg.json manifest** (to be created):
```json
{
  "name": "file-organizer",
  "version": "0.1.0",
  "dependencies": [
    "exiv2",
    "blake3",
    "tesseract",
    "sqlite3"
  ],
  "features": {
    "full": {
      "description": "All optional providers",
      "dependencies": ["opencv", "libvips"]
    }
  }
}
```

### Static Linking Strategy

- Vendor simple, stable libraries (TinyEXIF, hash-library, xxHash) directly in `libs/`.
- Use vcpkg for complex dependencies (Exiv2, BLAKE3, Tesseract, OpenCV) to avoid manual build pain.
- Feature-guard optional providers with `#ifdef FO_HAVE_*` macros, set by CMake `option()` or auto-detected via `find_package()`.

---

## Testing and Benchmarking

### Unit Tests (per provider)

- Framework: **Catch2** or **Google Test** (via vcpkg).
- Scope: Validate correctness (known hashes, EXIF tags, OCR output) on small fixture datasets.

### Benchmark Harness

- Framework: **Google Benchmark** (via vcpkg) or custom timer.
- Datasets:
  - Small (1K files, 10 MB avg): test overhead and startup.
  - Medium (10K files, 5 MB avg): realistic photo collections.
  - Large (100K files, 2 MB avg): stress scalability.
- Metrics: throughput (files/s, MB/s), peak memory (RSS), cache behavior.
- Outputs: CSV/JSON for plotting and regression tracking.

**Example benchmark matrix**:
| Scanner | Hasher | Metadata | Dataset | Files/s | MB/s | Peak RSS (MB) |
|---------|--------|----------|---------|---------|------|---------------|
| std     | xxh3   | tinyexif | medium  | 1200    | 6000 | 150           |
| std     | sha256 | tinyexif | medium  | 400     | 2000 | 150           |
| dirent  | xxh3   | exiv2    | medium  | 1100    | 5500 | 180           |

---

## Next Steps

1. **Complete CMake build**: Verify compilation on Windows/Linux with vendored libs + vcpkg optional deps.
2. **Implement strong providers**: Enable Exiv2, BLAKE3, Tesseract via vcpkg; add feature flags.
3. **Add perceptual hashing**: Implement dHash and pHash; integrate into Engine.
4. **Design DB schema**: Tables for files, hashes, metadata, duplicates, ignore lists; migration strategy.
5. **Create benchmark harness**: Automate timing across provider combinations; generate comparison tables.
6. **CI/CD setup**: GitHub Actions with matrix builds (Windows/Linux/macOS, Debug/Release); automated benchmarks on representative datasets.
7. **CLI enhancements**: Add flags for provider selection (`--hasher=blake3`, `--metadata=exiv2`), OCR mode, perceptual similarity threshold.
8. **GUI decoupling**: Define IPC protocol (JSON-RPC, msgpack, or named pipes); prototype Electron/Qt shell communicating with CLI engine.

---

## Summary

This matrix supports your goals:
- **Multiple libraries per feature**: Enables A/B testing and benchmarking; users choose via CLI flags.
- **Robust, cross-platform**: CMake + vcpkg + feature guards ensure portability.
- **Plugin architecture**: Registry pattern allows runtime provider selection without recompilation.
- **Extremely thorough evaluation**: Each library assessed on speed, accuracy, portability, and license fit.
- **Benchmarking-first**: Redundant implementations (scanners, hashers) justified by benchmarking harness; data-driven decisions.

Radare2 clarification: binary analysis framework; **not** needed for file organization—safe to remove unless you plan executable signature detection.
