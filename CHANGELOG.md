# Changelog

## [2.5.0] - 2026-04-02

### Added
- **Advanced Media Analysis Engine**:
    - **Video Perceptual Hashing (`vhash`)**: Implemented an FFmpeg-backed video analyzer that extracts evenly-spaced frames, converts them to grayscale thumbnails, and generates 64-bit dHash values to identify visually similar or duplicate video files regardless of resolution, codec, or frame rate.
    - **Audio Fingerprinting (`afingerprint`)**: Implemented an FFmpeg + Chromaprint integration to generate AcoustID-compatible audio fingerprints. Capable of identifying identical audio content across differing formats (mp3 vs flac), bitrates, or volume levels.
    - New CLI commands: `fo_cli vhash <file>`, `fo_cli afingerprint <file>`, `--list-vhash`, `--list-ahash`.
- **vcpkg Dependencies**: Added `ffmpeg` and `chromaprint` to `vcpkg.json` for media analysis support.
- **Documentation**: Substantial updates to the project's vision, architecture, and roadmap based on deep-dive codebase analysis.

## [2.4.3] - 2026-04-02

### Added
- **OmniUI Explorer Enhancements**:
    - Windows 11 style tabbed title bar.
    - Functional address bar and search field skeletons.
    - Detailed file list view with Name, Size, Type, and Date Modified.
    - Navigation sidebar with "This PC", "OneDrive", etc.
    - Status bar showing item and selection counts.
- **Documentation**:
    - `docs/COMPETITIVE_ANALYSIS_DETAILED.md`: Analysis of Files, File Pilot, and OpenSalamander.
    - `EXPLORER_PLAN.md`: Detailed implementation roadmap for the shell replacement, including a new "Integrated Image Management Suite" category and an "Advanced Dual-Pane Operations (Salamander Parity)" category.

### Improved
- mica-style visual effects and drop shadows for the explorer window.
- Updated `HANDOFF.md` for session transition.
