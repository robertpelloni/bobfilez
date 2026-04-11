# BTK Developer Overlay Groundwork

## Purpose
Prepare a bridge between BTK's new owner-aware focus runtime and future developer tooling overlays.

## Why this step matters
The BTK runtime now has:
- owner context on widgets
- focus-token arbitration
- popup/modal owner-aware gating
- diagnostics on `QApplication`

The next natural step is to make that information easier to consume from future overlays, inspectors, and debugging tools.

## Groundwork added
### Public QtGui surface
- `<QtGui/BTKFocusDiagnostics>`
- `<QtGui/BTKFocusOverlay>`

### Types
- `BtkFocusDiagnosticsSnapshot`
- `BtkFocusDiagnostics`
- `BtkFocusOverlay`

### Progress level
- `BTKFocusDiagnostics` provides the reusable structured reporting substrate
- `BTKFocusOverlay` now provides a visual HUD pass with owner grouping, blocker grouping, popup-stack summaries, blocked-route summaries, panel-based visibility control, preset-oriented panel modes, and blocked-only filtering over that substrate

## Current responsibilities
### `BtkFocusDiagnosticsSnapshot`
Carries structured snapshot data for:
- active popup owner
- active modal owner
- focus widget description
- focus widget path
- focus popup relationship
- focus owner id
- focus surface id
- popup-stack summaries
- owner-group summaries
- blocker-group summaries
- token summaries
- blocked-route summaries
- raw diagnostic lines

The popup-stack summaries now provide a clearer bridge between mixed-owner popup behavior and overlay inspection surfaces, including filtered focus-owner popup-stack views.

### `BtkFocusDiagnostics`
Provides static helpers to:
- capture a snapshot from the current `QApplication`
- format current state as text
- describe a widget tree path for debugging

## Why this is overlay groundwork rather than the final overlay
This work now includes the first minimal visual overlay scaffold (`BtkFocusOverlay`), but the broader point still stands: BTK is separating diagnostic data collection from overlay rendering so future tooling does not need to embed ad-hoc logic directly inside `QApplication` internals.

## Intended future consumers
- BTK debug HUD / overlay widgets
- inspection panels
- accessibility/focus debugging tools
- multiplayer collaboration diagnostics
- automated tests and screenshots

## Strategic finding
It is better to create a clean diagnostic adapter layer first than to jump straight to a visual overlay implementation. That keeps the runtime logic reusable and easier to test.
