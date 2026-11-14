# Memory: Observations & Design Preferences

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
