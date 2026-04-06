# Frontend History + Ignore Parity Expansion — 2026-04-06

## Summary

After lint parity was pushed broadly across the frontend matrix, the next obvious practical workflow gap was not another scanner-style operation, but a pair of lightweight maintenance workflows already exposed by the CLI/web backend surface:

- **history**
- **ignore rule management**

This session focused on the lightest lanes where those workflows could be surfaced honestly without forcing premature ABI expansion:

- **React/Express SPA**
- **BobGUI**

## What Changed

### React web UI
Updated:
- `bobui_web/public/react/app.js`

Added:
- history result normalization
- ignore-rule result normalization
- `fetchGetJson(...)` helper for GET-backed endpoints
- a new **History** tab
- a new **Ignore Rules** tab
- ignore-rule add/remove actions wired to:
  - `POST /api/ignore/add`
  - `POST /api/ignore/remove`
- refresh flows for:
  - `GET /api/history`
  - `GET /api/ignore`

This makes the React lane a more honest operational frontend instead of remaining limited to only scan/duplicate/stat/hash/metadata-style workflows.

### BobGUI
Updated:
- `frontends/bobgui_app/main.c`

Added:
- `operation_requires_path(...)`
- path-free CLI request handling for operations such as history and ignore-rule listing
- a **History** button
- an **Ignore Rules** button
- cleaner status labeling for path-free requests

Important nuance:
- these operations are **not** currently provided by the direct `fo_c_api`
- BobGUI therefore exercises its **per-operation fallback model** here exactly as intended
- direct mode remains preferred where available, while CLI-only operations still remain accessible from the same UI shell

## Why this matters

This session strengthened two important product truths:

1. The frontend matrix should include not only analysis workflows, but also lightweight maintenance / operational workflows.
2. BobGUI’s dual-backend model is proving useful in practice because it can expose CLI-real features before a direct C ABI seam exists for them.

## Validation

Validated on this host where practical:
- `node --check bobui_web/server.js` ✅
- `node --check bobui_web/public/react/app.js` ✅
- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅

Validation surface remains:
- **71 / 71 tests passed**

### Host caveat
End-to-end BobGUI application validation remains constrained on this machine because the host still lacks the easy Meson/pkg-config/ninja path for that lane. The implementation is therefore source-validated plus backend-validated, not full host-run validated.

## Recommended Next Step

Keep choosing parity targets that satisfy all three conditions:
1. the backend seam already exists and is real
2. the UX can be added incrementally without framework-specific overengineering
3. the validation story remains honest on this host
