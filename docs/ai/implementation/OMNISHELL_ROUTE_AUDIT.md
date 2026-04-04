# OmniShell Route Audit — 2026-04-03

## Purpose
This note captures the current relationship between:
- `shell.activePanel` routes in `gui/omni/assets/main.qml`
- visible launch surfaces such as `StartMenu.qml`, `Taskbar.qml`, and explorer/sidebar entry points
- stabilization work needed before adding more flagship Omni subsystems

## Route Inventory
The current `main.qml` host exposes these panel routes:

- `explorer`
- `search`
- `rename`
- `convert`
- `hex`
- `image`
- `md`
- `watcher`
- `fileops`
- `visual_dedup`
- `topology`
- `pruner`
- `hierarchy`
- `achievements`
- `cloud`
- `network`
- `vault`
- `forensic`
- `assets`
- `omnivision`
- `omniaudio`
- `omnigraph`
- `omniflow`
- `timemachine`
- `swarm`
- `recovery`
- `notary`
- `photos`
- `develop`
- `oracle`
- `omnimount`
- `omnisec`
- `omniclerk`
- `omnigit`
- `omniverse`
- `omnicrypt`
- `omnicluster`
- `terminal`
- `omnishare`

## Launch Surface Coverage

### Taskbar
Direct taskbar launchers now cover:
- `search`
- `oracle`
- `explorer`
- `omnigit`
- `omnivision`
- `omniaudio`
- `terminal`
- `omnishare`

### Start Menu
Pinned start-menu entries currently cover:
- `explorer`
- `search`
- `terminal`
- `omnishare`
- `omnicluster`
- `omnicrypt`
- `omniverse`
- `omnigit`
- `omniclerk`
- `omnisec`
- `omniaudio`
- `omnivision`
- `omnimount`
- `oracle`
- `timemachine`
- `omniflow`
- `omnigraph`
- `notary`
- `recovery`
- `swarm`
- `assets`
- `photos`
- `topology`
- `vault`

### Explorer / Sidebar / In-App Navigation
Observed direct in-app route jumps include:
- `cloud`
- `network`
- `omnigit`
- `hierarchy`

## Gaps Still Present
These routes exist in `main.qml` but are not yet clearly exposed from a top-level launch surface:
- `rename`
- `convert`
- `hex`
- `image`
- `md`
- `watcher`
- `fileops`
- `visual_dedup`
- `pruner`
- `achievements`
- `forensic`
- `develop`

This is not necessarily wrong, because some of these are better launched contextually from Explorer workflows rather than pinned globally. But the distinction should be deliberate.

## Findings
1. **The shell host is broader than the visible shell surface.**
   The app can open more panels than the taskbar/start menu currently advertise.

2. **Newer flagship systems were drifting out of sight.**
   `terminal`, `omnishare`, and `omnicluster` existed in the shell host but needed more obvious launcher affordances.

3. **Contextual vs global launch needs a policy.**
   Some tools should likely remain context-driven (`hex`, `image`, `md`, `rename`, `convert`), while flagship subsystems should be discoverable from Start/Taskbar.

4. **Stabilization is now more valuable than scope expansion.**
   The highest ROI work is ensuring routes are reachable, builds boot, and panels connect to real backends.

## Recommended Next Actions
1. Keep **flagship Omni systems** on visible launch surfaces.
2. Keep **file-context tools** available via Explorer context menus and file-type routing.
3. Add a dedicated "All apps" or "Tools" surface if route count keeps growing.
4. Validate that every top-level launcher opens the intended `activePanel` and no stale IDs remain.
