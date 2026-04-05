# BTK Upstream Refresh — 2026-04-05

## Summary
This session refreshed `libs/btk` to the latest upstream `origin/master`, rebased bobfilez's required MSVC fixes on top of that newer BTK tip, rebuilt BTK successfully, and re-ran the bobfilez BTK-backed GUI probe.

The result is important because it confirms the current blocker is stable across the newer upstream BTK state:
- **BTK itself still builds successfully on this host once the two local MSVC fixes are rebased on top of the new upstream tip**
- **bobfilez still fails at the same honest downstream boundary: missing `Declarative`**

So the upstream refresh did not invalidate the strategic conclusion already documented. It strengthened it by showing the same boundary persists even after moving forward to the latest upstream BTK master.

## Updated BTK state
### Upstream master advanced to
- `18e3770af` — `build: validate BTK focus reason package smoke`

### Rebased local BTK fixes now sit on top as
- `0546ebd70` — `fix: restore msvc build for focus and input routing`
- `4f5a809e4` — `fix: restore qapplication property lookups for msvc`

### Reproducible pushed BTK branch
- `origin/pi/msvc-focus-fixes-20260405`

## Validation performed
### 1. BTK in-place build
Ran:
- `scripts/build_btk_inplace.bat`

Result:
- success
- BTK still configures and builds successfully on this host with the rebased fixes

### 2. bobfilez BTK-backed GUI probe
Ran:
- `scripts/build_btk_gui.bat`

Result:
- unchanged failure boundary
- configure still stops at:
  - `Could not resolve a BTK/CopperSpice target for component 'Declarative'`

## Interpretation
This upstream refresh matters because it removes another potential source of ambiguity.

It shows that the current strategic blocker is **not** just an artifact of an older upstream BTK snapshot.
Even after updating BTK to the latest upstream master and rebuilding successfully, the downstream consumer boundary remains the same:
- no exported `Declarative`
- therefore no honest path for bobfilez's active QQml-based shell bootstrap to configure against current BTK

## Practical conclusion
The previously documented strategy remains correct:
- keep bobfilez on the Option C path
- continue reducing provider coupling in the active shell bootstrap
- treat BTK modernization as a separate upstream R&D effort rather than the immediate app runtime path
