# Scanner Providers: Windows vs. dirent vs. C++ std::filesystem

This document records the current status, analysis, and findings for the first pluggable choice in filez: the directory scanner and file metadata provider. It is designed to guide future contributors and AI agents.

---

## TL;DR Recommendations

- Windows: use the `win32` scanner by default for maximum performance and rich metadata.  
- Cross-platform: use `std` (C++20 `std::filesystem`) for portability and simplicity.  
- Alternative on Windows/Linux: `dirent` can be competitive with `win32` on pure enumeration speed but exposes less metadata; consider when you only need names/paths and basic `stat` fields.

CLI selection examples:

```
fo_cli --scanner=win32  C:\Photos
fo_cli --scanner=dirent /mnt/data
fo_cli --scanner=std    /data
```

---

## Providers Compared

1) win32 (Windows API)
- API: `FindFirstFileExW`/`FindNextFileW`, `GetFileAttributesExW`, optional `GetFileInformationByHandleEx` for extended info
- Pros: fastest on Windows, richest metadata (file IDs, reparse points, attributes, precise timestamps)
- Cons: Windows-only; requires careful UTF-16↔UTF-8 conversion; more verbose code

2) dirent (Portable POSIX-style)
- API: `opendir`/`readdir` + `stat`
- Pros: very low overhead; near `win32` performance in some cases; simple and familiar
- Cons: limited metadata without additional syscalls; on Windows relies on a compatibility layer; path normalization/UTF handling is tricky

3) std (C++20 std::filesystem)
- API: `recursive_directory_iterator`, `directory_entry::file_size/last_write_time`
- Pros: portable, readable, exception-safe; good default for cross-platform code
- Cons: typically slower on Windows due to iterator abstraction and conversions; less control over source syscalls and flags

---

## What "Metadata" Means Here

Minimum fields we rely on today:
- Path (UTF-8), size (bytes), last write time (nanoseconds precision when available)

Extended fields we may use next:
- File ID / inode, number of hard links, attributes/flags (hidden, system, archive), reparse tag / symlink target, creation time, access time
- Alternate Data Streams (ADS) count and names (Windows NTFS)

Coverage by provider:
- win32: best (can get everything above; ADS needs backup semantics or `FindFirstStreamW`)
- dirent: basic (path + `stat` for size, times; inode on POSIX)
- std: basic (path, size, write time, sometimes file type); extended fields not uniformly exposed

---

## Current Findings (Windows, anecdotal)

- The native Windows functions are fastest and most complete on Windows (unsurprising).
- `dirent` is nearly tied in raw enumeration speed in several cases, especially when only names and basic stats are required.
- `std::filesystem` lags both on Windows in most scenarios due to abstraction overhead and exception handling costs.

Note: These are practical observations from prior use and early tests. The project includes a proper benchmarking harness plan to formalize and reproduce measurements (see `docs/BENCHMARKING_PLAN.md`).

---

## Implementation Notes

- Recursion strategy: iterative stack preferred to avoid deep recursion costs and to handle very large directory trees reliably.
- Symlink policy: disabled by default; configurable `--follow-symlinks` flag. On Windows, check reparse points; on POSIX, `S_ISLNK`.
- Extension filtering: case-insensitive, supports `.jpg` and `jpg` forms; internal normalization to a lowercase set.
- Error handling: swallow permission errors by default and continue; count and surface as warnings.
- Encoding:
  - Windows: use wide-char APIs and convert to UTF-8 for internal representation.
  - POSIX: assume input paths are UTF-8; avoid lossy conversions.

---

## Planned Benchmarks (excerpt)

Metrics:
- Throughput (files/sec) on large trees
- Latency to first N files
- Syscall counts (perf when available)
- Memory footprint during traversal
- Accuracy: file counts and metadata parity across providers

Measurement protocol:
- Warm/cold cache runs (run each test 5×, report median).
- Separate "names only" vs "names+stat" modes where applicable.
- Exclude antivirus interference by using a stable dataset path when possible.

Datasets:
- Windows system drive sample (Program Files, Users, Windows)
- Photo library with deep nesting and long names

Reporting template:
```
Provider: win32 | dirent | std
Dataset: <name>
Mode: names | names+stat
Files: <count>  Dirs: <count>
Median Throughput: <files/s>  p90: <files/s>
RSS peak: <MB>
Notes: <edge cases, errors>
```

---

## Decision Matrix (Summary)

- Windows desktop users: choose `win32` by default
- Mixed OS environments or portability-first: choose `std`
- Server-side batch listing or very simple metadata needs: `dirent` can be competitive

---

## Next Steps

- Implement `win32` and `dirent` providers alongside existing `std` provider
- Expose provider names in `--scanner=` and include in `--help`
- Add microbenchmarks (`BM_Scanner_*`) once the benchmark harness lands
- Extend metadata structs to optionally carry file IDs and Windows attributes when available

---

Maintained by: filez core team
