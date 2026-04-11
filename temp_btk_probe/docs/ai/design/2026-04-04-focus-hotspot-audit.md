# BTK Focus Hotspot Audit

## Purpose
Record where the current single-focus assumptions are concentrated so the BTK multi-user model can be integrated with precision rather than broad rewrites.

## Primary Hotspot
### `src/gui/kernel/qapplication_cs.cpp`
This file contains the densest concentration of legacy focus ownership assumptions, including:
- `QApplicationPrivate::focus_widget`
- `QApplicationPrivate::hidden_focus_widget`
- `QApplicationPrivate::active_window`
- popup widget focus handling
- `QApplicationPrivate::setFocusWidget(...)`
- focus chain traversal via `focusNextPrevChild_helper(...)`
- active-window focus repair and restoration
- popup focus restoration when a popup closes

## Why this matters
This is effectively the center of truth for keyboard focus behavior in the widget/application layer. Any real BTK multi-focus work will eventually need to intersect here.

## Nearby Secondary Hotspots
### Accessibility
Focus state appears in accessible layers such as:
- `src/gui/accessible/qaccessible.cpp`
- `src/gui/accessible/qaccessibleobject.cpp`
- `src/gui/accessible/qaccessiblewidget.cpp`

These will need multiplayer-aware interpretations of "focused" and probably owner-specific focus reporting.

### Item views and editors
There are many direct assumptions that one focus widget is active at a time in:
- `src/gui/itemviews/qabstractitemview.cpp`
- related delegate/editor interactions

### Dialog and popup flow
Classic modal/focus behavior is strongly encoded in:
- `src/gui/dialogs/qdialog.cpp`
- popup open/close paths in `qapplication_cs.cpp`

## Suggested Integration Strategy
### Phase A - Sidecar policy
Keep legacy behavior intact, but consult BTK policy objects (`BtkFocusToken`, `BtkInputArbitrator`) before finalizing ownership decisions.

### Phase B - Owner-scoped modality
Teach modal and popup decisions to distinguish:
- same owner
- different owner
- shared modality
- owner-exclusive modality

#### Current progress
A first owner-aware integration now exists in two places:
- `QApplicationPrivate::setFocusWidget(...)`
- `QApplicationPrivate::tryModalHelper(...)` / modal blocking checks

This means BTK policy now influences both direct focus changes and the first layer of popup/modal gating.

### Phase C - Focus reporting
Expand accessibility and debug tooling to report:
- active focus tokens
- owning users/devices
- blocked owners/surfaces

#### Current progress
A first reporting surface now exists directly on `QApplication` through BTK diagnostic helpers that describe:
- widget owner/surface context
- focus routing decisions
- active popup/modal owners
- active focus token summaries

That surface is now wrapped by a dedicated QtGui-level diagnostics adapter:
- `<QtGui/BTKFocusDiagnostics>`

This is the first reusable substrate for future developer overlays.

### Phase D - Event routing
Route pointer, keyboard, shortcut, and touch behavior through BTK arbitration.

#### Current progress
Popup stack restoration is now partially owner-aware through `openPopup(...)` / `closePopup(...)`, reducing one more global-only focus assumption in the legacy GUI path.

## Strategic Finding
The focus stack is too central to rewrite blindly. The safest path is:
1. introduce BTK types
2. add narrow helper integration points
3. validate behavior incrementally
4. expand from popup/modal handling outward
