# Architectural Analysis & Vision Summary

**Date:** 2026-04-01
**Status:** Comprehensive Analysis Complete

## Executive Summary
**filez** is a high-performance, local-first file organization and deduplication engine. Its design is governed by three "First Principles":
1. **User Sovereignty:** "You choose your trade-offs." 
2. **Empirical Foundation:** Benchmarking-first approach for all providers.
3. **CLI-First / GUI-Optional:** Core engine (`fo_core`) is UI-agnostic.

## 1. Comprehensive Library Taxonomy
The project utilizes 130+ submodules, categorized as follows:

### Hashing & Integrity
- **Fast:** `xxHash` (XXH3, XXH64), `Fast64`.
- **Strong:** `BLAKE3`, `OpenSSL` (SHA-256).

### Filesystem & Discovery
- **Scanners:** `std::filesystem`, `Win32 FindFirstFile`, `dirent`, `fstlib`.
- **Metadata Pinning:** `lsads`, `SharpADS` (NTFS ADS), `xattrs`, `metastore`.

### Metadata & Media
- **Images:** `Exiv2`, `TinyEXIF`, `libexif`, `jhead`.
- **Video/Audio:** `MediaInfoLib`, `FFmpeg`, `ShazamAPI`.
- **VFX Standards:** `OpenImageIO`, `OpenColorIO`, `OpenTimelineIO`.

### Intelligence (AI/OCR)
- **Engines:** `ONNX Runtime`, `Tesseract`, `PaddleOCR`.
- **Libraries:** `OpenCV`, `libvips`, `ImageMagick`.

### Infrastructure
- **Persistence:** `SQLite3`, `LMDB`, `PostgreSQL`.
- **History/Undo:** `libgit2`.
- **UI:** `Qt6`, `lvgl`, `raylib`.

## 2. Core Architectural Pillars

### A. Date Fusion Strategy
Prioritizes metadata sources to find the "true" creation date:
1. EXIF `DateTimeOriginal`
2. EXIF `CreateDate`
3. Filename Regex patterns
4. Filesystem `mtime`

### B. Verification Modes
- **Fast:** Size + xxHash.
- **Safe:** Size + xxHash + BLAKE3.
- **Paranoid:** Full byte-compare.

### C. Stateless Incremental Scanning
Uses Alternate Data Streams (ADS) and Extended Attributes (Xattrs) to cache hashes directly on the file, allowing instant re-validation without a central database dependency.

## 3. Future Roadmap
- **Media Fingerprinting:** Perceptual hashing for Video and Audio.
- **Remote GUI:** Browser-based management for headless servers.
- **Cloud Interop:** Optional S3/GCS/Azure sync (Post-v1.0).
