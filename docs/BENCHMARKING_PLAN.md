# Benchmarking Plan

filez maintains multiple implementations per feature (scanners, hashers, metadata providers) to enable empirical comparison and optimization. This document defines datasets, metrics, methodology, and reporting.

---

## Goals

1. **Validate provider selection**: Confirm fastest/most accurate implementation for each feature.
2. **Detect regressions**: Track performance over time; flag slowdowns in CI.
3. **Optimize I/O patterns**: Compare cache-friendly vs. random access; identify bottlenecks.
4. **Inform user choices**: Document trade-offs (speed vs. accuracy, memory vs. throughput) so users can choose providers via CLI flags.

---

## Benchmark Datasets

### Dataset 1: Small (Overhead Focus)

- **Size**: 1,000 files
- **Avg file size**: 10 MB (JPEG photos)
- **Total**: ~10 GB
- **Purpose**: Measure overhead (thread startup, metadata parsing) vs. I/O time.
- **Location**: `benchmarks/datasets/small/`

### Dataset 2: Medium (Realistic Collection)

- **Size**: 10,000 files
- **Avg file size**: 5 MB (mixed JPEG/PNG/TIFF)
- **Total**: ~50 GB
- **Purpose**: Simulate typical photo library; balance I/O and CPU.
- **Location**: `benchmarks/datasets/medium/`

### Dataset 3: Large (Scalability Test)

- **Size**: 100,000 files
- **Avg file size**: 2 MB (smaller images, documents)
- **Total**: ~200 GB
- **Purpose**: Stress memory usage, cache behavior, and parallelism.
- **Location**: `benchmarks/datasets/large/`

### Dataset 4: Duplicate-Heavy

- **Size**: 5,000 files (1,000 unique + 4,000 duplicates)
- **Avg file size**: 3 MB
- **Total**: ~15 GB
- **Purpose**: Test duplicate grouping efficiency (hash collisions, byte-compare).
- **Location**: `benchmarks/datasets/duplicates/`

### Dataset 5: EXIF-Rich

- **Size**: 2,000 files (high-resolution JPEGs with full EXIF/GPS)
- **Avg file size**: 8 MB
- **Total**: ~16 GB
- **Purpose**: Measure metadata extraction speed and accuracy.
- **Location**: `benchmarks/datasets/exif/`

**Acquisition**: Use public datasets (e.g., COCO, Flickr Creative Commons) or generate synthetic files with known hashes/metadata.

---

## Metrics

### 1. Throughput

- **Files/s**: Files processed per second.
- **MB/s**: Megabytes read per second (useful for hash benchmarks).

### 2. Latency

- **Scan time**: Time to enumerate all files (directory traversal).
- **Hash time**: Time to compute hashes (prefilter + optional strong).
- **Metadata time**: Time to extract EXIF/dates/GPS.
- **Duplicate grouping time**: Time to identify and cluster duplicates.

### 3. Memory Usage

- **Peak RSS**: Maximum resident set size (via OS tools or instrumentation).
- **Average RSS**: Typical memory during steady-state operation.

### 4. Accuracy

- **Hash collision rate**: False positives in prefilter hashes (should be ~0 for xxHash).
- **Metadata extraction rate**: % of files with successfully parsed dates/GPS (vs. ground truth).
- **Duplicate detection rate**: True positives / (true positives + false negatives); requires labeled dataset.

### 5. Cache Behavior (Advanced)

- **Cache miss rate**: L3 cache misses per file (via `perf` on Linux, Instruments on macOS, VTune on Windows).
- **I/O wait %**: Time blocked on disk reads vs. CPU time.

---

## Benchmark Matrix

### Scanners (Directory Traversal)

| Scanner | Dataset | Files/s | Peak RSS (MB) | Notes |
|---------|---------|---------|---------------|-------|
| std::filesystem | Small | ? | ? | Baseline |
| std::filesystem | Medium | ? | ? | |
| std::filesystem | Large | ? | ? | |
| dirent | Small | ? | ? | POSIX walker |
| Win32 FindFirstFile | Small | ? | ? | Windows-only |
| Manual stack iterator | Small | ? | ? | Educational |

**Vary**: Follow symlinks (on/off), SSD vs. HDD, cold vs. warm cache.

---

### Hashers (Prefilter)

| Hasher | Dataset | MB/s | Peak RSS (MB) | Notes |
|--------|---------|------|---------------|-------|
| fast64 (custom) | Medium | ? | ? | Sampled chunks |
| XXH64 | Medium | ? | ? | Full file |
| XXH3_64 | Medium | ? | ? | SIMD-optimized |
| XXH3_64 (sampled) | Medium | ? | ? | Chunks only |

**Vary**: File size distribution, chunk size (4KB, 16KB, 64KB).

---

### Hashers (Strong/Verification)

| Hasher | Dataset | MB/s | Peak RSS (MB) | Notes |
|--------|---------|------|---------------|-------|
| SHA-256 | Medium | ? | ? | Cryptographic |
| SHA-1 | Medium | ? | ? | Legacy |
| MD5 | Medium | ? | ? | Fast but broken |
| BLAKE3 | Medium | ? | ? | Modern crypto |

**Vary**: Parallel hashing (multi-threaded) vs. serial.

---

### Metadata Providers

| Provider | Dataset | Files/s | Extraction Rate (%) | Peak RSS (MB) | Notes |
|----------|---------|---------|---------------------|---------------|-------|
| TinyEXIF | EXIF-Rich | ? | ? | ? | Read-only |
| libexif | EXIF-Rich | ? | ? | ? | EXIF only |
| Exiv2 | EXIF-Rich | ? | ? | ? | Full-featured |
| ExifTool (external) | EXIF-Rich | ? | ? | ? | Perl process |

**Vary**: File formats (JPEG, TIFF, HEIF, RAW), tags extracted (date vs. full EXIF).

---

### Duplicate Detection Strategies

| Strategy | Dataset | Files/s | False Positives | False Negatives | Peak RSS (MB) | Notes |
|----------|---------|---------|-----------------|-----------------|---------------|-------|
| Size + fast64 | Duplicates | ? | ? | ? | ? | Hash-only |
| Size + fast64 + byte-compare | Duplicates | ? | 0 | ? | ? | Verified |
| Size + fast64 + SHA256 | Duplicates | ? | 0 | ? | ? | Crypto-verified |

**Vary**: Duplicate ratio (10%, 50%, 80%).

---

### Perceptual Hashing (Image Similarity)

| Hash Type | Dataset | Images/s | Precision | Recall | Peak RSS (MB) | Notes |
|-----------|---------|----------|-----------|--------|---------------|-------|
| dHash | Medium | ? | ? | ? | ? | Fast gradient |
| pHash (DCT) | Medium | ? | ? | ? | ? | Robust |
| aHash | Medium | ? | ? | ? | ? | Baseline |

**Ground truth**: Use labeled near-duplicate pairs (e.g., rotated, cropped, gamma-adjusted).

---

### OCR Providers

| Provider | Dataset | Pages/s | Accuracy (%) | Peak RSS (MB) | Notes |
|----------|---------|---------|--------------|---------------|-------|
| Tesseract 5 | OCR-Test | ? | ? | ? | CPU |
| PaddleOCR | OCR-Test | ? | ? | ? | GPU |
| Windows OCR | OCR-Test | ? | ? | ? | Native |

**Ground truth**: Use labeled scanned documents (e.g., FUNSD, SROIE).

---

## Methodology

### Harness Design

1. **Framework**: Google Benchmark (C++) or custom timer.
2. **Repetitions**: 3–5 runs per configuration; report median and stddev.
3. **Warm-up**: 1 iteration to prime OS caches before measurement.
4. **Isolation**: Disable background tasks (antivirus, indexing) during benchmarks.

### Code Structure

```cpp
#include <benchmark/benchmark.h>
#include "fo/core/engine.hpp"

static void BM_Scanner_Std_Medium(benchmark::State& state) {
    fo::core::EngineConfig cfg{.scanner = "std", .hasher = "xxh3"};
    fo::core::Engine engine(cfg);
    std::vector<std::filesystem::path> roots = {"benchmarks/datasets/medium"};
    for (auto _ : state) {
        auto files = engine.scan(roots, {".jpg", ".png"}, false);
        benchmark::DoNotOptimize(files);
    }
    state.SetItemsProcessed(state.iterations() * 10000); // 10K files
    state.SetBytesProcessed(state.iterations() * 50ULL * 1024 * 1024 * 1024); // 50 GB
}
BENCHMARK(BM_Scanner_Std_Medium)->Unit(benchmark::kSecond);

BENCHMARK_MAIN();
```

### Memory Profiling

- **Linux**: `valgrind --tool=massif` or `heaptrack`.
- **Windows**: Visual Studio Performance Profiler or `Windows Performance Toolkit`.
- **macOS**: Instruments (Allocations template).

### Cache Analysis (Optional)

- **Linux**: `perf stat -e cache-misses,cache-references`.
- **Windows**: Intel VTune or AMD uProf.
- **macOS**: Instruments (CPU Counters).

---

## Reporting

### Output Format

**CSV** (for plotting/regression tracking):
```csv
benchmark,scanner,hasher,metadata,dataset,files_per_sec,mb_per_sec,peak_rss_mb,mean_latency_ms,stddev_ms
scan_std_xxh3,std,xxh3,none,medium,1200,6000,150,8.3,0.5
scan_std_sha256,std,sha256,none,medium,400,2000,150,25.0,1.2
```

**Markdown table** (for docs):
| Benchmark | Scanner | Hasher | Dataset | Files/s | MB/s | Peak RSS (MB) | Notes |
|-----------|---------|--------|---------|---------|------|---------------|-------|
| Scan + Hash | std | xxh3 | medium | 1200 | 6000 | 150 | Baseline |
| Scan + Hash | std | sha256 | medium | 400 | 2000 | 150 | 3× slower |

**Graphs** (via Python/gnuplot):
- Line chart: Throughput vs. file size.
- Bar chart: Provider comparison (side-by-side).
- Heatmap: Cache miss rate vs. chunk size.

---

## CI Integration

### GitHub Actions Workflow

```yaml
name: Benchmarks

on:
  push:
    branches: [main, develop]
  schedule:
    - cron: '0 2 * * 0' # Weekly on Sunday

jobs:
  benchmark:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
    steps:
      - uses: actions/checkout@v3
      - name: Install dependencies
        run: |
          vcpkg install --triplet=${{ matrix.triplet }}
      - name: Build benchmarks
        run: |
          cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
          cmake --build build --target fo_benchmarks
      - name: Download datasets
        run: |
          curl -o datasets.tar.gz https://example.com/benchmark-datasets.tar.gz
          tar -xzf datasets.tar.gz -C benchmarks/
      - name: Run benchmarks
        run: |
          ./build/benchmarks/fo_benchmarks --benchmark_format=csv > results.csv
      - name: Upload results
        uses: actions/upload-artifact@v3
        with:
          name: benchmark-results-${{ matrix.os }}
          path: results.csv
      - name: Regression check
        run: |
          python scripts/check_regression.py results.csv baseline.csv
```

### Regression Detection

- Store baseline CSVs in repo (`benchmarks/baseline/`).
- Compare new results; fail CI if throughput drops >10% without explanation.
- Auto-comment on PRs with performance summary.

---

## Example Results (Hypothetical)

### Scanners (Medium Dataset)

| Scanner | Files/s | Peak RSS (MB) | Notes |
|---------|---------|---------------|-------|
| std::filesystem | 1200 | 120 | Baseline |
| dirent | 1100 | 115 | Slightly slower; lower overhead |
| Win32 FindFirstFile | 1300 | 110 | Fastest on Windows/NTFS |
| Manual stack | 1150 | 125 | No advantage over std |

**Conclusion**: Use `std::filesystem` for portability; fallback to `Win32` on Windows if speed critical.

---

### Hashers (Medium Dataset)

| Hasher | MB/s | Files/s | Notes |
|--------|------|---------|-------|
| XXH3_64 (full) | 6000 | 1200 | Fastest non-crypto |
| XXH3_64 (sampled) | 8000 | 1600 | Faster but ~1/10^6 collision risk |
| SHA-256 | 2000 | 400 | Crypto-strong |
| BLAKE3 | 4000 | 800 | Fast crypto |

**Conclusion**: Use **XXH3_64 (sampled)** for prefilter; **BLAKE3** for strong verification.

---

### Metadata (EXIF-Rich Dataset)

| Provider | Files/s | Extraction Rate (%) | Notes |
|----------|---------|---------------------|-------|
| TinyEXIF | 800 | 95 | Fast; JPEG/TIFF only |
| Exiv2 | 600 | 98 | Slower; broader format support |
| ExifTool | 150 | 99 | Slowest; best coverage |

**Conclusion**: Use **TinyEXIF** for JPEG/TIFF; fallback to **Exiv2** for RAW/HEIF; reserve **ExifTool** for exotic formats.

---

## Next Steps

1. **Generate datasets**: Use synthetic files or public datasets; label ground truth for duplicates/OCR.
2. **Implement benchmark harness**: Add `benchmarks/` directory with Google Benchmark suite.
3. **Run baselines**: Measure current performance on all matrix cells; commit CSVs as baseline.
4. **Optimize hot paths**: Profile with `perf`/VTune; parallelize I/O-bound operations.
5. **Automate CI**: Integrate benchmark runs into GitHub Actions; flag regressions.
6. **Document trade-offs**: Update `LIBRARY_EVALUATION.md` with real numbers; guide users.

---

## Summary

This benchmarking plan:
- Defines realistic datasets covering common use cases (small/large, duplicate-heavy, EXIF-rich).
- Measures throughput, memory, and accuracy across all provider combinations.
- Enables data-driven decisions on default providers and optional alternatives.
- Integrates with CI for regression detection and historical tracking.
- Supports your goal of **extremely robust** evaluation via empirical comparison, not assumptions.

Redundant implementations (multiple scanners, hashers) are justified by this systematic benchmarking framework.
