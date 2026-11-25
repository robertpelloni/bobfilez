# Session Handoff - v6.5.116 (Index Follows Moves)

## What landed (2026-11-25, continuation)
- **Stale index fixed**: organize/rename completions
  and the undo handler re-point file-repository rows
  (`update_path`), so `similar` never answers with
  pre-move or post-undo phantom paths. Test pins the
  hash → organize → similar → undo → similar cycle.
- Version **6.5.116**: lean/MSVC **125 + 1 skip**,
  wiring **32/32**, JUCE **119 + 4 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`; Gemini recovery/index items answered).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- History purge: CLOSED — never purge (owner, 2026-11-25).

---
# Session Handoff - v6.5.115 (Recovery Identity)

## What landed (2026-11-25, continuation)
- **Recovery verifies identity** (Gemini drift class):
  intents record source size+hash at plan time;
  reconcile verifies both before completing a crashed
  move — drifted destinations stay `pending` (history
  JSON now shows `status`) and undo never touches them.
  Replay idempotence (organize→undo cycles + audit
  chain) pinned.
- Version **6.5.115**: lean/MSVC **124 + 1 skip**,
  wiring **32/32**, JUCE **119 + 3 skips**. (One MSVC
  run skip-cascaded when `build-msvc/cli/fo_cli.exe`
  vanished to an external cleanup; rebuilt green.)

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`; Gemini class above answered in-thread).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- History purge: CLOSED — never purge (owner, 2026-11-25).

---
# Session Handoff - v6.5.114 (Illegal/ADS Names)

## What landed (2026-11-25, continuation)
- **Illegal/ADS characters refused**: raw-string
  segment guard now also rejects `<>:"|?*` and control
  characters (drive `C:` excepted) with a clear unsafe
  name error. Deep-destination and ADS behaviors were
  verified already fail-safe and are now pinned by tests.
  Gemini acknowledged the junction fix and named the
  path-length/case/name class; case collisions and
  long paths were already pinned in earlier versions.
- Version **6.5.114**: lean/MSVC **122 + 1 skip**,
  wiring **32/32**, JUCE **117 + 3 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`; Gemini replied, class answered above).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- History purge: CLOSED — never purge (owner, 2026-11-25).

---
# Session Handoff - v6.5.113 (Segment Guard)

## What landed (2026-11-25, continuation)
- **Unsafe-name guard covers every segment**: raw
  rule-output string check (path normalization hid
  `sub.`-style segments from the old final-segment
  check) in organize + rename; path check as backstop.
- Version **6.5.113**: lean/MSVC **120 + 1 skip**,
  wiring **32/32**, JUCE **115 + 3 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`, Gemini `49b11b21`).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- History purge: CLOSED — never purge (owner, 2026-11-25).

---
# Session Handoff - v6.5.112 (Delete Batching)

## What landed (2026-11-25, continuation)
- **Delete-duplicates batched**: per-group Delete
  intent rows commit before any unlink; completions
  and audit commit after. Reconcile gained the Delete
  branch (source present → drop intent; gone →
  complete). Crash-window test kills after intent and
  after a removal. Stale-authorization re-hash intact.
- Version **6.5.112**: lean/MSVC **119 + 1 skip**,
  wiring **32/32**, JUCE **114 + 3 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`, Gemini `49b11b21`).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- History purge: CLOSED — never purge (owner, 2026-11-25).

---
# Session Handoff - v6.5.111 (Bulk Rename Chunked)

## What landed (2026-11-25, continuation)
- **Rename chunked** identically to organize (intent
  txn per 128, completion txn after; completion failure
  reconciles instead of rolling back). 2,000-file
  rename ~6.3s (host floor). Dead residue helper removed.
- Version **6.5.111**: lean/MSVC **118 + 1 skip**,
  wiring **32/32**, JUCE **113 + 3 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`, Gemini `49b11b21`).
- delete-duplicates still journals per file (batching
  needs a Delete-aware reconcile branch; designed, not
  yet built).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- History purge: CLOSED — never purge (owner, 2026-11-25).

---
# Session Handoff - v6.5.110 (Bulk Undo Batched)

## What landed (2026-11-25, continuation)
- **Bulk undo batched**: status transitions are one
  transaction per batch (was two fsyncs per record);
  2,000-file undo ~27s → ~13s, remainder is identity
  verification by design. Crash semantics unchanged
  (reconcile resolves `undoing` rows).
- Version **6.5.110**: lean/MSVC **118 + 1 skip**,
  wiring **32/32**, JUCE **113 + 3 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`, Gemini `49b11b21`).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- History purge: CLOSED — never purge (owner, 2026-11-25).

---
# Session Handoff - v6.5.109 (Bulk Organize Halved)

## What landed (2026-11-25, continuation)
- **Bulk organize fsync cost**: 128-file chunks (one
  intent txn before the chunk's moves, one completion
  txn after). 2,000-file organize 27.3s → ~14s (host
  raw-rename floor is 6.2s; rest is host I/O latency).
  Dropped MOVEFILE_WRITE_THROUGH from rename primitive.
- **History purge: CLOSED — never purge** (owner). No
  credential material exists in history; rewriting it
  would only destroy provenance.
- Version **6.5.109**: lean/MSVC **118 + 1 skip**,
  wiring **32/32**, JUCE **113 + 3 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`, Gemini `49b11b21`).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).

---
# Session Handoff - v6.5.108 (Build Markers Untracked)

## What landed (2026-11-25, continuation)
- **Generated-artifact exclusion**: five component
  `.build_success` stamps untracked; `.gitignore` rule
  generalized to any depth. No code change; suites
  unchanged from v6.5.107 (lean/MSVC 118+1, JUCE 113+3).
- Version **6.5.108**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`, Gemini `49b11b21`).
- History purge: **CLOSED — never purge.** The scan in
  `docs/SECRETS_HISTORY_SCAN.md` found zero credential
  material in any historical `.db` blob (all credential-
  shaped tables empty; zero credential-pattern hits).
  What remains is the owner's own session/transcript and
  file-path history. Rewriting it would destroy provenance
  for no security gain. Owner confirmed 2026-11-25.
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).

---
# Session Handoff - v6.5.107 (Cross-Volume + Recovery-in-Recovery)

## What landed (2026-11-25, continuation)
- **Cross-volume moves**: `move_file_no_replace` copy
  fallback (temp on dest volume, size-verify, no-replace
  rename, source removal) in organize + undo.
- **Recovery-in-recovery**: reconcile resolves per row
  (no sweep transaction), `reconcile.after-row`
  checkpoint; kill-mid-sweep + idempotency test.
- **Durability**: journal DB `synchronous=FULL` (intent
  must survive power loss, not just process crash).
- **Watch heartbeat**: temp+rename writes; empty
  heartbeat reads as not-running (was empty output).
- Version **6.5.107**: lean/MSVC **118 + 1 skip**,
  wiring **32/32**, JUCE **113 + 3 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`, Gemini `49b11b21`).
- History purge (owner; evidence in
  `docs/SECRETS_HISTORY_SCAN.md`).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).

---
# Session Handoff - v6.5.106 (No-Replace Rename)

## What landed (2026-11-25, continuation)
- **No-replace rename everywhere** (ChatGPT's
  stale-authorization class): `rename_no_replace`
  (MoveFileEx no-replace / link+unlink) backs vault
  encrypt+decrypt completion, organize, rename-batch,
  and undo move-back, closing every check-then-rename
  window. Copy engine overwrites only under explicit
  CollisionPolicy — reviewed, left as designed.
- Version **6.5.106**: lean/MSVC **115 + 1 skip**,
  wiring **32/32**, JUCE **110 + 3 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`, Gemini `49b11b21`).
- History purge (owner; evidence in
  `docs/SECRETS_HISTORY_SCAN.md`).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).

---
# Session Handoff - v6.5.105 (Keeper Alias Rule)

## What landed (2026-11-25, continuation)
- **Keeper alias rule**: delete-duplicates keeper sort
  prefers real files over links before any --keep
  strategy (keeping a link and deleting its target
  dangled the path and lost the bytes). Test is
  skip-guarded on hosts without symlink privilege.
- Version **6.5.105**: lean/MSVC **114 + 1 skip**,
  wiring **32/32**, JUCE **109 + 3 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`, Gemini `49b11b21`).
- History purge (owner; evidence in
  `docs/SECRETS_HISTORY_SCAN.md`).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).

---
# Session Handoff - v6.5.104 (Junction Escape)

## What landed (2026-11-25, continuation)
- **Junction escape fixed**: MSVC `is_symlink` misses
  NTFS junctions; organize moved an outside file through
  an in-root junction. Shared `is_link_like` (symlink or
  reparse-point) now guards every directory descent in
  the tree. CLI test pins it end to end.
- Version **6.5.104**: lean **114/114**, MSVC **114/114**,
  wiring **32/32**, JUCE **109 + 2 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`, Gemini `49b11b21`).
- History purge (owner; evidence in
  `docs/SECRETS_HISTORY_SCAN.md`).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).

---
# Session Handoff - v6.5.103 (Hardlink Twins)

## What landed (2026-11-25, continuation)
- **Hardlink twins collapse** (Gemini alias-boundary
  class): all three duplicate grouping implementations
  keep one representative per file identity (Migration
  12), so hardlink pairs no longer inflate groups or
  wasted-space claims. CLI test added.
- Version **6.5.103**: lean **113/113**, MSVC **113/113**,
  wiring **32/32**, JUCE **108 + 2 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited (Muse `6d2f6927`, ChatGPT
  `6b04ebe2`, Gemini `49b11b21` — briefed through
  v6.5.101 + this class answered for Gemini).
- History purge (owner; evidence in
  `docs/SECRETS_HISTORY_SCAN.md`).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).

---
# Session Handoff - v6.5.102 (Junction Cycles)

## What landed (2026-11-25, continuation)
- **Win32 scanner junction cycles**: reparse-point dirs
  are no longer descended (unless follow_symlinks); one
  file was being reported 41x through an ancestor
  junction, polluting the dedupe index. POSIX dirent
  walk got the lstat twin. Test added.
- Version **6.5.102**: lean **112/112**, MSVC **112/112**,
  wiring **32/32**, JUCE **107 + 2 skips**. Pushed.

## Still open (roast triage)
- Reviewer replies awaited on all three threads
  (Muse `6d2f6927`, ChatGPT `6b04ebe2`, Gemini
  `49b11b21` — all briefed through v6.5.101).
- History purge (owner; evidence in
  `docs/SECRETS_HISTORY_SCAN.md`).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).

---
# Session Handoff - v6.5.101 (Watch Pruned Polling)

## What landed (2026-11-25, continuation)
- **Watch**: `--debounce` was dead (global pre-parser
  rejected it, exit 2 — fixed + whitelisted
  `--full-scan-every`); polling now prunes by directory
  mtime with a full rescan every 20 ticks for content-only
  edits; redirected output flushed per tick. Real-process
  watch test added (baseline/NEW/MODIFIED).
- Version **6.5.101**: lean **111/111**, MSVC **111/111**,
  wiring **32/32**, JUCE **106 + 2 skips**. Pushed.
- Test-harness lesson: a child process with a
  non-inheritable stdout handle runs forever writing
  nothing (cout failbit, no crash) — the watch test's
  first implementation hit exactly that.
- **History secrets scan done (read-only)**:
  `docs/SECRETS_HISTORY_SCAN.md` — every .db blob in
  history checked; credential tables in metamcp.db are
  all 0 rows, zero credential-pattern hits anywhere.
  Remaining exposure is session transcript content.

## Still open (roast triage)
- History purge (owner decision): evidence now in
  `docs/SECRETS_HISTORY_SCAN.md`; what remains in
  history is session transcripts (privacy call, not a
  credential emergency).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- Muse review: thread `6d2f6927` (round 13 posted,
  reply pending); `ad74257e`/`fd6d6d5b` wedged.

---
# Session Handoff - v6.5.100 (Recursive Iterator Gone)

## What landed (2026-11-25, continuation)
- **Round-12 reviewer items fixed**: six range-for
  recursive walks (rename-batch, watch x2, integrity,
  C API search x2) + dirent Windows branch all use the
  shared `collect_regular_files` stack walk
  (`core/include/fo/core/fs_walk.hpp`); a wiring invariant
  test now bans the recursive iterator token in shipped
  sources. Rename-batch is collect-first (no live-iteration
  mutation, no self re-match). Flagged throwing calls
  converted to error-code forms.
- Version **6.5.100**: lean **110/110**, MSVC **110/110**,
  wiring **32/32**, JUCE **105 + 2 skips**. Pushed.
- Watch remains O(N) rescan per tick (reviewer item 5) —
  performance work, not correctness; still open.
- Fixture cleanup now verbatim-aware for >260 trees.

## Still open (roast triage)
- History purge / full-history secrets scan (owner) —
  reviewer urges moving this above other owner items.
- Watch incremental indexing (O(N) rescan per tick).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- Muse review: thread `6d2f6927` (round 13 to post);
  `ad74257e`/`fd6d6d5b` wedged, read-only history.

---
# Session Handoff - v6.5.99 (Flake Class Removed)

## What landed (2026-11-25, continuation)
- **Flake root-caused (test harness)**: fixture SetUp
  never cleaned its dir, so killed runs left residue
  (files + DB/WAL) the next run inherited — caught as
  "failed to hash 2 of 5" on a 3-file fixture. Dirs were
  also pointer-named (allocator reuse across tests).
  All fixtures now test-name their dirs and remove_all
  before creating; planted-residue proof passes.
  Lean/MSVC **108/108**, JUCE **103 + 2**, wiring **31/31**.
- **Reviewer thread moved**: `ad74257e` wedged mid-
  generation (3 queued reports); fresh thread
  **`6d2f6927`** opened with a full catch-up (round 12).
- Version **6.5.99**; counts confirmed at commit time.

## Still open (roast triage)
- History purge / full-history secrets scan (owner).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- Muse review: new thread `6d2f6927` (reply pending);
  `ad74257e`/`fd6d6d5b` wedged, read-only history.
- If full-suite flakes recur despite unique fixture dirs,
  root-cause child-process timing under load next.

---
# Session Handoff - v6.5.98 (Deep-Tree Walks Complete)

## What landed (2026-11-25, continuation)
- **Walker family fully converted**: copy engine (job
  enumeration + estimate), self-healing scrub, and the
  file watcher's recursive registration now stack-DFS.
  No `recursive_directory_iterator` throwing walks remain
  outside scanner_dirent (error-code form, verified fine).
- **New committed suite** `tests/test_deep_walks.cpp`
  (pruner/treemap/copy-estimate over a >260 tree).
- Version **6.5.98**: lean **108/108**, MSVC **108/108**,
  wiring **31/31**, JUCE **102 + 2 skips**. Pushed.
- **Flake pattern, fourth sighting**: single-test failures
  in full-suite runs under heavy parallel load (builds +
  browser CDP), always passing standalone and on re-run.
  Root-cause candidate: machine saturation, not code; if a
  fifth sighting lands, run the suite under controlled load
  before trusting either conclusion.

## Still open (roast triage)
- History purge / full-history secrets scan (owner).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- Muse review in chats `fd6d6d5b` / `ad74257e` (round
  9/10 replies still generating; rounds 10/11 posted).

---
# Session Handoff - v6.5.97 (Deep-Tree Walks Fixed)

## What landed (2026-11-25, continuation)
- **Remaining deep-tree walkers fixed**: linter (was
  exit-3 on >260 trees), data pruner (silently truncated),
  treemap sizing (aborted) all use the stack-DFS walk.
  `LintSurvivesOverLongPaths` pins the linter.
- Version **6.5.97**: lean **105/105**, MSVC **105/105**,
  wiring **31/31**, JUCE **100 + 2 skips**. Pushed.

## Still open (roast triage)
- History purge / full-history secrets scan (owner).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- enhanced_fileops/self_healing/file_watcher walks use
  `recursive_directory_iterator` with error codes (no crash
  observed; same iterator class as the std-scanner fault —
  convert on first symptom).
- Muse review in chats `fd6d6d5b` / `ad74257e` (round
  9/10 replies still generating; rounds 10/11 posted).

---
# Session Handoff - v6.5.96 (Long-Path Scanner AV)

## What landed (2026-11-25, continuation)
- **Long-path crash fixed**: scan/organize over trees with
  >260-char paths died 0xC0000005 inside the std scanner's
  recursive iterator. Now a manual stack DFS (per-level
  `directory_iterator`, error-code only): deep unenterable
  subtrees skip, siblings scan, exit 0. Organize into a
  >260 destination now fails per-file by name rather than
  crashing. `ScanSurvivesOverLongPaths` pins it.
- Version **6.5.96**: lean **104/104**, MSVC **104/104**,
  wiring **31/31**, JUCE **99 + 2 skips**. Two first-run
  load flakes (vault/MSVC single failures) cleared on
  re-run — watch for a third occurrence to root-cause.
- Process note: crashed probe processes can hold
  `build-lean/cli/fo_cli.exe` (LNK1104 on relink); the
  rename-aside workaround was needed once this session.

## Still open (roast triage)
- History purge / full-history secrets scan (owner).
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- Other recursive_directory_iterator users (treemap,
  data_pruner) throw rather than crash on deep trees —
  exit-3 class, not AV class; fix on demand.
- Muse review in chats `fd6d6d5b` / `ad74257e` (rounds
  9/10 replies still generating at last check).

---
# Session Handoff - v6.5.95 (Reserved Names + Vault Trail)

## What landed (2026-11-25, continuation)
- **Reserved-name data loss fixed**: rename/organize onto
  CON/NUL/AUX/COMn/LPTn (or trailing dot/space) used to
  "succeed" into the device — source gone. Now a named
  per-file refusal before any move. Case-collision pinned.
- **Vault**: failed initialize/lock/unlock now audited
  (paths only); `.meta` written before entry rename;
  `vault.after-encrypt`/`vault.after-decrypt` crash
  checkpoints prove both-present-or-done. Crypto verified:
  PBKDF2-HMAC-SHA256 100k, random per-entry salt+IV.
- **Wiring goldens** cover all array bridge routes (31/31).
- Version **6.5.95**: lean **103/103**, MSVC **103/103**,
  JUCE **98 + 2 designed skips**. Pushed.
- Round-9 reply triaged; round-10 report posted to
  `ad74257e`. Secrets-in-history: current-tree clean only —
  full-history scan + purge stays owner-gated.

## Still open (roast triage)
- History purge / full-history secrets scan (owner).
- Long-path (>260) organize destinations: untested; probe
  before promising support.
- Lean production checkout (owner); submodule pin drift
  (owner); `upstream/master` (owner); BobGUI meson (upstream).
- Muse review in chats `fd6d6d5b` / `ad74257e`.

---
# Session Handoff - v6.5.94 (True Crash Recovery)

## What landed (2026-11-25, continuation)
- **Intent-before-move crash recovery**: organize/rename
  journal `pending` intent → rename → complete; undo marks
  `undoing` first; every CLI start reconciles leftovers
  (Migration 13 status index). `FO_FAULT_AT` checkpoints
  hard-kill at after-mkdir/-intent/-rename/-complete and
  undo.after-rename; per-point crash tests assert recovery.
  One older fault test openly re-pinned (insert failure now
  prevents the move instead of rolling it back).
- **2s mtime tolerance** when file ID matches (FAT rounding).
- **Bridge `partial` payload** on nonzero exits + unified
  page renders it; undo JSON now names skipped records.
- Version **6.5.94**. Lean **100/100**, MSVC **100/100**,
  wiring **30/30**, JUCE rebuilt. All pushed (sync after
  the docs commit below).
- Claude round-8 reply triaged: crash-recovery + partial
  success + mtime edges were the valid items (all fixed);
  secrets-in-history scan remains an owner question
  (current-tree sweep was clean; history purge owner-gated).
  Round-9 report to be posted to thread `ad74257e`.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Submodule gitlink drift (161 of ~173): commit new pins or
  restore recorded pins (owner).
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e`.

---
# Session Handoff - v6.5.93 (Schema v2: Array Roots Wrapped)

## What landed (2026-11-25, continuation)
- **Schema v2**: every array-shaped CLI `--format=json`
  payload and every `fo_c_api` JSON string is wrapped as
  `{"schema_version": 2, "items": [...]}`; object payloads
  carry v2. Bridge fallbacks + error envelopes bumped; legacy
  React unwraps `items` in its fetch helpers; unified page
  renders verbatim. `docs/JSON_SCHEMA.md` rewritten for v2
  with the explicit breaking-change policy. New
  `JsonArrayRootsAreWrappedV2` conformance test. Lean
  **97/97**, MSVC **97/97**, wiring **29/29**, JUCE rebuilt.
- Version **6.5.93** (`012d8ee59`, pushed; tree in sync).
- Verified live: http://localhost:3131 `unified.html` 200;
  `/api/scan` returns `schema_version=2` with wrapped items
  through the pre-existing tray server (it relays the newest
  `fo_cli` per request). NOTE: that tray server still runs
  the pre-v6.5.93 `server.js`, so bridge-generated error
  envelopes stay v1 until the owner restarts it from the
  tray (Restart menu) — CLI payloads are already v2.
- Muse reviewer: read thread `ad74257e` — no round-8 reply
  yet; round-6 challenges all addressed (file-ID v6.5.83,
  envelope v6.5.81, arrays now). Round-8 report posted.

## Still open (roast triage)
- Crash-injection per-step test (kill between each journal
  write and its filesystem op) — reviewer suggestion, needs
  a harness hook; the 5,000-file kill test + v6.5.82 fault
  injection cover the realistic windows today.
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Submodule gitlink drift (161 of ~173): commit new pins or
  restore recorded pins (owner).
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e`.

---
# Session Handoff - v6.5.92 (Vault Ops Reach the Audit Ledger)

## What landed (2026-11-25, continuation)
- **Vault audit coverage**: `vault-lock`/`vault-unlock` now
  write `vault-lock`/`vault-unlock` entries to the forensic
  audit ledger on success (previously a silent hole — lock
  consumes the source file yet left no trace). New
  `VaultOpsAppearInAuditLedger` CLI test failed before,
  passes after. Lean **96/96**, MSVC **96/96**, wiring
  **28/28**, JUCE lane rebuilt.
- Version **6.5.92** (`7b1fc476f`, pushed; tree in sync).
- Verified live: dashboard on http://localhost:3131 serves
  `unified.html` (200) and `/api/audit` (200, verified:true)
  via the pre-existing tray-managed server; the bridge picks
  the newest `fo_cli` per request, so no restart was needed.
- Note: `fo_tests` target does not rebuild `fo_cli`; the CLI
  fixture runs the exe, so rebuild `fo_cli` explicitly when a
  CLI-only change seems not to take effect in tests.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Submodule gitlink drift (161 of ~173): commit new pins or
  restore recorded pins (owner).
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e`.

---
# Session Handoff - v6.5.91 (Convert Overwrite + Injection Guards)

## What landed (2026-11-24, continuation)
- **Convert guards**: existing outputs refused (no more
  ffmpeg `-y`), quote-bearing `--output` refused before any
  shell command is built. CLI test added. Lean **95/95**.
- Version **6.5.91**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e` (round 8 posted).

---
# Session Handoff - v6.5.90 (Ignore Add/Remove Report Truthfully)

## What landed (2026-11-24, continuation)
- **Ignore truthfulness**: repo add/remove return change
  counts; CLI exits 1 on duplicate-add/missing-remove; C API
  gains `"success"` on ignore actions. CLI test added. Lean
  ctest **94/94**.
- Version **6.5.90**.
- Also verified at v6.5.90: Qt (MinGW) and JUCE demos rebuilt
  against current core and launch-verified (Qt offscreen, JUCE
  live process check); quad concurrent scans on one fresh DB
  all succeed (v6.5.87 fixes).

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e` (round 8 posted).

---
# Session Handoff - v6.5.89 (Vault Unlock Path Traversal)

## What landed (2026-11-24, continuation)
- **Vault traversal guard**: flat-name check in
  `unlock_file` before any filesystem touch. CLI test added
  (attack via copied entry + `../`). Lean ctest **93/93**.
- Version **6.5.89**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e` (round 8 posted).

---
# Session Handoff - v6.5.88 (Bad Numeric Options Are Usage Errors)

## What landed (2026-11-24, continuation)
- **Strict numeric parsing**: all numeric CLI options go
  through `parse_int_strict`/validated sizes; bad values are
  named exit-2 usage errors (were process aborts). CLI test
  added. Lean ctest **92/92**.
- Note: four orphaned fo_cli processes (57100, 32200, 49472,
  35360; parents exited, access denied) hold renamed-aside
  lean exes; delete `build-lean/cli/fo_cli.exe.orphan-*` once
  they clear (untracked build artifacts).
- Version **6.5.88**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e` (round 8 posted).

---
# Session Handoff - v6.5.87 (Concurrent Invocations On One Database)

## What landed (2026-11-24, continuation)
- **DB concurrency**: busy_timeout 30s at open; migrations
  serialized under BEGIN IMMEDIATE with in-transaction version
  re-read; Engine scan/duplicates transactions BEGIN
  IMMEDIATE (deferred-upgrade BUSY bypassed the busy handler).
  Quad concurrent scans verified live. `tests/test_database.
  cpp` added. Lean ctest **91/91**.
- Version **6.5.87**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e` (round 8 posted).

---
# Session Handoff - v6.5.86 (Vault List Verifies The Password)

## What landed (2026-11-24, continuation)
- **Vault password gate**: `.vault_verify` HMAC verifier at
  init; wrong passwords rejected in `initialize()`; legacy
  vaults (no verifier) deliberately ungated. Verifier excluded
  from listings. CLI test added. Lean ctest **90/90**.
- Version **6.5.86**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e` (round 8 posted).

---
# Session Handoff - v6.5.85 (Bogus GPS Sentinels + Flow JSON Exit)

## What landed (2026-11-24, continuation)
- **Metadata GPS validation**: finite/range-checked before
  `has_gps` is set; flow JSON failure now exits 1 like text.
  EXIF-fixture CLI test added. Lean ctest **89/89**.
- Version **6.5.85**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e` (round 8 posted).

---
# Session Handoff - v6.5.84 (Malformed Patterns Are Usage Errors)

## What landed (2026-11-24, continuation)
- **Template validation**: `RuleEngine::validate_template`
  wired into rename/organize/YAML rules; rename-batch regex
  pre-validated. CLI test added. Lean ctest **88/88**.
- Version **6.5.84**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e` (round 8 posted).

---
# Session Handoff - v6.5.83 (Undo Verifies File Identity)

## What landed (2026-11-24, continuation)
- **File-ID identity**: Migration 12 `dest_file_id` recorded
  by organize/rename; `verify_identity` compares it (after
  size, before mtime/content). Replaced-file-with-forged-
  size+mtime now refused. Unit test added. Lean **87/87**.
- Version **6.5.83**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review in chats `fd6d6d5b` / `ad74257e` (round 8 posted).

---
# Session Handoff - v6.5.82 (Journal Rollback Fault Injection)

## What landed (2026-11-24, continuation)
- **Fault injection test**: operation_log view-swap forces
  the journal-failure branch; organize rollback proven
  end-to-end. Lean ctest **86/86**.
- Version **6.5.82**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review round 7 reply awaited (`ad74257e`).

---
# Session Handoff - v6.5.81 (Typed Bridge Error Envelope)

## What landed (2026-11-24, continuation)
- **Error envelope**: `cliError` helper in `server.js` (42
  catches routed), exit-code → HTTP status mapping, 31
  validation responses wrapped as `BAD_REQUEST`. Legacy React
  string contract preserved deliberately. Live dashboard
  restarted + re-probed (422s with CLI text). Wiring 28/28.
- Version **6.5.81**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review round 7 in chats `fd6d6d5b` / `ad74257e`.

---
# Session Handoff - v6.5.80 (Mtime Identity + Dir-Journal Order)

## What landed (2026-11-24, continuation)
- **Round-6 review acted on**: create-then-journal per segment
  for created dirs (+ user-content survival test); Migration
  11 `dest_mtime` identity for all files + configurable
  `--hash-limit-mb`; `docs/JSON_SCHEMA.md` breaking-change
  policy; git history verified in reported order. Lean
  **85/85**.
- Next: typed bridge error envelope (4xx/5xx split).
- Version **6.5.80**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review round 7 in chats `fd6d6d5b` / `ad74257e`.

---
# Session Handoff - v6.5.79 (Schema-Versioned JSON Contract)

## What landed (2026-11-24, continuation)
- **Schema v1 JSON contract**: `schema_version: 1` first field
  on all object-shaped CLI JSON + Exporter root + watch
  heartbeat (old heartbeats normalized on re-emit). CLI
  conformance test + golden wiring assertions. Lean **83/83**,
  wiring **28/28**.
- Version **6.5.79**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review round 6 in chats `fd6d6d5b` / `ad74257e`.

---
# Session Handoff - v6.5.78 (Journaled Dir Pruning + Hash Cap)

## What landed (2026-11-24, continuation)
- **Round-5 review acted on**: Migration 10 `created_dirs`
  journaling replaces the generic prune climb (pre-existing
  dirs now provably survive undo); journal-time hashing capped
  at 64 MB (big files keep size identity). Repo test added.
  Lean ctest **82/82**.
- Next: schema-versioned dashboard JSON (biggest agreed
  unstarted item).
- Version **6.5.78**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review round 6 in chat `fd6d6d5b` / fresh chat.

---
# Session Handoff - v6.5.77 (Undo Leaves No Empty Category Dirs)

## What landed (2026-11-24, continuation)
- **Dir-pruning undo**: move-back prunes emptied destination
  parents (empty-only remove, ≤8 levels); invariant now
  compares files+mtimes+directory set over the whole fixture.
  Remote `origin/main` tree verified free of the 18 untracked
  root files. Lean ctest **81/81**.
- Version **6.5.77**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review round 5 in chat `fd6d6d5b`.

---
# Session Handoff - v6.5.76 (Non-ASCII Paths Through argv)

## What landed (2026-11-24, continuation)
- **Wide argv**: fo_cli sources args from CommandLineToArgvW
  as UTF-8 (+ UTF-8 locale before parsing; shell32 linked).
  CJK root via argv verified on the shipped binary + a
  CreateProcessW regression test. Lean ctest **81/81**.
- **Secrets check (Muse round 4)**: metamcp.db/index.db/
  session files hold NO credentials (sensitive tables empty);
  fo.db has 3,156 scanned paths (privacy, not secrets).
- **Kill test (Muse round 4)**: 5,000-file organize hard-
  killed at 1.2s: 97 moved, 97 journaled, 0 orphaned — the
  per-file rename→autocommit-journal ordering holds
  empirically. Round-4 replies posted to chat `fd6d6d5b`.
- Version **6.5.76**.

## Still open (roast triage)
- History purge decision for root DBs in old commits (owner).
- Kill-test of journal under hard kill (Muse round 4) — argued
  safe by per-file autocommit journaling; empirical probe owed.
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review round 5 in chat `fd6d6d5b`.

---
# Session Handoff - v6.5.75 (Undo Verifies Content)

## What landed (2026-11-24, continuation)
- **Content-verified undo**: organize/rename record fast64 of
  the destination at journal time; `verify_identity` compares
  size then content before undo acts. Same-size/backdated-mtime
  tamper test added. Lean ctest **80/80**.
- Version **6.5.75**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- History purge decision for root DBs in old commits (owner).
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review round 4 in chat `fd6d6d5b`.

---
# Session Handoff - v6.5.74 (Root Hygiene + Copy-Undo Guard)

## What landed (2026-11-24, continuation)
- **Muse round 3 acted on**: 18 root clutter files untracked
  (+ ignore rules; history intentionally untouched — purge is
  an owner call since the DBs sit in public history); README
  test counts replaced with run-commands; Copy undo refuses
  copies modified after copying (mtime vs journal timestamp);
  journal residue appended to `<db>.residue.log`; CJK directory
  added to the undo invariant. Lean ctest **79/79**.
- Version **6.5.74**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- History purge decision for root DBs in old commits (owner).
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review round 4 in chat `fd6d6d5b`.

---
# Session Handoff - v6.5.73 (Atomic Batch IDs, Journal Rollback, CJK)

## What landed (2026-11-24, continuation)
- **Muse review, second half**: Migration 9 atomic batch
  counter (`BEGIN IMMEDIATE` upsert); organize/rename roll the
  move back when journaling fails (loud residue naming when
  rollback fails too); UTF-8 process locale from `Engine` so
  CJK/emoji filenames round-trip (invariant test extended).
  Lean ctest **78/78**.
- Version **6.5.73**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review round 3 in chat `fd6d6d5b`.

---
# Session Handoff - v6.5.72 (Undo Core Rework)

## What landed (2026-11-24, continuation)
- **Muse code review acted on** (operation_repository.cpp):
  uncapped batch undo; delete-batch refusal + `undo --dismiss`;
  size-identity check before undo touches a file; per-record
  skip reasons surfaced in CLI/C API; source-parent recreation;
  `get_by_id` primary-key lookup. Contract change documented in
  CHANGELOG (DeleteDoesNotBlockUndoOfMove re-pinned to
  refuse→dismiss→undo). ctest per-test TIMEOUT 120s added.
  Lean ctest **77/77**.
- Queued for v6.5.73 (Muse findings, second half): atomic
  batch-id allocation (MAX+1 race), journal-failure rollback in
  organize/rename, CJK filename probe.
- Version **6.5.72**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse review round 2 in chat `fd6d6d5b`.

---
# Session Handoff - v6.5.71 (Matrix Completeness Guard + mtime Invariant)

## What landed (2026-11-24, continuation)
- **Muse pushback acted on**: `--help`-driven completeness
  guard for the conformance matrix (caught `similar`); mtime
  added to the undo invariant; cross-volume transitions argued
  from journal-after-rename ordering in the thread reply;
  `operation_repository.cpp` pasted for Muse's deep review
  (chat `fd6d6d5b`). Lean ctest **70/70**.
- Version **6.5.71**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse code review reply awaited in chat `fd6d6d5b`.

---
# Session Handoff - v6.5.70 (Conformance Matrix + Undo Invariant)

## What landed (2026-11-24, continuation)
- **Muse roast received and triaged**: original claude.ai
  thread wedged mid-generation (server-side queue); a fresh
  thread answered from the repo description + stale snapshot.
  Surviving points acted on: conformance matrix across 16
  path commands; organize→undo byte-identical invariant test.
  Triage reply posted (chat `fd6d6d5b`). Lean ctest **69/69**.
- Version **6.5.70**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse may reply further in chat `fd6d6d5b`; read + triage.

---
# Session Handoff - v6.5.69 (Submodule Dashboard Stops Fabricating Commits)

## What landed (2026-11-24, continuation)
- **Dashboard generator honesty**:
  `scripts/generate_dashboard.py` no longer stamps uninitialized
  submodules with superproject HEAD data; regenerated
  `docs/SUBMODULES.md`. Also verified this turn: Qt MinGW demo
  rebuilds/launches on current core; scanners std/win32/dirent
  agree file-for-file; /api/quit downs the server (restarted);
  one duplicate tray icon (not mine) left running; 161 drifted
  submodule gitlinks + ~15k daemon-tracked ignored files are an
  owner hygiene decision (reported, not fought).
- Muse roast: owner signed in to claude.ai 2026-11-24; the
  v6.5.53-v6.5.69 summary + brutal-roast request was POSTED to
  the thread (composer verified sent). Reply had not landed
  after ~3 min (Free plan?) — next session read the
  `claude.ai/chat` tab via CDP 9222 and triage the reply.
- Version **6.5.69**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast reply awaited (posted 2026-11-24).

---
# Session Handoff - v6.5.68 (Search Grammar Works In Any Order)

## What landed (2026-11-24, continuation)
- **Search grammar**: flags parsed anywhere on the line; bare
  `search <q>` works again (guard exempts search; handler
  validates its own roots). CLI test added. Lean ctest **67/67**.
- Version **6.5.68**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.67 (Undo Restores The Whole Command)

## What landed (2026-11-24, continuation)
- **Batch undo**: Migration 8 `batch_id` on `operation_log`;
  organize/rename/delete-duplicates stamp one batch per command;
  `undo_last_batch()` (id-ordered grouping, stale fall-through)
  wired into CLI undo and C API undo (JSON keeps type/source/
  dest of newest record, adds count). Unit + CLI tests added.
  Lean ctest **66/66**.
- Version **6.5.67**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.66 (Similar Refuses Cross-Algorithm Queries)

## What landed (2026-11-24, continuation)
- **Similar algorithm guard**: providers whose `name()` is not
  `dhash` are refused against the dhash-only stored index
  (`opencv` alias still allowed). CLI test added. Lean ctest
  **64/64**.
- Version **6.5.66**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.65 (Organize Card Teaches {filename})

## What landed (2026-11-24, continuation)
- **Organize guidance**: dashboard card now teaches
  `{ext}/{filename}` and warns `{name}` drops extensions
  (engine behavior unchanged; literal `{ext}` folder for
  extension-less files left as a considered, visible behavior).
  Wiring **27/27**.
- Version **6.5.65**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.64 (Convert Failures Exit Nonzero)

## What landed (2026-11-24, continuation)
- **Convert honesty**: failed conversions are counted and exit
  1 (was stderr-only, exit 0, dashboard 200). CLI test added.
  Lean ctest **63/63**.
- Version **6.5.64**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.63 (Dashboard Surfaces CLI Failures)

## What landed (2026-11-24, continuation)
- **Bridge honesty**: `runCli` in `bobui_web/server.js` rejects
  on any nonzero CLI exit with the CLI's stderr (was:
  resolve-on-any-stdout). Dashboard restarted on the new server;
  live probes: vhash/afingerprint folders -> 500 with real
  message, OCR 200, classify 500 honest. Wiring **26/26**.
- Version **6.5.63**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.62 (Media Commands Process Every Input)

## What landed (2026-11-24, continuation)
- **Media multi-input**: `vhash`/`afingerprint` loop over all
  roots with per-file failure accounting (was first-input-only).
  CLI test added (WAV fixtures; skips in fingerprinter-less
  lanes). Lean ctest **62/62**. Similar positive path also
  covered by a test-only commit (`3dc4cc64a`, no version bump).
- Version **6.5.62**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.61 (Hash Reports Files It Could Not Hash)

## What landed (2026-11-24, continuation)
- **Hash failure honesty**: empty-hash results are named
  (stderr), skipped in output/storage, counted, and exit 1.
  CLI test added. Lean ctest **60/60**.
- Version **6.5.61**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.60 (Global Flags No Longer Hijack Positional Args)

## What landed (2026-11-24, continuation)
- **Positional parsing**: `pargs` view added in `fo_cli.cpp`;
  search/flow/autodev/sync/vault/clerk/oracle positional indexes
  no longer consume `--flag` tokens as values. CLI test added.
  Lean ctest **59/59**.
- Version **6.5.60**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.59 (Similar Names Its Input Expectation)

## What landed (2026-11-24, continuation)
- **Similar input check**: `similar` rejects non-file targets
  with a plain message instead of OpenCV warning dumps. CLI test
  added (accepts provider-missing in hasher-less lanes). Lean
  ctest **58/58**. Dashboard bridges re-probed live: classify
  500 names the missing model (v6.5.57 end-to-end), OCR 200,
  scan/duplicates 200.
- Version **6.5.59**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.58 (OCR Unavailable Is An Error)

## What landed (2026-11-24, continuation)
- **OCR honesty**: `IOCRProvider::unavailable_reason(lang)` added;
  Tesseract provider reports missing tessdata/`TESSDATA_PREFIX`;
  CLI `ocr` exits 1 when unavailable. CLI test added (accepts the
  provider-missing message in lanes without Tesseract). Lean
  ctest **57/57**.
- Version **6.5.58**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.57 (Classify Without A Model Is An Error)

## What landed (2026-11-24, continuation)
- **Classify honesty**: `IImageClassifier::unavailable_reason()`
  added; ONNX provider reports its model-load failure (once, not
  per file); CLI `classify` exits 1 when unavailable. CLI test
  added. Lean ctest **56/56**.
- Version **6.5.57**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.56 (Bogus Hasher Crashed The CLI)

## What landed (2026-11-24, continuation)
- **Crash fix**: `Engine::hasher()` throws instead of returning a
  null provider reference; `hash --hasher=bogus` is now a clean
  exit-3 error (was 0xC0000005). `--keep` validated at parse
  (exit 2). CLI test added. Lean ctest **55/55**.
- Note: the pre-fix crash left one orphaned `fo_cli.exe` (PID
  57100, access denied) holding `build-lean/cli/fo_cli.exe`
  renamed aside as `fo_cli.exe.orphan-57100`; delete it once the
  hung process clears (generated artifact, untracked).
- Version **6.5.56**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.55 (Missing Paths Are Errors)

## What landed (2026-11-24, continuation)
- **Honest paths**: CLI warns per nonexistent root and exits 1
  when a path-consuming command has no existing root (was empty
  output + exit 0); bogus export formats exit 2 instead of
  silently writing JSON. CLI test added. Lean ctest **54/54**.
- Version **6.5.55**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.54 (Plain Commands Stay Quiet)

## What landed (2026-11-24, continuation)
- **Lazy Engine services**: OmniClerk/Oracle/AutoDev/Sync/Swarm/
  OmniFlow/Nexus no longer constructed in `Engine`'s constructor;
  lazy accessors in `engine.cpp` preserve cross-wiring. Plain scan
  stderr 0 bytes; status commands still work on demand. CLI test
  added. Lean ctest **53/53**.
- Version **6.5.54**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending (claude.ai logged out in Edge).

---
# Session Handoff - v6.5.53 (Ignore Rules Actually Filter)

## What landed (2026-11-24, continuation)
- **Ignore globs work**: regex treatment made "*.log" unmatchable;
  glob matching (filename + path, brace alternation) implemented
  in `IgnoreRepository::is_ignored`. CLI test added.
  Lean ctest **52/52**.
- Version **6.5.53**.
- Follow-ups this turn: TODO `/api/delete` entry corrected (retired
  v6.5.47), ROADMAP status refreshed, side branches verified fully
  contained in `main` (0 unique commits), dashboard coverage audited
  (every CLI command has a route; daemon start honestly status-only),
  wiring **25/25**, bounded fuzz campaign **59.5M runs clean** (241s,
  BelowNormal). Muse coordination blocked: claude.ai is logged out
  in Edge CDP 9222 — owner sign-in requested.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.52 (Undo Works From Any Directory)

## What landed (2026-11-24, continuation)
- **Absolute journaling**: organize/rename record absolute
  source+destination, so undo works from any CWD. Old journals
  stay CWD-bound (documented). Test initially vacuous (helper
  never chdir'd) — corrected to fail pre-fix, pass post-fix.
  Lean ctest **51/51**.
- Version **6.5.52**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.51 (Vault Unlock Refuses Existing Destinations)

## What landed (2026-11-24, continuation)
- **Vault unlock guard**: existing destinations are refused
  (entry survives); the v6.5.36 atomic rename no longer doubles
  as a silent overwrite. CLI test added. Lean ctest **50/50**.
- Version **6.5.51**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.50 (Flow Actions Refuse to Overwrite)

## What landed (2026-11-24, continuation)
- **Flow overwrite guards**: `Action.MoveTo`/`Action.Rename` in
  the OmniFlow engine refuse existing destinations (run fails,
  bytes intact); CopyTo keeps explicit overwrite semantics.
  Registry test added. Lean ctest **49/49**.
- Version **6.5.50**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.49 (Stale-Authorization Guards)

## What landed (2026-11-24, continuation)
- **Reviewers' challenge answered**: delete-duplicates re-hashes
  candidates against the kept file at delete time (stale groups
  skipped, reported); undo never renames over an externally
  recreated source. Two failing-first CLI tests. Lean **48/48**.
- Gemini + ChatGPT both replied to the fix summary with this
  stale-authorization / external-drift challenge class.
- Version **6.5.49**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.48 (Scan JSON Carries mtime)

## What landed (2026-11-24, continuation)
- **Scan JSON `mtime`**: classic dashboard date column works
  again. CLI test added. Lean ctest **46/46**.
- Version **6.5.48**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.47 (Raw Delete Endpoint Retired)

## What landed (2026-11-24, continuation)
- **`/api/delete` retired**: dead (no caller), dangerous (raw
  unlink, no journal/audit). Route removed, 404 verified live,
  wiring-pinned. TODO live list updated.
- Version **6.5.47**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.46 (Dashboard Launch Scripts Set OCR Env)

## What landed (2026-11-24, continuation)
- **start-server.bat tessdata**: direct script launch now sets
  `TESSDATA_PREFIX` like the tray spawn does; proven on port 3132.
  MEMORY.md observations refreshed to current state (docs commit).
- Version **6.5.46**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.45 (Benchmark Executables Can Start)

## What landed (2026-11-24, continuation)
- **Benchmarks fixed**: `fo_benchmarks`/`fo_bench_dhash` got the
  runtime-DLL post-build copy; suite runs with real numbers
  (see CHANGELOG). Version **6.5.45**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.44 (MSI Packaging Uses VERSION.md)

## What landed (2026-11-24, continuation)
- **MSI lane parameterized** on `VERSION.md` + current build lanes
  (was hardcoded 2.2.1/2.2.0 and the dead `build/` tree). WiX is
  not installed here, so only the script logic is verified.
- Version **6.5.44**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.43 (Persona Example Rules Actually Load)

## What landed (2026-11-24, continuation)
- **Persona rules work**: dual-schema YAML loader (rules:/destination
  and persona pattern/target), filename glob patterns with brace
  alternation, `{filename}` placeholder, persona examples rewritten
  to supported placeholders with safe review-folder purge targets.
  Caught and fixed a self-inflicted regression (new struct field
  broke aggregate initializers; field now last + hazard noted).
  Lean ctest **45/45**.
- Version **6.5.43**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.42 (Fuzz Lane Builds; Campaign Clean)

## What landed (2026-11-23, continuation)
- **Fuzz lane fixed**: `build_fuzzer.bat` now disables the
  default-ON Omni shell that broke its configure. 60s libFuzzer
  campaign on RuleEngine templates: 14.5M runs, no crash.
- JUCE + Qt demos launch-verified at current core. Wiring **22/22**.
- Version **6.5.42**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.41 (Tray Restart Race Fixed)

## What landed (2026-11-23, continuation)
- **Tray lifecycle hardened**: no duplicate spawn against an
  externally running server, stop waits for port release before
  restart, tray-spawned servers get `TESSDATA_PREFIX` for OCR.
  Full lifecycle proven live; tray restarted on the new code.
  Wiring **21/21**.
- Version **6.5.41**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.40 (C API Header Deduplicated)

## What landed (2026-11-23, continuation)
- C API header deduplicated; declaration/definition sweep clean
  both directions. Version **6.5.40**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.39 (C API Vault Declared and Consumer-Tested)

## What landed (2026-11-23, continuation)
- **C API vault declared**: the four vault entry points existed in
  the .cpp but not the public header. Declared now, and
  `tests/test_c_api.cpp` proves the consumer chain (round-trip
  byte-identical + scan JSON). Lean ctest **44/44**.
- Version **6.5.39**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.38 (Sync Status Stops Claiming "In Sync")

## What landed (2026-11-23, continuation)
- **Sync status honesty**: `check_sync_status` hardcoded
  `is_in_sync = true`; now history-aware with an explicit
  "unknown" answer (reason surfaced by CLI + dashboard). Registry
  test pins it. Lean ctest **42/42**.
- Version **6.5.38**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.37 (Whole-Word Content Search Honored)

## What landed (2026-11-23, continuation)
- **Whole-word content search fixed**: literal mode ignored
  `content_whole_word` entirely. Boundary-scanning literal match
  added; CLI test pins "wor" ≠ "word", "word" matches.
  Lean ctest **41/41**.
- Version **6.5.37**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.36 (Vault Writes Are Atomic)

## What landed (2026-11-23, continuation)
- **Vault atomicity**: encrypt/decrypt write via `.part` temp +
  rename on success; wrong-password unlock no longer leaves a
  garbage output file. New CLI test. Lean ctest **40/40**.
- Version **6.5.36**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.35 (Audit Tamper Evidence, Honestly Bounded)

## What landed (2026-11-23, continuation)
- **Audit tamper probe**: forged entry detected (`verified:false`);
  tail truncation is a hash-chain design limit, now stated on the
  Audit card (wiring 20/20). 
- ChatGPT verdict in: interruption test credited; recommends the
  owner authorize a lean production checkout (not deletion).
- Version **6.5.35**.

## Still open (roast triage)
- Lean production checkout: owner authorization needed.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast still pending in its thread.

---
# Session Handoff - v6.5.34 (Undo Skips Stale Records)

## What landed (2026-11-23, continuation)
- **Undo robustness**: `undo_last` walks up to 50 recent records and
  undoes the first still-reversible one; stale records (destination
  gone, move-back throws) are skipped instead of blocking the whole
  journal. New CLI test. Lean ctest **39/39**.
- Version **6.5.34**.

## Still open (roast triage)
- Version churn (a bump per fix) is a standing user rule, not a bug.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast + ChatGPT verdict on the fix reports still pending.

---
# Session Handoff - v6.5.33 (Deletes No Longer Block Undo)

## What landed (2026-11-23, continuation)
- **Undo journal trap fixed**: Delete records (irreversible) were
  returned as "undoable" and blocked every earlier undo forever.
  `get_undoable` now filters to reversible types; CLI test proves
  organize → delete-duplicates → undo restores the moved file.
  Lean ctest **38/38**.
- Version **6.5.33**.

## Still open (roast triage)
- Version churn (a bump per fix) is a standing user rule, not a bug.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast + ChatGPT verdict on the fix reports still pending.

---
# Session Handoff - v6.5.32 (Submodule Inventory)

## What landed (2026-11-23, continuation)
- **`docs/SUBMODULE_INVENTORY.md`** (ChatGPT's requested read-only
  inventory): 173 libs dirs = 12 build-referenced, 5 framework
  trees, 159 unreferenced. Pruning remains owner-gated.
- Version **6.5.32** (docs-only code-wise; lanes rebuilt for parity).

## Still open (roast triage)
- Version churn (a bump per fix) is a standing user rule, not a bug.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast + ChatGPT verdict on the fix reports still pending.

---
# Session Handoff - v6.5.31 ({extension} Alias; Crash Consistency)

## What landed (2026-11-23, continuation)
- **`{extension}` alias**: dashboard taught a placeholder the
  RuleEngine didn't know (literal `{extension}` folders). Aliased
  to `{ext}`, card corrected, CLI test added. Lean **37/37**.
- **Crash probe (ChatGPT challenge)**: killed organize mid-run on
  600 files — 600/600 accounted for, audit chain verified, undo
  works. Note: destinations are CWD-relative; undo needs same CWD.
- HTTP smoke batch 2 all 200; `/api/quit` proven end-to-end
  (server stops, restarted after).
- Version **6.5.31**.

## Still open (roast triage)
- Version churn (a bump per fix) is a standing user rule, not a bug.
- 173 submodule dirs: inventory/pruning is an owner decision.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast + ChatGPT verdict on the fix reports still pending.

---
# Session Handoff - v6.5.30 (Vault Lock Warning; Watcher Verified)

## What landed (2026-11-23, continuation)
- **Vault lock warning**: dashboard now states lock removes the
  plaintext original (unlock restores), pinned by wiring test 19/19.
- **Watcher re-verified live**: heartbeat + event counting + HTTP
  bridge at this version.
- Version **6.5.30**.

## Still open (roast triage)
- Version churn (a bump per fix) is a standing user rule, not a bug.
- 173 submodule dirs: pruning is an owner decision.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast + ChatGPT reply to the fix report still pending.

---
# Session Handoff - v6.5.29 (Rename Safety)

## What landed (2026-11-23, continuation)
- **No silent overwrites**: organize/rename/rename-batch refuse
  existing destinations (MSVC rename replaces). Rename handlers
  also got the organize treatment: only successful renames are
  reported, `renamed`/`failed` counts, exit 1 on failure, live-DB
  files skipped. Two new CLI tests. Lean ctest **36/36**.
- Version **6.5.29**.

## Still open (roast triage)
- Version churn (a bump per fix) is a standing user rule, not a bug.
- 173 submodule dirs: pruning is an owner decision.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast + ChatGPT reply to the fix report still pending.

---
# Session Handoff - v6.5.28 (Delete-Duplicates Scoped and Honest)

## What landed (2026-11-23, continuation)
- **Delete-duplicates scoped**: it acted on all DB groups regardless
  of the requested folder (cross-folder deletion proven by test).
  Now filtered to the given roots; failed removals are no longer
  counted/journaled as deletions (`failed` count, exit 1).
  Lean ctest **34/34**.
- Version **6.5.28**.

## Still open (roast triage)
- Version churn (a bump per fix) is a standing user rule, not a bug.
- 173 submodule dirs: pruning is an owner decision.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).
- Muse roast + ChatGPT reply to the fix report still pending.

---
# Session Handoff - v6.5.27 (Organize Never Moves Its Own Database)

## What landed (2026-11-23, continuation)
- **Self-database protection**: organize excludes the active DB and
  its `-wal`/`-shm`/`-journal` files when `--db` lives inside the
  organized tree (`DatabaseManager::db_path()` added). New CLI test
  proves the DB stays put while other files move. Lean **33/33**.
- Roast replies: Gemini + ChatGPT in; ChatGPT was still composing
  on the fix report, Muse's roast still pending (nudged).
- Version **6.5.27**.

## Still open (roast triage)
- Version churn (a bump per fix) is a standing user rule, not a bug.
- 173 submodule dirs: pruning is an owner decision.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).

---
# Session Handoff - v6.5.26 (Organize Reports Failed Moves)

## What landed (2026-11-23, continuation)
- **Organize failure honesty**: failed moves were reported as done
  with exit 0. Now only successful moves are listed, JSON reports
  `moved`/`failed`, any failure exits 1. Proven by new CLI test +
  manual blocked-destination probe. Lean ctest **32/32**.
- Roasts: Gemini + ChatGPT delivered (posted to their threads +
  Muse thread; Muse's own roast pending). Fix reports posted
  back to all three.
- Version **6.5.26**.

## Still open (roast triage)
- Version churn (a bump per fix) is a standing user rule, not a bug.
- 173 submodule dirs: pruning is an owner decision.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).

---
# Session Handoff - v6.5.25 (Roast Fixes)

## What landed (2026-11-23, continuation)
- Roast requested from Muse+Gemini+ChatGPT via the Muse thread
  (CDP 9222); replies pending there. Self-roast fixes shipped now:
  flow action failures propagate (`FlowFailurePropagates`), engine
  init banners moved stdout→stderr (clean CLI stdout), ROADMAP
  scope-cut banner + unpinned status, README version unpinned,
  174 generated build files untracked (artifacts kept on disk).
- Version **6.5.25**.

## Still open (roast triage)
- Version churn (a bump per fix) is a standing user rule, not a bug.
- 130+ submodules, most not linked by any build: pruning is an
  owner decision (destructive); `docs/SUBMODULES.md` lists them.
- Omni* QML-demo mocks remain behind unbuilt GUI lanes.
- `upstream/master` decision (owner); BobGUI meson fix (bobgui repo).

---
# Session Handoff - v6.5.24 (build-msvc Lane Green Again)

## What landed (2026-11-17, continuation)
- **build-msvc unblocked**: the orphaned `fo_tests.exe` processes
  cleared; the remaining failure was the runtime-DLL resolver
  hard-erroring on conflicting DLL paths. Conflicts are now
  collected (`CONFLICTING_DEPENDENCIES_PREFIX`), script declared as
  a post-build `DEPENDS`. `build_headless.bat` completes; msvc
  ctest **29/29** alongside lean **29/29**, JUCE **26/26**,
  wiring **18/18**. All three CLI lanes at one version.
- Version **6.5.24**.

## Blocked / needs owner
- `upstream/master` unrelated-history decision (owner).
- BobGUI `meson.build` corruption is in the bobgui repo itself.
- Real Oracle RAG (embedder + local LLM) remains a design.

---
# Session Handoff - v6.5.23 (Convert Command Unbroken)

## What landed (2026-11-17, continuation)
- **Convert fixed**: global parser now accepts `--to=`/`--quality=`
  (dashboard grammar), and the handler's `substr(4)` off-by-one
  (`=flac`) is `substr(5)`. Real wav→flac (CLI) and wav→ogg (HTTP)
  conversions verified; new CLI test pins both defects. Classify
  without a model fails honestly; flow list remains scaffold.
- Coordination: caught up via imported-session scan (no Antigravity/
  ChatGPT transcripts exist on this machine; aider + claude-code
  only) and posted the v6.5.22 check-in to the Muse thread over
  Edge CDP 9222 as [muse-wonder-1.0 in Muse Code in bobfilez].
- Version **6.5.23**. Suites: lean **29/29**, JUCE **26/26**,
  wiring **18/18**.

## Blocked / needs owner or other session
- Orphaned `fo_tests.exe` PIDs 19124/63392 still lock
  `build-msvc/tests`; stop them where possible, then rerun
  `scripts/build_headless.bat`.
- `upstream/master` unrelated-history decision (owner).
- BobGUI `meson.build` corruption is in the bobgui repo itself.

---
# Session Handoff - v6.5.22 (Scrub Actually Detects Corruption)

## What landed (2026-11-17, continuation)
- **Scrub fixed**: baselines were in-memory only (could never detect
  anything). They now persist via `FileRepository` hashes; scrub
  reports baselined/corrupted counts and corrupted file names in
  text and JSON. New CLI integration test + live HTTP tamper cycle
  verify it. Lean ctest **28/28**, JUCE-tree ctest **25/25**,
  wiring **18/18**.
- Version **6.5.22**.

## Blocked / needs owner or other session
- Orphaned `fo_tests.exe` PIDs 19124/63392 still lock
  `build-msvc/tests`; stop them where possible, then rerun
  `scripts/build_headless.bat`.
- `upstream/master` unrelated-history decision (owner).
- BobGUI `meson.build` corruption is in the bobgui repo itself.
- Many Omni* QML-demo engines remain mocks (cluster/git/sec/verse
  etc.); only dashboard-exposed paths are being hardened.

---
# Session Handoff - v6.5.21 (Oracle Stops Fabricating Answers)

## What landed (2026-11-17, continuation)
- **Oracle honesty**: `OmniOracle::ask/summarize` fabricated citations
  and summaries; both now report "not connected" with no citations,
  and the dashboard card describes the scaffold truthfully. New
  registry test pins it. Verified CLI + HTTP. Full POST-endpoint
  smoke (search/lint/organize/rename/scrub/clerk/oracle/prune/export)
  all 200 at v6.5.20 before the bump.
- Version **6.5.21**. Suites: lean **27/27**, JUCE **24/24**,
  wiring **18/18**.

## Blocked / needs owner or other session
- Orphaned `fo_tests.exe` PIDs 19124/63392 still lock
  `build-msvc/tests`; stop them where possible, then rerun
  `scripts/build_headless.bat`.
- `upstream/master` unrelated-history decision (owner).
- BobGUI `meson.build` corruption is in the bobgui repo itself.
- Real Oracle RAG (embedder + local LLM) remains a design, not code.

---
# Session Handoff - v6.5.20 (Similar-Image Search Fixed)

## What landed (2026-11-17, continuation)
- **`similar` fixed end-to-end**: dhash bit convention aligned between
  the stb hasher and the OpenCV perceptual hasher; `find_similar_images`
  now parses the stored hex (and skips garbage instead of reading 0).
  Proven by CLI workflow (decreasing-gradient pair: 0 matches before,
  2 after) and 3 new `FileRepoFixture` tests. Existing DBs need a
  `hash --hasher=dhash` re-run to refresh old-convention rows.
  Media chain re-verified (hash/vhash/afingerprint/OCR). Lean ctest
  **26/26**, JUCE-tree ctest **23/23**, wiring **18/18**.
- Version **6.5.20**.

## Blocked / needs owner or other session
- Orphaned `fo_tests.exe` PIDs 19124/63392 still lock
  `build-msvc/tests`; stop them where possible, then rerun
  `scripts/build_headless.bat`.
- `upstream/master` unrelated-history decision (owner).
- BobGUI `meson.build` corruption is in the bobgui repo itself.

---
# Session Handoff - v6.5.19 (Root Artifact-Dir Cleanup)

## What landed (2026-11-17, continuation)
- `.gitignore` now covers the untracked probe/demo build trees
  (188 MB `temp_btk_probe/` etc.); nothing deleted. Qt MinGW lane
  rebuilt + launch-verified offscreen against current core.
- Version **6.5.19**. Suites: lean **23/23**, JUCE **20/20**,
  wiring **18/18**.

## Blocked / needs owner or other session
- Orphaned `fo_tests.exe` PIDs 19124/63392 still lock
  `build-msvc/tests`; stop them where possible, then rerun
  `scripts/build_headless.bat`.
- `upstream/master` unrelated-history decision (owner).
- BobGUI `meson.build` corruption is in the bobgui repo itself.
- Root still has tracked generated files inside several build dirs
  (daemon sweeps); untracking them is a separate, deliberate change.

---
# Session Handoff - v6.5.18 (Dashboard Ignore-Rule Removal)

## What landed (2026-11-17, continuation)
- **Ignore-rule removal reached the dashboard**: route↔card diff found
  `/api/ignore/remove` uncalled while the card text promised removal.
  Added the Remove rule button (+ wiring test, 18/18) and verified the
  live add → list → remove → gone cycle. `/api/delete` stays for the
  legacy React UI; unified deletes only via dry-run duplicate-delete.
- Tray started (PID 53496 this session) and all GET/POST dashboard
  smokes re-verified at v6.5.17 before the bump.
- Version **6.5.18**. Suites: lean **23/23**, JUCE **20/20**.

## Blocked / needs owner or other session
- Orphaned `fo_tests.exe` PIDs 19124/63392 (parents exited; access
  denied) lock `build-msvc/tests`. Stop them where possible, then
  rerun `scripts/build_headless.bat`.
- `upstream/master` unrelated-history decision (owner).
- BobGUI `meson.build` corruption is in the bobgui repo itself.

---
# Session Handoff - v6.5.17 (Test Runtime-DLL Resolution)

## What landed (2026-11-17, continuation)
- **fo_tests runtime-DLL copy fixed**: replaced the incomplete
  `TARGET_RUNTIME_DLLS`-only copy with `tests/copy_runtime_deps.cmake`
  (`file(GET_RUNTIME_DEPENDENCIES)` on the linked exe; api-ms/ext-ms
  excluded). A glob-copy attempt in between proved why: debug-variant
  DLLs shadowed good ones (`0xc0000005`). Lean ctest **23/23**,
  JUCE-tree ctest **20/20**, wiring **17/17**.
- Version **6.5.17**.

## Blocked / needs owner or other session
- `build-msvc/tests`: two hung `fo_tests.exe` (PIDs 19124, 63392;
  access denied, no readable identity) lock its DLLs; VS2019 test
  binary also access-violates at discovery. Whoever owns those
  processes should kill them, then rerun `scripts/build_headless.bat`.
- `upstream/master` unrelated-history decision (owner).
- BobGUI: corrupt `project()` in bobgui's own `meson.build` at both
  its recorded pin and its `main`; fix in the bobgui repo, then retry
  `scripts/build_bobgui_gui.bat`.

---
# Session Handoff - v6.5.16 (Vault Registry Registration)

## What landed (2026-11-17, continuation)
- **Vault registry bug fixed**: `IVaultManager` registration was a
  TU-local static initializer with a never-called `force_register_*()`,
  so the C API vault (`fo_bobfilez_vault_initialize`) always got
  `nullptr`. Now explicit `register_vault_manager()` via
  `register_all_providers()`, plus `ensure_providers_registered()` in
  the C entry point. New `VaultManagerRegistered` test failed before,
  passes after. Lean ctest **23/23**, JUCE-tree ctest **20/20**,
  wiring **17/17**. JUCE demo re-launch-verified at v6.5.16.
- Version **6.5.16**.

## Next
1. `upstream/master` unrelated-history decision (owner).
2. BobGUI: fix `project()` duplication upstream in bobgui, then retry
   the Meson lane (unblocks verifying the C API vault direct path).
3. Audit remaining `(void)reg_*` guard registrations only if a binary
   shows a missing provider; the pattern works when the register
   function is actually called from `register_all_providers()`.

---
# Session Handoff - v6.5.15 (JUCE Demo + Perceptual Registration)

## What landed (2026-11-17, continuation)
- **JUCE demo builds + launches**: VS18 vcvars discovery and vcpkg
  applocal PowerShell pinning in `scripts/build_juce_gui.bat`;
  `libs/JUCE` restored to recorded gitlink `762a87fc30`. Exe
  launch-verified (title "bobfilez JUCE Demo", responding).
- **Perceptual registration bug fixed**: `perceptual_opencv.cpp` static
  registrar was linker-dropped (JUCE-tree registry test failed on
  `dhash`); now explicit `register_perceptual_opencv()`, `FO_HAVE_OPENCV`
  PUBLIC, guarded registry test added. Lean ctest **22/22**, JUCE-tree
  ctest **19/19**, wiring **17/17**.
- **BobGUI probed, blocked upstream**: `libs/bobgui/meson.build` has a
  duplicate unclosed `project()` at its pin; `meson setup` fails parsing.
  Submodule not edited; probe dir removed.
- Version **6.5.15**.

## Next
1. `upstream/master` unrelated-history decision (owner).
2. BobGUI: fix `project()` duplication upstream in bobgui, then retry
   the Meson lane (GTK4 wrap chain is the next hurdle).
3. Native BobUI/BTK lanes still blocked on framework provider surfaces;
   Qt MinGW and JUCE VS18 are the verified native demo paths.

---
# Session Handoff - v6.5.14 (Qt Demo Aligned)

## What landed (2026-11-17, continuation)
- **Qt demo builds + launches**: aligned `frontends/qt/src/main.cpp`
  with current core APIs; MinGW Qt 6.11 lane (`build-qt-mingw/`,
  w64devkit GCC) compiles clean and the exe stays alive offscreen.
  Web dashboard remains the primary verified UI.
- Version **6.5.14**. Suites at last full run: ctest 21/21, wiring 17/17.

## Next
1. `upstream/master` unrelated-history decision (owner).
2. Other native demo lanes (BobUI/JUCE/BTK/BobGUI) still need their
   framework providers; Qt MinGW is the verified native path today.

---
# Session Handoff - v6.5.13 (Scanner + Rename-Batch Fixes)

## What landed (2026-11-17, continuation)
- **Scanner file roots** fixed in std/dirent/win32 (Hash/Metadata file
  inputs no longer return `[]`); **dirent directory-scan crash** fixed
  (vendored shim truncates the CRT find handle — Windows path now uses
  `std::filesystem`, vendored code untouched); **batch rename** grammar
  fixed at parser + bridge + card (it could never run before).
- New tests: scanner contracts, rename-batch dry-run. ctest **21/21**,
  wiring **17/17**. Version **6.5.13**.
- Finding: `fo_qt_demo` configures (MinGW Qt) but `main.cpp` is
  bit-rotted vs current core APIs → TODO `qt-demo-alignment`.

## Next
1. Qt demo API alignment (or honest retirement).
2. `upstream/master` unrelated-history decision (owner).

---
# Session Handoff - v6.5.12 (Audit-Write Coverage)

## What landed (2026-11-17, continuation)
- CLI integration tests now cover **applied** audit writes: organize
  move and delete-duplicates each append their ledger entry with a
  verified chain (16/16 ctest, 15/15 wiring, HTTP smoke all 200).
- Version **6.5.12**.

## Next
1. `upstream/master` unrelated-history decision (owner).
2. Native GUI lanes remain blocked on Qt/BobUI provider surface (docs).

---
# Session Handoff - v6.5.11 (CLI Integration Tests)

## What landed (2026-11-17, continuation)
- **CLI-level integration tests**: `tests/test_cli_integration.cpp`
  drives the real `fo_cli.exe` (audit limit validation exits, fresh-DB
  audit JSON, vault byte-compare round-trip). ctest **14/14**, wiring
  **15/15**, HTTP smoke all 200. Lean lane gained cloud scanners via the
  shared GTest prefix — both lanes now expose the identical provider
  matrix (runtime DLLs sidecarred beside each CLI).
- Version **6.5.11**.

## Next
1. `upstream/master` unrelated-history decision (owner).
2. Native GUI lanes remain blocked on Qt/BobUI provider surface (docs).
3. Grow tests toward organize/delete audit-write integration at CLI level.

---
# Session Handoff - v6.5.10 (Tracked Tests Restored + Browser Verification)

## What landed (2026-11-17, continuation)
- **Tracked `tests/` tree restored**: 11 GTest cases (audit ledger incl.
  tamper detection, operation journal incl. rename undo, provider
  registry incl. FFmpeg/Chromaprint when compiled in). `ctest` in
  build-lean: **11/11 passed**. Runtime DLLs auto-copied for discovery.
- **Browser verification via MS Edge CDP 9222** (agent-browser): 32-card
  dashboard rendered; Scan/Audit/Providers/Watcher exercised with real
  results (screenshot inspected). Caught and restarted a stale pre-audit
  server. Shared-Edge tabs from other apps can steal focus — re-open the
  dashboard URL in the same command chain when automating.
- Version **6.5.10**. MCP read-only surface (status, observability,
  memory, workflows, repograph) healthy.

## Next
1. `upstream/master` unrelated-history decision (owner).
2. Native GUI lanes remain blocked on Qt/BobUI provider surface (docs).
3. Grow `tests/` coverage toward CLI-level integration (audit --limit,
   vault round-trip) as the tree matures.

---
# Session Handoff - v6.5.9 (Media Providers Live — Matrix Complete)

## What landed (2026-11-17, continuation)
- **Video/audio providers live in both lanes**: prebuilt FFmpeg 9 shared
  release + Chromaprint 1.6 built against staged FFTW in `build-lean-deps`
  (vcpkg FFmpeg source build abandoned after fork-exhaustion and a
  90-minute `sanm.c` optimizer spin). Explicit registration for both
  providers (static registrars were linker-dropped), `FFmpeg::swresample`
  linked, `Chromaprint::` alias. Verified: `--list-vhash` ffmpeg,
  `--list-ahash` chromaprint, identical MP4 frame hashes across lanes,
  59 chunks on 10s WAV, HTTP 200s, wiring tests 15/15.
- Version **6.5.9**. **Every provider row is populated in every lane.**

## Next
1. Restore/recreate tracked C++ `tests/` tree.
2. `upstream/master` unrelated-history decision (owner).
3. Native GUI lanes remain blocked on Qt/BobUI provider surface (docs).

---
# Session Handoff - v6.5.8 (Persistent Audit Query)

## What landed (2026-11-16, continuation)
- **Audit is real end-to-end**: `make_audit_logger()` factory over the
  Engine's persistent DB, `fo_cli audit [--limit=N] [--format=json]` with
  full hash-chain verification, audit writes at applied organize-move /
  delete-duplicates / rename sites, `/api/audit` + dashboard card wired.
  Verified on an isolated DB from both lanes (same chain hash,
  `verified: true`, `--limit=0` → exit 2). Wiring tests 14/14.
- Version **6.5.8** (both lane banners rebuilt and checked).

## Next
1. Opt-in FFmpeg/Chromaprint `media-analysis` decision for vhash/audio.
2. Restore/recreate tracked C++ `tests/` tree.
3. `upstream/master` unrelated-history decision (owner).

---
# Session Handoff - v6.5.7 (Manifest Parity + Coverage Completion)

## What landed (2026-11-16)
- **Full manifest parity**: `scripts/build_headless.bat` completes and now
  supplements the vcpkg manifest with the verified lean provider prefix,
  sidecars its DLLs, and selects an installed PowerShell for vcpkg applocal.
  Manifest `fo_cli --modules` shows cloud scanners plus OCR, ONNX Classify,
  and OpenCV perceptual hash in one v6.5.7 binary.
- **Dashboard coverage completed**: Provider Directory, model-setup help,
  AutoDev/Sync status + Sync history, and Forensic Audit availability cards
  were added to the one-page dashboard. `/api/audit` no longer fabricates
  empty history; it honestly returns 501 pending a persistent CLI query.
  Video/audio cards now badge missing optional media-analysis providers.
- **Verification**: wiring tests 13/13; full HTTP smoke passed health,
  page, providers, model help, AutoDev/Sync, audit honesty, modules, scan,
  OCR, Classify, watcher, and quit. Tray started the server, health passed,
  and `/api/quit` stopped it. Existing `fo_tests.exe`: 436 passed, 1 skipped
  (stale binary; no tracked `tests/` tree in this checkout).
- **Scripts**: duplicate destructive `build_with_env.bat` archived;
  `dev_all.bat` now calls the maintained wiring test file explicitly.

## Blockers / next
1. Implement persistent `audit` CLI query, then wire `/api/audit` for real.
2. Decide on opt-in FFmpeg/Chromaprint `media-analysis` build for vhash/audio.
3. Restore/recreate tracked C++ `tests/`; do not treat stale `fo_tests.exe`
   as current-source proof.
4. `upstream/master` has no merge base with `main` (73/62 divergent commits);
   owner must choose separate/import/replace rather than a blind merge.
5. Direct push still depends on the host credential path; the Protocol
   daemon has been sweeping and pushing local commits during this session.

---
# Session Handoff - v6.5.5 (All Providers Live)

## What landed (2026-11-14, cron continuation)
- **OCR live — provider matrix complete**: Tesseract 5.5.2 (vcpkg, 40 min)
  in `build-lean-deps`; real `recognize` implemented in the providers/
  Tesseract class (was placeholder), explicit registration, `::tesseract::`
  shadowing fix. `--modules`: OCR tesseract · Classifiers onnx ·
  Perceptual opencv · Vault OpenSSL — **no empty rows, no badged cards**.
  Verified CLI + HTTP ("Hello bobfilez OCR 2028"). Server needs
  `TESSDATA_PREFIX=…\build-lean-deps\x64-windows\share\tessdata`.
- Version **6.5.5**.

## Next
1. Full manifest `build-msvc` rebuild only with ample disk + short vcpkg path.
2. C++ `fo_tests` rebuild needs a restored `tests/` tree (absent here).
3. Softmax DONE in v6.5.6 (confidences are probabilities now).

---
# Session Handoff - v6.5.4 (Classify Provider Live)

## What landed (2026-11-14, cron continuation)
- **Classify live**: prebuilt ONNX Runtime 1.20.1 + OpenCV 4.10.0 staged in
  `build-lean-deps` (vcpkg source chain abandoned at protobuf — ~10 obj/15min;
  abseil needed `VCPKG_MAX_CONCURRENCY=4`). Fixed the real registration bug:
  `classifier_onnx.cpp` static registrar was linker-dropped; now explicit
  `register_classifier_onnx()` in `register_all_providers()`. Verified:
  `--modules` shows `Classifiers: onnx`, Perceptual gains `opencv`; Classify
  CLI + HTTP 200 (MobileNetV2 in `bobui_web/model.onnx` at runtime — not
  committed; labels.txt optional).
- Version **6.5.4**. Tray gained watcher-aware status (`7859d44bc`).

## Next
1. OCR: Tesseract vcpkg chain (smaller than OpenCV was) or prebuilt.
2. Full manifest `build-msvc` rebuild only with ample disk + short vcpkg path.
3. Softmax in classifier postprocess (confidences are raw logits today).

---
# Session Handoff - v6.5.3 (Queryable Watcher — Last 501 Closed)

## What landed (2026-11-14, cron continuation)
- **Queryable watcher**: `watch` publishes a temp-dir heartbeat JSON every
  poll (events + last event); new `watch-status` CLI query + `/api/watch/status`
  bridge report fresh/stale/absent honestly (200, no 501). Live-verified:
  file creation moved events 0→1. **No honest-501 bridges remain.**
- Version **6.5.3**. Lean lane (`build-lean/`) is the verified MSVC build;
  server picks it as newest. Tests 9/9, dashboard smoke all 200.

## Next
1. Provider builds for OCR/Classifiers (Media cards badge their absence).
2. Full manifest `build-msvc` rebuild only with ample disk + short vcpkg path.
3. Tray custodian badge (IDEAS §6) — duplicate-space in the tray tooltip.

---
# Session Handoff - v6.5.2 (Vault Proof + Lean MSVC Lane)

## What landed (2026-11-14, cron continuation)
- **Vault proven end-to-end**: standalone OpenSSL 3.6.1 (`build-lean-deps/`,
  vcpkg classic mode, 1.5 h BelowNormal build) + lean MSVC lane (`build-lean/`,
  no manifest toolchain; all heavy deps are `find_package(QUIET)`-optional).
  CLI and HTTP init→lock→list→unlock round-trips restore byte-identical
  content (`restored-match: true`). Fixed MSVC `std::max` macro break
  (`NOMINMAX` in `enhanced_fileops.cpp`). Server CLI candidates now include
  `build-lean/cli/fo_cli.exe`; OpenSSL DLLs sidecarred beside it.
- Version **6.5.2** (`VERSION.md` + `version.hpp`, binary banner verified).
- Disk discipline learned: full manifest rebuild wipes `vcpkg_installed`
  and needs 60 packages + >10 GB; purged regenerable `vcpkg/buildtrees`
  (kept `openssl`) recovering ~7 GB. Never run two builds on one tree.

## Next
1. `watch-status` queryable watcher (last honest-501 bridge).
2. Provider builds for OCR/Classifiers (Media cards already badge absence).
3. Full manifest `build-msvc` rebuild only with ample disk + short vcpkg path.

---
# Session Handoff - v6.5.1 (Dashboard Wiring Fixes + Script Archive + Governance)

## What landed (2026-11-14, Muse Code / muse-wonder-1.0)
- **Dashboard wiring fixes** (`bobui_web/`): History primary button no longer
  fires `/api/undo` on load; `api()` surfaces HTTP errors (501 vault/watch
  now render as errors, not OK); Clerk bridge runs real `clerk status`
  grammar (card relabeled *OmniClerk status* — `clerk start` blocks forever,
  terminal-only); Oracle forwards the bare question; Search forwards
  wildcard/case/whole-word/invert/max-results; tray polls `/api/health` +
  quits via `/api/quit` (was dead `/health` + `/quit`); `start-server.bat`
  defaults `PORT=3131`.
- **Durable tests**: `bobui_web/test/wiring.test.js` (`node --test test/`,
  6 tests incl. live health + honest-501) wired into `scripts/dev_all.bat`.
- **Scripts cleanup (nondestructive)**: 20 root `patch_*.py`/`fix_*.py`
  one-shots `git mv`'d to `scripts/archive/root-helpers/` (see
  `scripts/README.md`). Canonical entrypoints unchanged.
- **Governance**: created missing `VISION.md`; refreshed `ROADMAP.md`
  (status v6.5.1), `TODO.md` (+`rebuild-cli`), `MEMORY.md`, `DEPLOY.md`
  (unified page + tray + tests), `IDEAS.md` (§6 pivots), `CHANGELOG.md`.
  Version: `VERSION.md` + `version.hpp` → **6.5.1**.

## Verified this session
- `node --test test/` in `bobui_web`: 6/6 pass (live server on :3199).
- `build-msvc/cli/fo_cli.exe --help` exit 0 but banner **v6.3.0** (stale);
  `--modules`: Scanners gdrive/std/win32/dirent/s3/azure, Hashers
  fast64/sha256/xxhash/blake3/dhash, Metadata tinyexif, **OCR/Classifiers
  empty**. `clerk`/`oracle` = Unknown command on this binary (source in
  `cli/fo_cli.cpp:2005/2027` has the dispatch — rebuild required).

## Open / next (in priority order)
1. ~~`rebuild-cli`~~ DONE same session: MinGW lane rebuilt (`build-mingw/`,
   w64devkit gcc 15.2, BelowNormal) after restoring `libs/hash-library` +
   `libs/xxHash` pins; fresh CLI = v6.5.1, live smoke health/scan/clerk/
   oracle/modules/history all 200, wiring tests 7/7. Server now picks the
   newest-built CLI by mtime. `fo_tests` not rebuilt (no `tests/` tree in
   this checkout); stale MSVC binary passes 436/437.
2. `vault-cli`, `watch-status`, `submodule-blake3` (pre-existing TODO items).
3. Do NOT stage `libs/*` gitlink drift (160+ submodules show modified);
   commit explicit paths only.

---
# Session Handoff - v6.5.0 (MinGW Lane + Exit-Crash Fix + Verified Bridges)

## What landed (2026-11-03, this turn)
- Portable no-admin toolchain at `C:/tools` (cmake 4.4.5, ninja 1.13.2, winlibs gcc 16.2).
  `scripts/build_headless_mingw.bat` configures `build-mingw/`; full compile+link OK.
- `build-mingw/cli/fo_cli.exe` runs; every CLI invocation previously crashed at exit
  (`0xC0000374`) — root cause was `SwarmHub` use-after-destroy during static teardown
  (Engine `get_shared` autodev chain). Fixed by leaking the hub singleton on purpose
  (`core/src/swarm_engine.cpp`). Latent on MSVC too.
- `lint` CLI was implemented but never dispatched — wired with `--format=json`.
- `bobui_web/server.js`: `extractJson` payload extractor (service chatter on stdout),
  `build-mingw` CLI candidate, organize/delete-duplicates arg fixes, watch 501.
- HTTP-verified end-to-end (node server, this host): scan/duplicates/stats/lint/count/
  organize/delete-duplicates/scrub/undo/hash/metadata/export/search/history/ignore/
  flow-list/modules = 200; vault + watch/status = honest 501.

## Open (not started)
- `vault-cli` (needs OpenSSL/MSVC host), `watch-status` query mode, `submodule-blake3`
  gitlink repair, `tests/` dir absent (FO_BUILD_TESTS defaults ON against nothing),
  model-dependent bridges unverified (ocr/similar/vhash/afingerprint/classify/oracle).
- Do NOT stage `libs/*` gitlink drift or `build-mingw/` (now gitignored); `fo.db`
  reverted (test artifact, tracked).

## Next
- Bump tested: VERSION.md + version.hpp = 6.5.0, CHANGELOG + TODO updated here.
- `git add` explicit paths only, commit `feat:`/`fix:` prefix, push.

---# Session Handoff - v6.3.4 (Autonomous Coordination & Feedback)

## Executive Summary
This session focused on maturing the **Autonomous Ecosystem** (Phase 10 of the Roadmap) by transitioning from a singleton-heavy architecture to an explicit **Dependency Injection (DI)** model for coordination services (`INexus`, `ISwarmEngine`, `IAutonomousSyncService`). This was critical to enable reliable integration testing where multiple `Engine` instances coexist without interfering with each other's state or database connections.

## Completed Milestones
- **Architectural Refactoring (Ownership & DI)**: Services are now created as `shared_ptr` within the `Engine` and wired together using explicit setters (`set_swarm_engine`, `set_sync_service`).
- **Registry Enhancements**: `Registry<T>` now supports `get_shared` and `clear_shared_instances` to manage singleton lifecycle during test teardown.
- **Distributed Swarm Simulation**: The `SwarmHub` (process-local routing) allows independent nodes to broadcast file update notices, simulating a cluster environment.
- **Autonomous Protocol Logic**:
    - Mandatory checkpointing (automatic `upload_file`) before task execution in `AutonomousDevProtocolImpl`.
    - Human-in-the-loop feedback mechanism (`provide_feedback`) for manual approval/rejection of autonomous tasks.
- **UI Integration**: Wired `activeSwarmPeers` and `lastCheckpointId` to `AutonomousMonitoringPanel.qml` through `FileModel`.
- **Integration Test Stability**: All 22+ tests passing. Resolved DB lock contention and race conditions in file watchers by introducing stabilization delays and explicit DB pathing.

## Technical Notes
- **DB Isolation**: Each test node now has its own `nodeN_sync.db` to prevent cross-contamination.
- **Watcher Debounce**: CI environments required increasing watcher debounce to 500ms-1000ms to avoid overlapping event bursts during rapid file writes.
- **Swarm Peer Discovery**: Currently simulated via `SwarmHub`. Real-world UDP/mDNS discovery is scaffolded in `swarm_engine.cpp` but not yet fully activated in the simulation layer.

## Outstanding Items (ROADMAP.md)
- [ ] Transition `bob_shell` to use the new `AutonomousMonitoringPanel`.
- [ ] Implement `io_uring` support for zero-copy file operations in `enhanced_fileops.cpp`.
- [ ] Full BTK native surface refactoring for the Explorer UI.

## Environment & Build
- **Branch**: `main`
- **Version**: `6.3.4`
- **Build Status**: Verified passing with `cmake -DFO_BUILD_GUI=OFF -DFO_BUILD_OMNI=OFF ..` and `./tests/fo_tests`.

## 2026-11-03 autonomous session (v6.4.0)
- Unified dashboard + 13 API bridges + tray + script archive; server smoke-tested (health/unified/ocr-error/quit). Build impossible on this host (no MSVC/cmake/Qt). Pushed to origin/main. Next model: run scripts/dev_all.bat on a real dev host, then close TODO build-host.
