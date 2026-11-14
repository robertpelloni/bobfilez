# scripts/ — what to run

Recommended entry points (everything else is a special case):

| Script | Purpose |
|---|---|
| `build_headless.bat` | **Default build.** Verified MSVC+Ninja fallback, GUI/Omni off, tests+bench on → `build-msvc/` |
| `dev_all.bat` | One-shot: headless build → `fo_tests.exe` → CLI smoke test → dashboard server |
| `repo_status.py` | Tracked-only `git status` (avoids Windows long-path warning noise) |
| `download_models.py` | Fetch optional AI models (classify/OCR) |
| `generate_dashboard.py` | Regenerate `docs/SUBMODULES.md` dashboard |

Specialized GUI/demo builds: `build_bobui_gui.bat`, `build_bobgui_gui.bat`, `build_qt_gui.bat`, `build_juce_gui.bat`, `build_btk_gui.bat` (+ `*_inplace.bat` variants that compile the framework itself under `libs/`).
Packaging: `package.bat`, `package_msi.bat`, `package_appimage.sh`, `package_macos.sh`, `deploy_staging.sh`.
Fuzzing: `build_fuzzer.bat` / `build_fuzzer.sh` (canonical — see `archive/` note).

## archive/

Nondestructive attic for superseded entry points (kept in git history via `git mv`):

- `build_limited.bat` (ex-root) — hardcoded VS2019 path, built stale `build/` dir, ran one test. Superseded by `build_headless.bat`.
- `build_fuzz.bat` (ex-root) — old fuzz flags (`FO_BUILD_FUZZ_TESTS`) into stale `build/` dir. Superseded by `build_fuzzer.bat` (`FO_BUILD_FUZZERS` → `build_fuzz/`).
- `root-helpers/` — 20 one-shot `patch_*.py` / `fix_*.py` scripts formerly cluttering the repo root (archived 2026-11-14, contents preserved verbatim). They patched one historical build break each; do not re-run against current sources.

Do not delete: they document the migration path. Do not revive without updating paths/flags.

Portable MinGW lane (no-admin hosts): toolchain lives at C:/tools (cmake 4.4.5 + ninja 1.13.2 + winlibs gcc 16.2 UCRT, all portable zips, installed 2026-11-03). uild_headless_mingw.bat configures uild-mingw/ with GUI/Omni/Bench off; OpenSSL+BLAKE3 degrade to QUIET and the vault compiles to honest stubs when OpenSSL is absent. Needs no elevation; MSVC remains canonical where present.
