# BobGUI Status and Field-Role Clarity — 2026-04-06

## Summary

After multiple rounds of BobGUI output and presentation refinement, one more subtle consistency issue remained:

- the output panel had become clearer
- but the **status label** still used more generic phrasing than ideal in a few important states
- and the ignore-reset flow could do more to explain what each field is actually for

This session tightens that final layer of control-surface clarity.

## What Changed

Updated:
- `frontends/bobgui_app/main.c`

### 1. Ready-state messaging is more explicit
BobGUI now uses a small helper to present clearer ready states depending on backend mode, such as:
- direct-preferred mode through `fo_c_api`
- fallback-active mode through `fo_cli`
- waiting-for-backend mode when neither path is available

### 2. Running-state messaging now reflects the active operation
Instead of a generic “Running request...” label, BobGUI now uses the human-facing operation display name so the status label tracks the action more honestly while work is in flight.

### 3. Ignore-reset guidance now explains field roles more clearly
The output shown after resetting ignore fields now explicitly explains:
- the role of the **Ignore Pattern** field
- the role of the **Reason** field
- which actions to use next

## Why this matters

These are small refinements, but they improve the feeling that the BobGUI lane is an intentional operational panel rather than a generic action launcher.

In a UI with many workflows, users benefit when:
- the status label reflects what is happening right now
- the ready state explains which backend mode is active
- field-reset behavior reinforces how the panel is meant to be used

## Validation

Repo-wide safety validation was re-run:
- `scripts/build_headless.bat` ✅
- `ctest --test-dir build-msvc --output-on-failure` ✅

Validation surface remains:
- **73 / 73 tests passed**

## Recommended Next Step

If BobGUI remains the current focus, the next likely refinement is another small consistency pass around status/output alignment and field-role explanation, rather than more backend width.
