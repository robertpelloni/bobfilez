# Benchmark Results

**Date:** 2025-12-31
**Version:** 2.1.0
**Platform:** Windows 10/11 x64 (32 Logical Cores)

## Summary

This document records the performance benchmarks for the core components of filez: File Scanning and Hashing.

## 1. File Scanning

We compared the standard C++ `std::filesystem` recursive iterator against a native Win32 `FindFirstFile/FindNextFile` implementation.

| Method | Time (ns) | Time (ms) | Notes |
| :--- | :--- | :--- | :--- |
| `ScannerFixture/ScanStd` | 2,498,961 | ~2.50 | Standard C++20 iterator |
| `ScannerFixture/ScanWin32` | 2,895,888 | ~2.90 | Native Win32 API |

**Observation:**
Currently, `std::filesystem` is performing slightly faster (~14%) than the raw Win32 implementation for our test fixture. This suggests that for typical workloads, the standard library is sufficiently optimized and we do not strictly need the complexity of the Win32 scanner unless dealing with specific edge cases (e.g., paths > 260 chars, which `std::filesystem` handles well on modern Windows anyway).

## 2. Hashing

We compared the performance of our fast non-cryptographic hash (xxHash64) against a strong cryptographic hash (BLAKE3).

| Hasher | Time (ns) | Bandwidth | Use Case |
| :--- | :--- | :--- | :--- |
| `BM_Hasher_Fast64` | 360,307 | **2.77 GiB/s** | Quick deduplication, change detection |
| `BM_Hasher_Blake3` | 2,388,829 | 426.67 MiB/s | Integrity verification, crypto-signing |

**Observation:**
The fast hasher is approximately **6.6x faster** than the cryptographic hasher.
- Use `Fast64` for initial file scanning and database lookup.
- Use `Blake3` only when explicitly requested by the user or for exact bit-by-bit verification.

## Methodology

Benchmarks were run using Google Benchmark.
Command: `.\build\benchmarks\fo_benchmarks.exe`
