# Competitive Analysis: Files vs. File Pilot vs. bobfilez (OmniUI)

## 1. Files (github.com/files-community/Files)
**Target:** Modern Windows users seeking a native-feeling WinUI 3 experience.

### Key Strengths
- **Native Design:** Perfect alignment with Windows 11 Fluent design.
- **Tabs & Panes:** Advanced multi-tasking features (tabs, dual-pane, column view).
- **Cloud Integration:** First-class support for OneDrive, Google Drive, etc.
- **Polish:** Extremely well-documented, localized, and feature-rich.

### Comparison to bobfilez
- **Tech Stack:** C# / WinUI 3 vs. C++ / Qt (OmniUI). bobfilez is focused on high performance and cross-platform (native) execution beyond just Windows.
- **Core Engine:** Files is primarily a shell wrapper; bobfilez is built on a high-performance deduplication and indexing engine (`fo_core`).
- **Vision:** Files is an *alternative* explorer; bobfilez aims to be a *replacement* for `explorer.exe` as part of a complete OS shell (`bob_shell`).

---

## 2. File Pilot (filepilot.tech)
**Target:** Users who prioritize speed, indexing, and minimal UI.

### Key Strengths
- **Speed:** High-performance indexing and search.
- **Minimalism:** Clean, focused UI.
- **Efficiency:** Strong keyboard-centric workflow.

### Comparison to bobfilez
- **Scope:** File Pilot is a specialized tool for search and organization; bobfilez is a complete shell and file manager replacement.
- **Engine Power:** bobfilez's `fo_core` engine is more robust, supporting advanced hashing, perceptual hashing, and AI-driven classification.

---

## 3. bobfilez (OmniUI Explorer)
**Target:** High-performance shell replacement for power users and developers.

### Unique Selling Points
- **Deduplication Engine:** Integrated `fo_core` for real-time duplicate detection.
- **Date Fusion:** Advanced algorithm to find the "true" creation date of media files.
- **Stateless Incremental Scanning:** Uses ADS/Xattrs for zero-DB indexing.
- **OmniUI/Qt Fork:** Native, high-performance UI that mimics Windows 11 aesthetics but runs with C++ speed.
- **Cross-Platform Vision:** Designed for Windows, Linux, and potentially beyond.

---

## Conclusion
While `Files` provides the best visual experience today, `bobfilez` is building a more powerful *engine* that will eventually surpass existing tools in performance, robustness, and core shell integration. The transition to OmniUI (Qt) allows bobfilez to close the visual gap with `Files` while maintaining its performance lead.
