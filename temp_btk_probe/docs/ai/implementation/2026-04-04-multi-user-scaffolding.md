# BTK Multi-User Scaffolding Session Notes

## Implemented
### Core public types
- `src/core/kernel/btkinputowner.h`
- `src/core/kernel/btkinputowner.cpp`
- `src/core/kernel/btkfocustoken.h`
- `src/core/kernel/btkfocustoken.cpp`

### GUI public types
- `src/gui/kernel/btkinputarbitrator.h`
- `src/gui/kernel/btkinputarbitrator.cpp`

### Build integration
- updated `src/core/kernel/kernel.cmake`
- updated `src/gui/kernel/kernel.cmake`

## Public API Added
### QtCore include surface
- `<QtCore/BTKInputOwner>`
- `<QtCore/BTKFocusToken>`

### QtGui include surface
- `<QtGui/BTKInputArbitrator>`

### Narrow real integration API
Added to `QApplication`:
- `setBtkFocusTokens(...)`
- `btkFocusTokens()`
- `setBtkOwnerContext(...)`
- `btkOwnerId(...)`
- `btkSurfaceId(...)`
- `btkActivePopupOwnerId()`
- `btkActiveModalOwnerId()`
- `btkDescribeWidgetContext(...)`
- `btkDescribeFocusDecision(...)`
- `btkFocusDiagnostics()`
- `btkWouldBlockFocusChange(...)`

### Popup/modal integration progress
The BTK owner model now also affects:
- `QApplicationPrivate::tryModalHelper(...)`
- `QApplicationPrivate::isWindowBlocked(...)`
- `QApplicationPrivate::openPopup(...)`
- `QApplicationPrivate::closePopup(...)`

Current behavior:
- popups can permit same-owner interactions without being treated as globally exclusive by default
- modal widget windows do not automatically block other windows owned by the same BTK owner
- popup restoration prefers the most recent popup belonging to the same BTK owner
- opening a first popup no longer forces focus-out on an unrelated owner by default

## Intent
This work does not yet alter the existing focus engine. It establishes BTK-native concepts for:
- user/session ownership
- device grouping
- owner-scoped focus
- modality policy
- arbitration decisions

## Why this approach is safe
The current BTK/CopperSpice focus logic is tightly interwoven with widget, window, popup, and modality internals. Replacing it directly would be risky. Introducing explicit BTK types first allows future integration in measured steps.

## Observed hotspot
`src/gui/kernel/qapplication_cs.cpp` contains the deepest concentration of single-focus assumptions, including:
- `QApplicationPrivate::focus_widget`
- `QApplicationPrivate::active_window`
- popup focus restoration
- focus chain traversal
- active window focus transfer

That file is the main future integration hotspot for owner-scoped multi-focus behavior.

## Integration progress in hotspot
This session now threads BTK policy into three real paths in that file:
- direct focus changes via `setFocusWidget(...)`
- popup/modal admission checks via `tryModalHelper(...)`
- modal blocking checks via `isWindowBlocked(...)`

## Overlay groundwork added
A first reusable diagnostics adapter now exists in QtGui:
- `src/gui/kernel/btkfocusdiagnostics.h`
- `src/gui/kernel/btkfocusdiagnostics.cpp`

This adapter packages current BTK owner/focus state into a structured snapshot suitable for future overlays and debugging tools.

## First visual overlay scaffold added
A first minimal visual overlay widget now exists:
- `src/gui/widgets/btkfocusoverlay.h`
- `src/gui/widgets/btkfocusoverlay.cpp`

This widget now goes beyond a raw text dump and provides a richer HUD pass with:
- owner summary chips
- focus widget and path sections
- focus owner / focus surface summaries
- focus-popup relationship summaries
- popup-stack summaries
- relationship digest summaries (focus-vs-popup, popup-vs-modal, focus-owner popup-stack presence)
- owner-group summaries derived from active tokens, now count-prioritized for more stable inspection
- blocker-group summaries derived from blocked routes, now count-prioritized for more stable inspection
- token summaries
- blocked-route summaries
- optional target-widget context, popup-relationship, and decision reporting
- panel-based visibility control for summary/focus/owner/popup/relationship/token/target/blocked/raw sections
- preset-oriented modes for compact, owner-centric, analysis, and full views
- blocked-only filtering and preset cycling support

The popup stack is now part of the diagnostics substrate as well, including filtered focus-owner popup-stack views, which helps connect mixed-owner popup behavior to the overlay/tooling story.

The popup model is also now threaded further into runtime behavior:
- popup-aware wheel-event gating consults BTK popup allowance so same-owner widgets are not automatically treated like unrelated popup outsiders
- popup-aware context-menu routing now uses the same BTK popup allowance logic
- popup-aware help/tooltip routing now uses the same BTK popup allowance logic
- popup-aware drag/drop routing now uses the same BTK popup allowance logic
- popup-aware tablet routing now uses the same BTK popup allowance logic
- popup-aware touch routing now uses the same BTK popup allowance logic
- popup-aware gesture routing now uses the same BTK popup allowance logic
- popup-aware mouse enter/leave receiver updates now avoid blindly tracking unrelated-owner widgets while a popup is active
- popup-aware synthetic enter/leave generation now avoids blindly targeting unrelated-owner widgets while a popup is active

## Recommended next step
Continue improving mixed-owner concurrent popup behavior and deepen the overlay from a lightweight HUD into a more inspectable multi-panel developer tool, especially around popup-stack ownership relationships and blocked-route inspection.
