# BTK Multi-User Scaffolding Validation

## Scope
This validation covers the newly introduced BTK ownership/focus/arbitration scaffolding.

## Added surfaces to validate later in a compiler-ready environment
### QtCore
- `<QtCore/BTKInputOwner>`
- `<QtCore/BTKFocusToken>`

### QtGui
- `<QtGui/BTKInputArbitrator>`
- `<QtGui/BTKFocusDiagnostics>`
- `<QtGui/BTKFocusOverlay>`

### QApplication integration
- `QApplication::setBtkFocusTokens(...)`
- `QApplication::btkFocusTokens()`
- `QApplication::setBtkOwnerContext(...)`
- `QApplication::btkActivePopupOwnerId()`
- `QApplication::btkActiveModalOwnerId()`
- `QApplication::btkDescribeWidgetContext(...)`
- `QApplication::btkDescribeFocusDecision(...)`
- `QApplication::btkPopupStackDiagnostics()`
- `QApplication::btkFocusDiagnostics()`
- `QApplication::btkWouldBlockFocusChange(...)`
- popup/modal owner-aware gating through `tryModalHelper(...)` / `isWindowBlocked(...)`

## Recommended validation cases
1. Create two `BtkInputOwner` instances representing two local users.
2. Create multiple `BtkFocusToken` instances with different scopes and modality policies.
3. Verify `BtkInputArbitrator::route(...)` behavior for:
   - shared modality
   - owner-exclusive modality same owner
   - owner-exclusive modality different owner
   - transfer within same owner to a different surface
4. Verify public header generation exposes the new BTK include wrappers correctly.
5. Verify same-owner windows are not spuriously blocked by BTK-aware modal logic.
6. Verify different-owner popup/modal requests are accepted or rejected according to installed focus tokens.
7. Verify popup close restores same-owner popup focus before falling back to the global popup stack.
8. Verify first-popup opening does not force focus-out on an unrelated owner by default.
9. Verify popup-aware wheel event gating allows same-owner routing while still blocking unrelated-owner routing when appropriate.
10. Verify popup-aware context-menu routing allows same-owner routing while still blocking unrelated-owner routing when appropriate.
11. Verify popup-aware help/tooltip routing allows same-owner routing while still blocking unrelated-owner routing when appropriate.
12. Verify popup-aware drag/drop routing allows same-owner routing while still blocking unrelated-owner routing when appropriate.
13. Verify popup-aware tablet routing allows same-owner routing while still blocking unrelated-owner routing when appropriate.
14. Verify popup-aware touch routing allows same-owner routing while still blocking unrelated-owner routing when appropriate.
15. Verify popup-aware gesture routing allows same-owner routing while still blocking unrelated-owner routing when appropriate.
16. Verify popup-aware mouse enter/leave receiver updates do not blindly promote unrelated-owner widgets while a popup is active.
17. Verify popup-aware synthetic enter/leave generation does not blindly target unrelated-owner widgets while a popup is active.

## Smoke examples prepared
- `docs/ai/testing/btk-multi-user-smoke-example.cpp`
- `docs/ai/testing/btk-focus-routing-smoke-example.cpp`
- `docs/ai/testing/btk-modal-owner-smoke-example.cpp`
- `docs/ai/testing/btk-focus-diagnostics-smoke-example.cpp`
- `docs/ai/testing/btk-popup-restoration-smoke-example.cpp`
- `docs/ai/testing/btk-popup-stack-diagnostics-smoke-example.cpp`
- `docs/ai/testing/btk-focus-overlay-groundwork-smoke-example.cpp`
- `docs/ai/testing/btk-focus-overlay-visual-smoke-example.cpp`

The overlay smoke path now exercises the richer HUD scaffold, including popup-stack summaries, focus/target popup-relationship summaries, relationship digest summaries, count-prioritized owner/blocker summaries, preset-oriented panel modes, blocked-only filtering, filtered relationship/popup-panel visibility, and panel-based blocked-route visibility, rather than only a plain text rendering shell.

## Known limitation
No full compiler validation was possible in this environment because CMake could not complete configuration without a usable C/C++ compiler toolchain.
