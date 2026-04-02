# Memory: Observations & Design Preferences

## Observations (2026-04-01)
- **Transition back to Native (BobUI)**: We are moving the primary GUI back to a custom-forked Qt library (`libs/bobui`). This allows for tighter integration with the C++ engine (`fo_core`), lower memory overhead, and true native performance.
- **Node.js BobUI as `bobui_web`**: The existing Node.js-based GUI is now `bobui_web`. It remains a valuable asset for cross-platform, remote management of the deduplication engine.
- **Submodule Scale**: The project has reached a massive scale (130+ submodules). Managing these with a dashboard (`SUBMODULES.md`) is essential for the health of the project.
- **Plugin Architecture Success**: The engine remains robust and modular despite the constant addition of new providers and UI layers.

## Preferences
- **Local-First AI**: Always prefer local models (ONNX Runtime) over cloud APIs to ensure user privacy and offline operation.
- **Strong Verification by Default**: "Safe" mode (Size + xxHash + BLAKE3) is preferred for most user workflows to avoid any data loss.
- **CLI-First**: All engine functionality must be accessible via `fo_cli` before being exposed to any GUI.
- **Conventional Commits**: Maintain high-quality commit messages to ensure the `CHANGELOG.md` remains readable.
- **Single Source of Versioning**: `VERSION.md` is the only truth. All other files (Java build scripts, WiX installers) must read from it.
