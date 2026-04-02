# filez TODO List

## Immediate (Short Term)
- [ ] **BobUI Build Integration**: Integrate `libs/bobui` (the Qt fork) into the main CMake build system as the primary GUI.
- [ ] **`fo_gui` Refactor**: Update the existing GUI to leverage the custom BobUI framework for performance and a premium feel.
- [ ] **Video Perceptual Hashing**: Research and implement a robust dHash/pHash for video frames via FFmpeg.
- [ ] **Audio Fingerprinting**: Integrate `ShazamAPI` or similar for deduplicating audio by signature.
- [ ] **Cloud Pagination Stress Test**: Test S3/Google Drive/Azure with >1000 objects to ensure stability.
- [ ] **MSI Installer**: Verify WiX build on a clean machine with the latest `VERSION.md` sync.

## In Progress
- [ ] **Java 21 Port Refinement**: Update the Java version with latest CLI parity for `stats` and `--mode`.
- [ ] **Fuzz Testing Campaigns**: Continuous libFuzzer execution on the `RuleEngine`.
- [ ] **Packaging**: AppImage and DMG generation for each build.

## Bug Fixes
- [ ] Resolve any remaining linker issues with `vcpkg` and cloud SDKs in non-standard environments.
- [ ] Update `generate_dashboard.py` to handle nested submodules (submodules of submodules).

## Low Priority
- [ ] Web GUI (`bobui_web`) polish and additional REST API endpoints.
- [ ] Example `rules.yaml` files for various user personas (Photographer, SysAdmin).
