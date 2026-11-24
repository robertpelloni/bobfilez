# Memory: Observations & Design Preferences

## Observations (2026-11-24, v6.5.45)
- **Provider matrix is complete in the lean lane**: scanners gdrive/std/win32/dirent/s3/azure, hashers fast64/sha256/xxhash/blake3/dhash, tinyexif, tesseract OCR, onnx classifier, dhash/phash/ahash/opencv perceptual, ffmpeg video, chromaprint audio — all live via `/api/providers`. Provider registration must be explicit `register_*()` calls; TU-local static registrars get linker-dropped (bit us three times: perceptual, vault, classifiers).
- **Destructive ops are now failure-honest**: organize/delete-duplicates/rename/rename-batch report only successes, count failures, exit nonzero on failure, refuse existing destinations (MSVC rename replaces!), skip the live DB files, and scope delete-duplicates to the given folder. Undo skips irreversible/stale records instead of blocking forever.
- **Native lanes that actually work**: Qt MinGW (`build-qt-mingw`) and JUCE VS18 (`build-juce-demo`) demos build + launch; BobUI/BTK lanes remain blocked on their framework provider surfaces; BobGUI's own `meson.build` is corrupt in its repo. The web dashboard is the primary UI.
- **Runtime DLL discipline**: executables that link `fo_core` need the exact loader set beside them (`tests/copy_runtime_deps.cmake`); glob-copying prefix bins shadows good DLLs with debug variants.
- **Docs sell the scope cut**: README/ARCHITECTURE/ROADMAP all carry it — bobfilez is a file organizer; OmniShell material is parked aspiration. `docs/SUBMODULE_INVENTORY.md`: 12 of 173 `libs/` trees are build-referenced.

## Observations (2026-11-16, v6.5.7)
- **Newest binary is not automatically the best binary**: the dashboard picks the newest `fo_cli.exe`, so a fresh cloud-only manifest build can shadow the provider-complete lean build. `build_headless.bat` now supplements the manifest with `build-lean-deps` and sidecars its DLLs; verify `--modules`, not timestamps alone.
- **Provider completeness is a configure-time matrix**: cloud scanners come from the vcpkg manifest, while OCR/OpenCV/ONNX can come from the lean prefix in the same binary. Video/audio hash providers remain behind the opt-in `media-analysis` feature and must be badged until enabled.
- **No fake empty results**: `/api/audit` once returned `{history: []}` for an unimplemented ledger query. That is worse than a 501; unavailable backend queries must say so and point to the closest real surface (`/api/history`).
- **`main` and `upstream/master` are unrelated histories**: fetch showed 73/62 divergent commits and `git merge-base` fails. Treat upstream synchronization as an explicit import/replacement decision, not a routine merge.

## Observations (2026-11-14, v6.5.1)
- **One-page dashboard is the web truth**: `bobui_web/public/unified.html` (groups A–E by value, ⓘ tooltips, shared activity log) supersedes fragmented tab/page UIs; classic (`/`) and React (`/react/`) remain linked legacy lanes only.
- **Tray = server lifecycle**: `bobui_web/tray.py` (pystray, tkinter fallback) owns Open/Status/Restart/Logs/Quit; Quit posts `/api/quit`. Health route is `/api/health` (the bare `/health` polling bug shipped until v6.5.1).
- **CLI grammar is the bridge contract**: `clerk` accepts only `start|status` (start blocks forever — never launch from HTTP), `oracle` takes a bare query and prints plain text. Bridges must map to that grammar or answer honest 501 (vault, watch/status).
- **Binary/source drift is real**: checked-out `build-msvc` CLI reports v6.3.0 while source is v6.5.1 — always compare `fo_cli --help`/banner against `VERSION.md` before declaring a card broken.
- **Full `git status` is expensive** (160+ submodules): prefer `scripts/repo_status.py` / `--untracked-files=no` and path-scoped status.

## Observations (2026-04-01)
- **Transition to BTK Native Framework**: The native GUI direction is now being retargeted to `libs/btk` / `BTK_ROOT` rather than the older BobUI/Qt-fork path.
- **Node.js Web Dashboard**: The existing Node.js-based GUI remains `bobui_web`. It is still a valuable asset for cross-platform, remote management of the deduplication engine.
- **Submodule Scale**: The project has reached a massive scale (130+ submodules). Managing these with a dashboard (`SUBMODULES.md`) is essential for the health of the project.
- **Plugin Architecture Success**: The engine remains robust and modular despite the constant addition of new providers and UI layers.

## Preferences
- **Local-First AI**: Always prefer local models (ONNX Runtime) over cloud APIs to ensure user privacy and offline operation.
- **Strong Verification by Default**: "Safe" mode (Size + xxHash + BLAKE3) is preferred for most user workflows to avoid any data loss.
- **CLI-First**: All engine functionality must be accessible via `fo_cli` before being exposed to any GUI.
- **Conventional Commits**: Maintain high-quality commit messages to ensure the `CHANGELOG.md` remains readable.
- **Single Source of Versioning**: `VERSION.md` is the only truth. All other files (Java build scripts, WiX installers) must read from it.
