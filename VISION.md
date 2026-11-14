# bobfilez — Vision

> Single source of truth for *what* bobfilez is ultimately for. Build/version
> truth lives in `VERSION.md`; architecture observations in `MEMORY.md`;
> sequencing in `ROADMAP.md`; granular work in `TODO.md`.

## Ultimate goal

bobfilez is a **local-first, CLI-first file organization and deduplication
custodian**: a C++20 engine (`fo_core`) with a plugin registry of scanners,
hashers, metadata/OCR/classifier providers, exposed identically through
`fo_cli`, a C ABI (`fo_c_api`), native shells (Qt/BobUI/JUCE/BTK/BobGUI), and
one consolidated web dashboard. Every capability must be reachable from the
CLI first, previewable before it mutates anything (dry-run), journaled so it
can be undone, and honest when a provider or model is missing (explicit 501 /
error, never fake success).

## Foundational concepts

- **Registry plugin architecture** — scanners/hashers/providers register by
  name (`std`, `win32`, `s3`, `gdrive`, `azure`; `fast64`, `sha256`, `xxhash`,
  `blake3`); feature guards (`FO_HAVE_*`) degrade cleanly when a dependency
  is absent.
- **Preview → apply → undo** — organize/rename/delete-duplicates default to
  dry-run; mutating runs are journaled (`history` / `undo`).
- **One dashboard, ordered by value** — `bobui_web/public/unified.html` is
  the single web page: A Essentials (scan, duplicates, stats, search,
  organize) → B Verify & Manage → C Media & AI → D Organize & Protect →
  E Automate. No fragmented subpages; every card carries a descriptive
  label, guidance tooltip (ⓘ), and reports to one activity log.
- **System tray as the server's home** — `bobui_web/tray.py` lives in the
  Windows notification area: open dashboard, check status, restart, open
  logs, and *Quit server + tray* (via `POST /api/quit`). The dashboard
  header offers the same quit.
- **Local-first AI** — OCR/classify/Oracle run on-device (ONNX/local models);
  no file content leaves the machine by default.
- **130+ vendored submodules** under `libs/` are dependencies, never edited
  in place; the superproject pins published commits only.

## User-satisfaction design

1. Highest-value tasks (find, dedupe, tidy) are at the top and need the
   fewest fields; destructive actions are grouped, preview-first, and
   reversible where the backend supports it.
2. Every control explains itself (label + tooltip); every failure names the
   missing provider/tool instead of failing silently.
3. One start path per surface: `scripts/dev_all.bat` (build → tests → CLI
   smoke → dashboard wiring tests → server), `bobui_web/start-server.bat`,
   or `bobui_web/tray.bat`. Heavy processes run BelowNormal on shared hosts.

## Non-goals (for now)

- No cloud-required mode; cloud scanners are optional providers.
- No silent background mutation: watchers/listeners (`watch`, `clerk start`)
  are explicit terminal-run processes, never auto-started by the dashboard.
