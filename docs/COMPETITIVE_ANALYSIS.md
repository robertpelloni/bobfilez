# Competitive Analysis

This document analyzes other major file management and organization software to identify standout or unique features.

## 1. dupeGuru

**Website:** [https://dupeguru.voltaicideas.net/](https://dupeguru.voltaicideas.net/)

**Key Features:**

*   **Cross-platform GUI:** Works on Linux, macOS, and Windows.
*   **Fuzzy Matching:** Can find duplicate filenames even when they are not exactly the same.
*   **Specialized Modes:**
    *   **Music Mode:** Scans tags and shows music-specific information.
    *   **Picture Mode:** Scans pictures fuzzily to find similar, but not identical, images.
*   **Customizable Matching Engine:** Users can tweak the matching engine to find specific types of duplicates.
*   **Safe Deletion:** A reference directory system and grouping system prevent accidental deletion of files.
*   **Multiple Actions:** Users can delete, move, or copy duplicate files.

**Unique Selling Points:**

*   The specialized modes for music and pictures are a significant advantage over general-purpose duplicate finders.
*   The fuzzy matching algorithm for filenames is a powerful feature for finding duplicates with minor variations in their names.

## 2. rmlint

**Website:** [https://rmlint.readthedocs.io/](https://rmlint.readthedocs.io/)

**Key Features:**

*   **Command-line focused:** Designed for speed and efficiency.
*   **Finds More Than Just Duplicates:** Can also find:
    *   Non-stripped binaries
    *   Broken symbolic links
    *   Empty files and directories
    *   Files with broken user or group IDs
*   **Flexible Output:** Can produce executable shell scripts to delete files.
*   **Caching and Replaying:** Can save scan results and replay them later.
*   **BTRFS Support:** Can leverage BTRFS features for more efficient deduplication.

**Unique Selling Points:**

*   The ability to find a wide range of "lint" beyond just duplicate files is a key differentiator.
*   The focus on performance and command-line automation makes it a powerful tool for power users and system administrators.

## 3. jdupes

**Website:** [https://github.com/jbruchon/jdupes](https://github.com/jbruchon/jdupes)

**Key Features:**

*   **Speed:** Claims to be significantly faster than other duplicate scanners.
*   **Data Safety:** Prioritizes data safety over performance.
*   **Native Windows Port:** Supports Unicode and other Windows-specific features.
*   **Block-level Deduplication:** Can use filesystem-level deduplication (reflink/CoW) on supported filesystems (BTRFS, XFS, APFS).

**Unique Selling Points:**

*   The focus on both speed and data safety is a compelling combination.
*   The support for block-level deduplication is a powerful feature that can save significant disk space.

## 4. Czkawka

**Website:** [https://github.com/qarmin/czkawka](https://github.com/qarmin/czkawka)

**Key Features:**

*   **Cross-platform GUI and CLI:** Available for Linux, macOS, and Windows.
*   **Multiple Tools:** Includes tools for finding:
    *   Duplicates
    *   Empty folders
    *   Large files
    *   Empty files
    *   Temporary files
    *   Similar images
    *   Similar videos
    *   Same music
    *   Invalid symbolic links
    *   Broken files
*   **Advanced Features:**
    *   Finds similar images using pHash.
    *   Finds similar videos.
    *   Finds music duplicates by artist, album, etc.
*   **Fast:** Written in Rust for performance.

**Unique Selling Points:**

*   The sheer number of tools and features is impressive.
*   The ability to find similar videos is a unique feature.

## Summary

| Feature                 | dupeGuru | rmlint | jdupes | Czkawka | filez (Planned) |
| ----------------------- | :------: | :----: | :----: | :-----: | :---------------------: |
| GUI                     |   Yes    |   No   |   No   |   Yes   |           Yes           |
| CLI                     |    No    |  Yes   |  Yes   |   Yes   |           Yes           |
| Fuzzy Filename Matching |   Yes    |   No   |   No   |   No    |           No            |
| Similar Image Detection |   Yes    |   No   |   No   |   Yes   |           Yes           |
| Similar Video Detection |    No    |   No   |   No   |   Yes   |           No            |
| Music Tag Comparison    |   Yes    |   No   |   No   |   Yes   |           No            |
| Find "Lint"             |    No    |  Yes   |   No   |   Yes   |           No            |
| Block-level Dedupe      |    No    |  Yes   |  Yes   |   No    |           No            |
| Cross-platform          |   Yes    |  Yes   |  Yes   |   Yes   |           Yes           |
