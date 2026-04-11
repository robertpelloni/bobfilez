# BTK Visual Overlay Scaffold

## Purpose
Introduce the first real visual developer-facing BTK overlay surface built on top of the new owner-aware diagnostics adapter.

## Added public surface
- `<QtGui/BTKFocusOverlay>`

## Added type
- `BtkFocusOverlay`

## Role
`BtkFocusOverlay` is a lightweight diagnostic widget that renders:
- current BTK focus diagnostics
- active popup/modal owner summaries
- token summaries
- optional target-widget path/context information

## Why this matters
This is the first step from:
- raw runtime concepts
- to diagnostics
- to a visible on-screen developer tool

It is intentionally modest. The goal is not final UX polish; the goal is to establish a real rendering surface that future BTK tooling can evolve from.

## Current behavior
- transparent-to-mouse overlay widget
- auto-refreshing overlay surface
- panel-based visibility control (`Summary`, `Focus`, `Owner`, `Popup`, `Relationship`, `Token`, `Target`, `Blocked`, `Raw`)
- preset-oriented panel modes (`Compact`, `OwnerCentric`, `Analysis`, `Full`)
- preset cycling support and blocked-only filtering mode
- summary chips for popup owner, modal owner, refresh cadence, focus owner, focus surface, popup count, blocker count, blocked-route count, and preset/mode state
- focus widget and path sections
- focus-popup relationship section
- owner-group summary section with count-prioritized ordering
- blocker-group summary section with count-prioritized ordering
- popup-stack section including focus-owner popup-stack context
- relationship digest section for focus-vs-popup, popup-vs-modal, and focus-owner popup-stack summaries
- token summary section with color-coded emphasis
- blocked/exclusive route section with more precise summaries
- optional target widget section with decision summary and popup-relationship summary
- built directly on `BtkFocusDiagnostics`

## Current assessment
The overlay has now moved beyond a plain text dump into a richer HUD pass with owner grouping, blocker grouping, relationship digests, popup-stack inspection, blocked-route visualization, and stronger panel behavior. It is still intentionally lightweight, but it already demonstrates the shape of a future BTK developer experience: runtime state transformed into a readable, always-on-top tool surface.

The surrounding runtime also continues to become less globally single-owner:
- popup-aware wheel-event gating now consults BTK owner-aware popup allowance rather than only comparing the active popup window directly
- popup-aware context-menu routing now follows the same BTK popup allowance model
- popup-aware help/tooltip routing now follows the same BTK popup allowance model
- popup-aware drag/drop routing now follows the same BTK popup allowance model
- popup-aware tablet routing now follows the same BTK popup allowance model
- popup-aware touch routing now follows the same BTK popup allowance model
- popup-aware gesture routing now follows the same BTK popup allowance model
- popup-aware mouse enter/leave receiver tracking now avoids blindly promoting unrelated-owner widgets while a popup is active
- popup-aware synthetic enter/leave generation now follows the same ownership discipline

## Future directions
- richer styling and layout refinement
- interactive panel switching / controls
- click-driven or shortcut-driven panel interaction
- token tables / deeper color coding
- per-owner cursor/focus panels
- modal/popup block reason visualization
- owner-scoped context-menu and additional popup-sensitive event routing refinements
- more explicit target-to-popup relationship inspection
- integration with immediate-mode debug surfaces
