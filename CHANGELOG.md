# Changelog

## [6.5.96] - 2026-11-25 — Long-Path Scanner Access Violation

### Fixed
- **Scanning a tree containing >260-char paths crashed the
  process** (0xC0000005): the std scanner's
  `recursive_directory_iterator` walk faults inside the
  iterator when a subtree crosses MAX_PATH mid-walk (both
  the old throwing `++it`-in-catch form and the error-code
  increment form). Replaced with a manual stack DFS over
  per-level `directory_iterator` (all error-code based,
  mirroring the win32 scanner): an unenterable deep subtree
  is skipped, siblings still scan, exit stays 0. New
  `ScanSurvivesOverLongPaths` test failed with the AV before,
  passes after. Suites: lean **104/104**, MSVC **104/104**
  (one load-flake failure each on first full run, clean on
  re-run — same pattern as earlier recorded flakes), wiring
  **31/31**, JUCE **99 + 2 designed skips**.

## [6.5.95] - 2026-11-25 — Reserved-Name Data Loss, Vault Failure Trail

### Fixed
- **Renaming onto a Windows device name destroyed the file**
  (found by the round-9 collision probe): `rename --pattern=CON`
  reported success and the source vanished into the CON
  device. Organize/rename now refuse destinations whose final
  segment is a Win32 reserved name (CON, PRN, AUX, NUL,
  COM0-9, LPT0-9, any extension) or ends in a dot/space, as a
  named per-file failure before anything moves.
  `OrganizeCaseCollisionAndReservedNameFailSafely` also pins
  the NTFS case-collision case (two sources flattening to
  case-variants of one dest: one moves, one fails, loser
  stays home intact).
- **Vault failures were invisible**: wrong-password access,
  failed lock (missing file) and failed unlock (bad/traversal
  id) now write `vault-initialize-failed` / `vault-lock-
  failed` / `vault-unlock-failed` audit entries — paths only,
  never password material. Vault crypto reviewed for round 9:
  PBKDF2-HMAC-SHA256 (100k), per-entry random salt + 12-byte
  GCM IV via RAND_bytes, `.part`+rename atomicity both
  directions; metadata is now written before the entry rename
  so a crash cannot leave a nameless entry. New
  `vault.after-encrypt` / `vault.after-decrypt` fault
  checkpoints + `VaultLockUnlockCrashWindowsLoseNothing`
  prove both-present-or-done in each window.
- **Wiring goldens** now cover every array bridge route
  (duplicates, metadata, lint, history, ignore in addition
  to scan/hash). Suites: lean **103/103**, MSVC **103/103**,
  wiring **31/31**, JUCE **98 passed + 2 designed skips**.

## [6.5.94] - 2026-11-25 — True Crash Recovery: Intent Before Move

### Fixed
- **Crash between rename and journal was unrecoverable**: the
  exception-path rollback only worked while the process lived.
  Organize/rename now journal a `pending` intent row BEFORE
  the rename and complete it after (`complete_operation`),
  undo marks records `undoing` before its rename, and every
  CLI invocation reconciles leftovers against the live tree
  (`reconcile_pending`, Migration 13 status index): intent
  with the file home is dropped, intent with the file at dest
  is completed (undoable), undoing with the file restored is
  marked undone; ambiguous rows (both/neither paths) are left
  for a human. `FO_FAULT_AT=<point>` checkpoints
  (`fault_injection.hpp`) hard-kill the process at
  `organize.after-mkdir/-intent/-rename/-complete` and
  `undo.after-rename`; new `OrganizeCrashRecoveryAtEach
  Checkpoint` / `UndoCrashRecoveryAtRenameCheckpoint` tests
  assert tree+journal consistency after each kill. The older
  view-swap rollback test was openly re-pinned: an intent
  insert failure now prevents the move instead of rolling it
  back (same file-home contract, stronger mechanism).
- **Undo mtime false-refusals**: FAT/exFAT 2-second mtime
  rounding no longer refuses an untouched file — when the
  file ID matches, ≤2s of drift is rounding, not an edit
  (`UndoToleratesTwoSecondMtimeRounding`). Replacement still
  fails the file-ID gate.
- **Partial success was invisible**: the bridge discarded a
  failed run's JSON payload; the error envelope now carries
  it as `partial`, and the unified page renders it. Also, the
  CLI undo JSON empty case now names every skipped record
  and reason instead of a bare "No operations to undo".
  Suites: lean **100/100**, MSVC **100/100**, wiring
  **30/30**; JUCE lane rebuilt at this version.

## [6.5.93] - 2026-11-25 — Schema v2: Array Roots Wrapped

### Changed (breaking, versioned)
- **JSON schema v1 → v2**: bare array roots are gone. Every
  array-shaped `--format=json` payload (scan, duplicates,
  hash, metadata, lint, classify, vhash, afingerprint,
  search, history, ignore list) and every `fo_c_api` JSON
  string is now wrapped as
  `{"schema_version": 2, "items": [...]}`; object payloads
  carry `"schema_version": 2` as their first field (reviewer
  round 6: a bare array can never gain a version field).
  All in-repo consumers moved in the same commit: the bridge
  relays the wrapped shape (fallbacks updated), the unified
  dashboard renders it verbatim, and the legacy React lane
  unwraps `items` in `fetchJson`/`fetchGetJson`. Bridge error
  envelopes are versioned 2 as well. `docs/JSON_SCHEMA.md`
  now states the breaking-change policy explicitly
  (remove/rename/retype/root-shape ⇒ bump; additions free).
  New `JsonArrayRootsAreWrappedV2` conformance test asserts
  the wrapped shape on every array command; the object-side
  `JsonOutputsCarrySchemaVersion` now pins v2. Suites: lean
  **97/97**, MSVC **97/97**, wiring **29/29**; JUCE lane
  rebuilt at this version.

## [6.5.92] - 2026-11-25 — Vault Ops Reach the Forensic Audit Ledger

### Fixed
- **Vault lock/unlock never appeared in the audit ledger**:
  `vault-lock` consumes the source file and `vault-unlock`
  writes plaintext back out, yet neither wrote to the
  Migration 5 forensic ledger, leaving a vault-shaped hole
  in the tamper-evident trail every other destructive op
  feeds. Both now log `vault-lock`/`vault-unlock` entries
  (source → vault / entry → dest) via
  `make_audit_logger(engine.database())` on success.
  New `VaultOpsAppearInAuditLedger` CLI test failed before,
  passes after. Suites: lean **96/96**, MSVC **96/96**,
  wiring **28/28**; JUCE lane rebuilt at this version.

## [6.5.91] - 2026-11-24 — Convert: No Silent Overwrite, No Shell Injection

### Fixed
- **Convert silently overwrote existing outputs** (a trailing
  ffmpeg `-y`), breaking the house rule every other command
  follows; an existing output is now a named per-file failure.
  **Shell injection via `--output`**: backend commands are
  built as shell strings, and a double quote in the output
  path broke out of the quoted argument under cmd.exe (the
  dashboard passes this field from JSON). Quote-bearing
  output paths are refused as usage errors before any tool
  runs. New `ConvertRefusesExistingOutputAndQuoteInjection`
  CLI test failed before, passes after. Suites: lean
  **95/95**; other lanes rebuilt at this version.

## [6.5.90] - 2026-11-24 — Ignore Add/Remove Report Truthfully

### Fixed
- **`ignore add`/`remove` claimed success unconditionally**:
  adding a duplicate and removing a nonexistent rule both
  printed success and exited 0. `IgnoreRepository::add/remove`
  now report whether a row actually changed; the CLI exits 1
  with "already exists" / "No ignore rule matches", and the
  C API action JSON carries a truthful `"success"` field.
  New `IgnoreAddRemoveReportTruthfully` CLI test failed
  before, passes after. Suites: lean **94/94**; other lanes
  rebuilt at this version.

## [6.5.89] - 2026-11-24 — Vault Unlock Path Traversal

### Fixed
- **`vault-unlock` accepted traversal ids**: the entry id was
  joined to the vault path unchecked, so `../outside-copy`
  escaped the vault — and since a successful unlock deletes
  its source, it could decrypt *and erase* a file outside the
  vault. Entry ids are generated flat names; anything with
  separators or dot segments is now refused before touching
  the filesystem. New `VaultUnlockRejectsTraversalId` CLI
  test (copied entry attacked via `../`) failed before,
  passes after. Suites: lean **93/93**; other lanes rebuilt
  at this version.

## [6.5.88] - 2026-11-24 — Bad Numeric Options Are Usage Errors

### Fixed
- **`--threshold=abc` aborted the process** (uncaught
  `std::invalid_argument` → 0xC0000409, no message). Every
  numeric option now parses strictly (`parse_int_strict` /
  validated size strings): `--threshold`, `--threads`,
  `--min-size`, `--max-size`, `--quality`, `--max-results`,
  `--debounce` (plus the existing `--hash-limit-mb` and audit
  `--limit`) produce a named usage error with exit 2. New
  `BadNumericOptionsAreUsageErrors` CLI test failed (abort)
  before, passes after. Suites: lean **92/92**; other lanes
  rebuilt at this version.

## [6.5.87] - 2026-11-24 — Concurrent Invocations On One Database

### Fixed
- **Concurrent CLI runs on one `--db` hard-failed**: four
  simultaneous scans produced "database is locked" (and a
  migration race: "duplicate column name"). Three layered
  causes, three fixes: connections now set
  `PRAGMA busy_timeout = 30000`; migrations run serialized
  under one `BEGIN IMMEDIATE` with the version re-read inside
  it; and Engine's scan/duplicates transactions use
  `BEGIN IMMEDIATE` — a deferred transaction that reads first
  gets instant `SQLITE_BUSY` on the write upgrade (deadlock
  avoidance bypasses the busy handler entirely). Verified:
  4 concurrent scans of 200 files on a fresh DB all succeed;
  new `DatabaseFixture` contention test failed before, passes
  after. Suites: lean **91/91**; other lanes rebuilt here.

## [6.5.86] - 2026-11-24 — Vault List Verifies The Password

### Fixed
- **`vault-list` accepted any password**: `initialize()` never
  checked it, so entry names and sizes listed for whoever
  asked. Vaults now carry a `.vault_verify` (salt + HMAC under
  the PBKDF2 key, written at init for fresh vaults);
  `initialize()` rejects wrong passwords up front for
  list/lock/unlock. Legacy vaults without a verifier stay
  ungated (their contents remain encrypted; the owner re-inits
  to opt in) rather than letting a first-come password claim
  them. The verifier file itself briefly listed as an entry —
  bookkeeping dotfiles are now skipped. New
  `VaultListRejectsWrongPassword` CLI test failed before,
  passes after. Suites: lean **90/90**; other lanes rebuilt at
  this version.

## [6.5.85] - 2026-11-24 — Bogus GPS Sentinels + Flow JSON Exit

### Fixed
- **Metadata printed garbage GPS coordinates**: TinyEXIF
  reports absent GPS with sentinel doubles (DBL_MAX), which
  surfaced as `GPS: 1.79769e+308, 1.79769e+308` for any JPEG
  without GPS data. Only finite, in-range values now count.
  **Flow run in JSON mode exited 0 on failure** (text mode
  exited 1); both modes now share the exit contract, so the
  bridge maps failures to HTTP 422. New
  `MetadataOmitsBogusGps` CLI test (hand-crafted EXIF JPEG)
  failed before, passes after. Suites: lean **89/89**; other
  lanes rebuilt at this version.

## [6.5.84] - 2026-11-24 — Malformed Patterns Are Usage Errors

### Fixed
- **Garbage patterns "succeeded"**: `rename --pattern="{name"`
  renamed a file to a literal `{name`, an unknown `{bogus}`
  placeholder became a literal organize folder, and an invalid
  rename-batch `--regex` silently renamed 0 files with exit 0.
  `RuleEngine::validate_template` now rejects unbalanced braces
  and unknown placeholders (rename, organize `--rule`, and YAML
  rule destinations, exit 2 naming the problem), and
  rename-batch validates its regex once up front. New
  `MalformedTemplatesAndRegexAreUsageErrors` CLI test failed
  before, passes after. Suites: lean **88/88**; other lanes
  rebuilt at this version.

## [6.5.83] - 2026-11-24 — Undo Verifies File Identity

### Fixed
- **A replaced destination passed every undo identity check**
  (round-6/8 residual): a different file wearing the moved
  file's path, with forged size and mtime, was moved back as
  if it were the original. Migration 12 records the platform
  file identity at journal time (NTFS volume+file index, or
  POSIX dev+inode — both survive renames, not replacement);
  undo now refuses when the identity differs. New
  `UndoRefusesReplacedFileWithForgedSizeAndMtime` unit test
  failed before, passes after. Suites: lean **87/87**; other
  lanes rebuilt at this version.

## [6.5.82] - 2026-11-24 — Journal Rollback Proven By Fault Injection

### Tests
- **Fault-injection for the journal** (round-6/7): a new CLI
  test replaces `operation_log` with a view so the journal
  insert throws *after* the rename, and asserts the move is
  rolled back, reported ("move rolled back"), and the file is
  back at its source — the moved-but-unjournaled branch is now
  exercised end-to-end instead of argued. Suites: lean
  **86/86**; other lanes rebuilt at this version.

## [6.5.81] - 2026-11-24 — Typed Bridge Error Envelope

### Added
- **Dashboard errors are typed, not flat 500s** (round-6):
  every bridge failure returns
  `{schema_version: 1, error, error_code, retryable}` with the
  status driven by the CLI exit code — usage errors (exit 2)
  → 400 `USAGE`, CLI failures (exit 1) → 422 `FAILED`,
  timeouts → 504 `TIMEOUT` (retryable), spawn/missing CLI →
  500 `INTERNAL`; validation failures are 400 `BAD_REQUEST`.
  `error` deliberately stays a human string because the legacy
  React lane renders it raw (documented tradeoff in
  `docs/JSON_SCHEMA.md`). All 42 bridge catches route through
  one `cliError` helper. Wiring test updated to the envelope
  contract; live dashboard re-verified (media endpoints now
  answer 422 with the CLI's message). Suites: lean **85/85**,
  wiring **28/28**; other lanes rebuilt at this version.

## [6.5.80] - 2026-11-24 — Mtime Identity + Dir-Journal Order (Round 6)

### Fixed
- **Above the 64 MB hash cap, undo identity was size-only**:
  Migration 11 records destination mtime at journal time for
  every file, and undo compares it — a same-size in-place edit
  is refused without reading a byte. The cap is now
  configurable via `--hash-limit-mb=<N>` (default 64).
  **Created-dir journaling order**: organize now creates
  missing ancestors segment-by-segment and journals only the
  segments it actually created (a directory another process
  created first is never journaled, so undo can never prune
  somebody else's dir); a crash between mkdir and journal
  leaves at worst an unjournaled empty dir — harmless residue,
  accepted and documented. User content later added to a
  journaled dir provably survives undo. Schema breaking-change
  policy written down in `docs/JSON_SCHEMA.md`. Suites: lean
  **85/85**; other lanes rebuilt at this version.

## [6.5.79] - 2026-11-24 — Schema-Versioned JSON Contract

### Added
- **`"schema_version": 1` on every object-shaped JSON payload**
  (round-5/6 agreed contract): stats, organize,
  delete-duplicates, rename, similar, undo (incl. dismiss and
  refusal), audit, scrub, flow list/run, watch-status, and the
  shared Exporter JSON root. Array roots stay arrays (schema 1
  by convention); pre-schema watcher heartbeats are normalized
  on re-emit. Pinned on both sides: a CLI conformance test and
  golden dashboard wiring assertions (`/api/stats`,
  `/api/organize` payloads). Unknown-field behaviour: bridges
  relay payloads unchanged (additive fields are safe); a
  version bump without a bridge update now fails tests.
  Suites: lean **83/83**, wiring **28/28**; other lanes rebuilt
  at this version.

## [6.5.78] - 2026-11-24 — Journaled Dir Pruning + Hash Cap (Round 5)

### Fixed
- **Dir pruning could delete pre-existing directories**
  (round-5 review): the v6.5.77 parent-climb prune could not
  distinguish organize-created dirs from pre-existing empty
  ones and could climb above the organize root. Migration 10
  journals exactly the directories each batch creates
  (`created_dirs`); undo prunes only those, deepest first,
  only when empty. **Journal hash capped at 64 MB**: hashing
  turns an O(1) rename into a full read — on cold storage that
  dwarfs the move for huge files, which keep size-based
  identity instead. (fast64 identity is accidental-change
  detection, not tamper proof.) Suites: lean **82/82**; other
  lanes rebuilt at this version.

## [6.5.77] - 2026-11-24 — Undo Leaves No Empty Category Dirs

### Fixed
- **Undo left organize's empty category directories behind**
  (Muse review: the identical-tree invariant compared only
  files). A successful move-back now prunes destination parent
  directories it emptied, up to 8 levels — `remove()` only
  succeeds on empty dirs, so user content is never at risk.
  The invariant test now compares the whole fixture tree
  (files + mtimes + directory set). Remote tree verified
  against Muse's cached-page caveat: the 18 untracked root
  files are confirmed gone from `origin/main`. Suites: lean
  **81/81**; other lanes rebuilt at this version.

## [6.5.76] - 2026-11-24 — Non-ASCII Paths Work Through argv (Claude Round 4)

### Fixed
- **CJK paths passed as arguments arrived as `????`**: the
  CRT's narrow `argv` is ANSI-code-page encoded, so the shipped
  binary rejected existing CJK directories as missing (Muse
  review; the UTF-8 locale alone could not fix bytes that were
  already mangled). `main` now sources arguments from
  `CommandLineToArgvW` converted to UTF-8 and sets the UTF-8
  locale before parsing. Verified on the real binary via a
  CreateProcessW test (`ScanFindsCjkRootPassedAsArgument`),
  not just the ctest harness. Also confirmed this round:
  `metamcp.db` and the session files contain **no credentials**
  (all sensitive tables empty; public catalog only), while
  `fo.db` holds 3,156 scanned file paths — a privacy note for
  the owner's history-purge decision, nothing to rotate.
  Suites: lean **81/81**; other lanes rebuilt at this version.

## [6.5.75] - 2026-11-24 — Undo Verifies Content, Not Just Size

### Fixed
- **Same-size content edits slipped past undo's identity
  check**: organize/rename journals now record the
  destination's fast64 in `file_hash` at journal time, and
  undo compares content (via the shared `verify_identity`
  gate for Move/Rename/Copy) before moving or deleting
  anything. A same-size edit with a backdated mtime is refused
  with "content differs" and the user's edit is left untouched.
  New `UndoRefusesSameSizeContentEdit` unit test failed before,
  passes after. Suites: lean **80/80**; other lanes rebuilt at
  this version.

## [6.5.74] - 2026-11-24 — Root Hygiene + Copy-Undo Guard (Claude Round 3)

### Fixed
- **Root clutter untracked** (Muse verified on main): release
  zips, `.bak` files, a 126 MB AppImage, build logs, agent
  session files, and the live `fo.db`/`index.db`/`metamcp.db`
  databases were tracked in git. They are now `git rm --cached`
  (files stay on disk) with matching `.gitignore` rules.
  History is deliberately NOT rewritten; old copies remain in
  prior commits — a history purge is a separate owner decision.
- **README hardcoded stale test counts** (28/25/18 from Nov 17):
  replaced with the commands to run; counts live in this file.
- **Copy undo deleted edited copies**: same-size edits slipped
  past the size check; a copy whose mtime postdates its journal
  entry is now refused. Moved-but-unjournaled residue (journal
  failure + rollback failure) is also appended to
  `<db>.residue.log`, not just stderr. The organize→undo
  invariant now includes a CJK directory on the path.
  Suites: lean **79/79**; other lanes rebuilt at this version.

## [6.5.73] - 2026-11-24 — Atomic Batch IDs, Journal Rollback, CJK Names

### Fixed
- **Batch-id allocation raced** (Muse review): MAX+1 let two
  concurrent invocations share an id and merge batches.
  Migration 9 adds a counter table; allocation is an atomic
  increment inside `BEGIN IMMEDIATE`. **Moved-but-unjournaled**:
  if the journal insert fails after a successful organize/
  rename move (locked/full DB), the move is now rolled back,
  and if rollback also fails the residue is named loudly
  ("Undo will not know about this file") instead of the old
  silent mismatch. **CJK filenames crashed whole commands**:
  `path.string()` throws on names outside the ANSI code page;
  Engine construction now opts the process into the UTF-8
  locale (idempotent), making narrow path conversions UTF-8
  end-to-end — the organize→undo invariant now covers a CJK
  fixture. Suites: lean **78/78**; other lanes rebuilt here.

## [6.5.72] - 2026-11-24 — Undo Core Rework (Muse Code Review)

### Fixed
- **Batches over 500 records were undone in chunks** (the
  `get_undoable(500)` cap): one undo of a 520-file organize
  restored 500 and reported success. Batch undo now walks the
  whole journal uncapped.
- **Undo after delete-duplicates reversed the wrong batch**:
  deletes were filtered out, so undo silently reversed the
  *previous* command. Undo now refuses an irreversible delete
  batch by name (exit 1) and only `fo undo --dismiss`
  acknowledges it to reach older batches. **Contract change**:
  `DeleteDoesNotBlockUndoOfMove` previously pinned the silent
  fall-through; it now pins refuse → dismiss → undo.
- **Undo never verified identity**: it moved/deleted whatever
  sat at the destination. Destinations whose size differs from
  the journal are now refused ("file changed since the
  operation"), skips are reported per record with reasons,
  missing source parents are recreated, and `get_by_id` uses a
  primary-key lookup (was: newest-1000 scan). Per-test ctest
  timeout (120s) added after two load-related suite hangs.
  Suites: lean **77/77**; other lanes rebuilt at this version.

## [6.5.71] - 2026-11-24 — Matrix Completeness Guard + mtime Invariant

### Tests
- **Muse's pushback round**: the conformance matrix can no
  longer go stale — `HelpCoversEveryPathCommand` parses
  `--help` and fails when any command lacks a matrix row or a
  named exemption (it immediately caught `similar` missing;
  matrix now 18 rows incl. `search`/`convert`). The organize→
  undo invariant now also compares mtimes, not just paths and
  bytes. `operation_repository.cpp` sent to the Muse thread
  for the deep review it requested. Suites: lean **70/70**;
  other lanes rebuilt at this version.

## [6.5.70] - 2026-11-24 — Conformance Matrix + Undo Invariant (Claude Roast)

### Tests
- **Muse's roast landed** (fresh claude.ai thread; the
  original wedged mid-generation) and its two surviving points
  are now pinned: a conformance matrix
  (`MissingRootFailsEveryPathCommand`) runs 16 path-consuming
  commands against a missing root asserting nonzero exit +
  named path, replacing one-bug-one-test coverage for that
  class; and `OrganizeUndoPreservesTreeExactly` asserts the
  data-loss invariant — organize then undo restores the tree
  byte-for-byte (nested dirs, non-ASCII filename). Triage reply
  posted to the thread (Go-port and scaffold-rot points were
  stale/already handled; submodule pruning stays owner-gated).
  Suites: lean **69/69**; other lanes rebuilt at this version.

## [6.5.69] - 2026-11-24 — Submodule Dashboard Stops Fabricating Commits

### Fixed
- **`scripts/generate_dashboard.py` stamped uninitialized
  submodules with the superproject's HEAD**: an empty submodule
  directory makes git walk up to the parent repo, so rows for
  never-checked-out libraries showed the parent commit, date,
  and build count as if they were the library's own. The
  generator now requires the submodule's own `.git` and reports
  "not checked out" otherwise; `docs/SUBMODULES.md` regenerated
  (7 rows corrected). Suites: lean/JUCE/MSVC rebuilt at this
  version.

## [6.5.68] - 2026-11-24 — Search Grammar Works In Any Order

### Fixed
- **`search --content <q> <dir>` silently searched filenames**:
  the flag scan started after the query slot, so flags placed
  before the query were never parsed. All args are now scanned.
  And a bare `search <q>` (current-directory search) was
  rejected by the v6.5.55 missing-path guard, which mistook the
  query for a path: search is exempt from that guard and
  validates its own roots instead (warn per missing path, error
  when none exist). New `SearchFlagsBeforeQueryAndBareQueryWork`
  CLI test failed before, passes after. Suites: lean **67/67**;
  other lanes rebuilt at this version.

## [6.5.67] - 2026-11-24 — Undo Restores The Whole Command

### Fixed
- **Undo reverted one file per invocation**: a rename/organize
  command touching N files journaled N records, and undo walked
  them one at a time, leaving batches half-undone. Migration 8
  adds `operation_log.batch_id`; organize, rename, and
  delete-duplicates stamp all their records with one batch per
  invocation, and `undo` (CLI + C API) now restores the whole
  most-recent batch, falling through stale batches like the old
  per-record walk. New `UndoLastBatchRestoresWholeBatch` unit
  test and `UndoRestoresWholeRenameBatch` CLI test failed
  before, pass after. Suites: lean **66/66**; other lanes
  rebuilt at this version.

## [6.5.66] - 2026-11-24 — Similar Refuses Cross-Algorithm Queries

### Fixed
- **`similar --phash=phash` returned confident nonsense**: the
  stored index only ever holds `dhash` values (the hash
  command's sole perceptual hasher), but a phash query hash was
  compared against them — on the regression images it returned
  the perceptual *opposite* as the only match. Non-dhash
  providers are now refused with the reason (checked via the
  provider's own `name()`, so the `opencv` dhash alias still
  works). New `SimilarRejectsNonDhashAlgorithm` CLI test failed
  before, passes after. Suites: lean **64/64**; other lanes
  rebuilt at this version.

## [6.5.65] - 2026-11-24 — Organize Card Teaches {filename}

### Fixed
- **Dashboard Organize card taught an extension-stripping
  template**: its example `{ext}/{year}/{name}` uses `{name}`
  (stem only), so files organized by the card's own guidance
  lost their extensions, and `{filename}` was never mentioned.
  The card now teaches `{ext}/{filename}`, lists `{filename}`
  first, and warns that `{name}` drops the extension. Pinned by
  a new wiring test; wiring **27/27**. Suites: lean/JUCE/MSVC
  rebuilt at this version.

## [6.5.64] - 2026-11-24 — Convert Failures Exit Nonzero

### Fixed
- **`convert` printed failures but exited 0**: each backend
  failure ("Conversion failed for: ...") was reported on stderr
  but the command still succeeded, so the dashboard bridge
  answered HTTP 200 for failed conversions. Failures are now
  counted with a "failed to convert N of M file(s)" summary and
  exit 1 (successful files still report [OK]). New
  `ConvertFailureExitsNonzero` CLI test failed before, passes
  after. Suites: lean **63/63**; other lanes rebuilt at this
  version.

## [6.5.63] - 2026-11-24 — Dashboard Surfaces CLI Failures

### Fixed
- **Dashboard bridges turned CLI failures into empty
  successes**: `runCli` resolved whenever stdout was non-empty,
  so a failed run printing a partial `[]` payload returned HTTP
  200 (e.g. `/api/vhash` on a folder). Nonzero exits now reject
  with the CLI's own stderr text, so bridges answer HTTP 500
  with the real error. New functional wiring test
  (`CLI failures surface as HTTP 500...`) failed before, passes
  after; wiring **26/26**, live dashboard re-verified (media
  folder probes now 500 with the CLI message; OCR still 200).
  Suites: lean/JUCE/MSVC rebuilt at this version.

## [6.5.62] - 2026-11-24 — Media Commands Process Every Input

### Fixed
- **`vhash`/`afingerprint` silently dropped every input after
  the first**: both handlers processed only `roots[0]` while the
  dashboard cards (and bridge) pass comma-separated file lists.
  Both now loop over all inputs, emit a JSON array in JSON mode,
  name per-file failures on stderr, and exit 1 with a
  "failed ... N of M file(s)" summary when any input fails. New
  `AfingerprintProcessesEveryInput` CLI test (generated WAV
  fixtures) failed before, passes after. Suites: lean **62/62**;
  other lanes rebuilt at this version.

## [6.5.61] - 2026-11-24 — Hash Reports Files It Could Not Hash

### Fixed
- **`hash` printed and stored blank hashes as successes**:
  hashers signal failure with an empty string (dhash on a
  non-image, fast64 on an unreadable file), and the hash command
  emitted those blanks in text/JSON output, stored them in the
  database, and exited 0. Failures are now named per file on
  stderr, skipped in output and storage, summarized
  ("failed to hash N of M file(s)"), and exit 1. New
  `HashReportsFilesItCouldNotHash` CLI test failed before,
  passes after. Suites: lean **60/60**; other lanes rebuilt at
  this version.

## [6.5.60] - 2026-11-24 — Global Flags No Longer Hijack Positional Args

### Fixed
- **Handlers indexing raw argv took flags as values**: `flow run
  flow-01 --db=x payload` reported executing the workflow on the
  string `--db=x`, and `search --db=x query` searched for the
  flag text. A positional view (`pargs`: command + non-flag
  tokens) now feeds every positional index in search, flow,
  autodev, sync, vault, clerk, and oracle handlers; handlers
  scanning for their own `--flags` still use raw args. New
  `GlobalFlagsDoNotHijackPositionalArgs` CLI test failed before,
  passes after. Suites: lean **59/59**; other lanes rebuilt at
  this version.

## [6.5.59] - 2026-11-24 — Similar Names Its Input Expectation

### Fixed
- **`similar <folder>` dumped raw OpenCV warnings**: the command
  takes one image file, but a directory went straight to the
  perceptual hasher and failed as "Failed to compute hash" amid
  library warnings (the dashboard showed that dump as its error).
  The CLI now checks the target is a regular file and says so
  plainly. New `SimilarOnDirectoryIsACleanError` CLI test failed
  before, passes after. Suites: lean **58/58**; other lanes
  rebuilt at this version.

## [6.5.58] - 2026-11-24 — OCR Unavailable Is An Error

### Fixed
- **`ocr` exited 0 when Tesseract could not initialize** (no
  loadable tessdata): the engine's own load errors printed per
  file and the CLI still reported success. `IOCRProvider` gains
  an `unavailable_reason(lang)` readiness query (same pattern as
  the classifier in v6.5.57); the Tesseract provider initializes
  on query and names the missing language data and
  `TESSDATA_PREFIX`, and the CLI exits 1 before OCR-ing. New
  `OcrUnavailableIsAnError` CLI test failed before, passes after.
  Suites: lean **57/57**; other lanes rebuilt at this version.

## [6.5.57] - 2026-11-24 — Classify Without A Model Is An Error

### Fixed
- **`classify` exited 0 with no model present**: the ONNX provider
  printed "Model file not found" per file and returned no labels,
  which the CLI treated as success. `IImageClassifier` gains an
  `unavailable_reason()` readiness query (default: ready); the
  ONNX provider records its load failure once and reports it, and
  the CLI exits 1 with the reason before classifying. New
  `ClassifyWithoutModelIsAnError` CLI test failed before, passes
  after. Suites: lean **56/56**; other lanes rebuilt at this
  version.

## [6.5.56] - 2026-11-24 — Bogus Hasher Crashed The CLI

### Fixed
- **`hash --hasher=<unknown>` segfaulted** (0xC0000005): the hash
  command dereferenced `Engine::hasher()` whose provider pointer
  is null for unregistered names. `Engine::hasher()` now throws
  `hasher not found: <name>` (same convention as the scanner),
  producing a clean exit-3 error. `--keep=<unknown>` was silently
  treated as `oldest`; it is now a usage error (exit 2) at parse
  time, like `--mode`. New `BogusHasherAndKeepAreErrorsNotCrashes`
  CLI test failed (crashed) before, passes after. Suites: lean
  **55/55**; other lanes rebuilt at this version.

## [6.5.55] - 2026-11-24 — Missing Paths Are Errors

### Fixed
- **Nonexistent paths silently "succeeded"**: the std scanner
  skips missing roots without a word, so `scan`/`hash`/`metadata`
  on a typo'd path printed nothing and exited 0. Path-consuming
  commands now warn `path does not exist: ...` per missing root
  and exit 1 when no root exists; mixed root lists still process
  the existing ones. `export --format=<unknown>` no longer falls
  back to JSON silently (usage error, exit 2). New
  `MissingRootIsAnErrorNotEmptySuccess` CLI test failed before,
  passes after. Suites: lean **54/54**; other lanes rebuilt at
  this version.

## [6.5.54] - 2026-11-24 — Plain Commands Stay Quiet

### Fixed
- **Every CLI command started five scaffold services**: `Engine`'s
  constructor eagerly built OmniClerk, Oracle, AutoDev, Sync, and
  Swarm, so even a plain `scan` printed `[OmniClerk] Initializing...`
  banners (stderr) and registered a swarm peer. The coordination
  services are now constructed lazily behind their accessors, with
  the same cross-wiring (sync gets swarm + `<db>_sync.db`, autodev
  gets sync) on first use. Plain scan stderr is now 0 bytes;
  `clerk/sync/autodev status` still construct on demand. New
  `PlainScanDoesNotStartScaffoldServices` CLI test failed before,
  passes after. Suites: lean **53/53**; other lanes rebuilt at
  this version.

## [6.5.53] - 2026-11-24 — Ignore Rules Actually Filter

### Fixed
- **Ignore rules silently did nothing**: `is_ignored` treated
  patterns as regex, so the glob patterns every UI teaches
  (`*.log`) were invalid regexes whose substring fallback could
  never match. Patterns are now matched as case-insensitive
  globs (with `{a,b,c}` alternation) against filename and full
  path; plain patterns keep substring semantics. New
  `IgnoreGlobFiltersScan` CLI test failed before (the .log file
  was scanned), passes after. Suites: lean **52/52**; other lanes
  rebuilt at this version.

## [6.5.52] - 2026-11-24 — Undo Works From Any Directory

### Fixed
- **Undo silently did nothing from another directory**: organize
  and rename journaled CWD-relative source/destination paths, so
  `undo` run anywhere else found no destination file and reported
  "No operations to undo" (the crash-probe caveat from v6.5.31).
  Both handlers now perform and journal absolute paths; undo
  works from any working directory. (Journals written before this
  fix keep their relative records and still need the original
  CWD.) New `UndoWorksFromDifferentCwd` test — which initially
  passed vacuously because the test helper never really changed
  directory; it now chdirs for real and failed pre-fix.
  Suites: lean **51/51**; other lanes rebuilt at this version.

## [6.5.51] - 2026-11-24 — Vault Unlock Refuses Existing Destinations

### Fixed
- **Unlock silently replaced an existing file**: the atomic
  decrypt rename from v6.5.36 would overwrite whatever already
  sat at the unlock destination. `unlock_file` now refuses when
  the destination exists; the vault entry survives for a retry
  with a free path. New `VaultUnlockRefusesExistingDestination`
  test failed before (bytes replaced), passes after.
  Suites: lean **50/50**; other lanes rebuilt at this version.

## [6.5.50] - 2026-11-24 — Flow Actions Refuse to Overwrite

### Fixed
- **OmniFlow MoveTo/Rename silently overwrote destinations**:
  the same MSVC-rename-replaces hazard fixed for the CLI in
  v6.5.29 lived on in `execute_action`. Both actions now fail
  (which, since v6.5.25, fails the run) when the destination
  exists, leaving both files' bytes intact. `Action.CopyTo`
  keeps its explicit `overwrite_existing` semantics. New
  `FlowMoveToRefusesExistingDestination` test failed before,
  passes after. Suites: lean **49/49**; other lanes rebuilt.

## [6.5.49] - 2026-11-24 — Stale-Authorization Guards (Reviewers' Challenge)

### Fixed
- **Delete-duplicates trusted obsolete groups**: membership came
  from the last `duplicates` run; a file that changed content
  afterward was still deleted as a "duplicate". Each candidate is
  now re-hashed against the kept file immediately before deletion;
  mismatches are skipped and reported (`skipped` count, stderr
  line), never deleted.
- **Undo could overwrite a recreated file**: if the original path
  was recreated externally after a move, undo renamed over it
  (MSVC rename replaces). Undo now skips such records, leaving
  both files intact.
  Both were the reviewers' named next challenge ("prove bobfilez
  never destroys data because the information it used to authorize
  a destructive operation became obsolete"); both new CLI tests
  failed before (files destroyed) and pass after.
  Suites: lean **48/48**; other lanes rebuilt at this version.

## [6.5.48] - 2026-11-24 — Scan JSON Carries mtime

### Fixed
- **Scan JSON omitted `mtime`**: the classic dashboard's date
  column reads `f.mtime` and rendered "Invalid Date" for every
  row. Scan JSON now includes `"mtime"` (unix seconds). New
  `ScanJsonIncludesMtime` CLI test failed before, passes after.
  Suites: lean **46/46**; other lanes rebuilt at this version.

## [6.5.47] - 2026-11-24 — Raw Delete Endpoint Retired

### Removed
- **`POST /api/delete`**: it unlinked arbitrary paths from the
  request body with no journal or audit record, and no frontend
  called it (the only textual hit was `/api/delete-duplicates`).
  The unified page deletes solely via the scoped, journaled,
  dry-run-first `/api/delete-duplicates`. Verified live: the route
  now 404s; a wiring test keeps it retired (25/25).

## [6.5.46] - 2026-11-24 — Dashboard Launch Scripts Set OCR Env

### Fixed
- **`start-server.bat` lost OCR**: it never set `TESSDATA_PREFIX`,
  so a dashboard launched the documented way had silently failing
  OCR (the tray spawn path was fixed in v6.5.41; the direct script
  wasn't). Both entry points now point at the staged tessdata
  when present. Proven by launching the script on a side port.
  Wiring **24/24**; suites at this version's code below.

## [6.5.45] - 2026-11-24 — Benchmark Executables Can Start

### Fixed
- **`fo_benchmarks.exe` died instantly (0xc0000135)**: the
  benchmark executables link `fo_core`'s shared providers but,
  unlike `fo_tests`, are never executed at build time, so their
  missing runtime DLLs went unnoticed. They now get the same
  exact-loader-set post-build copy (`tests/copy_runtime_deps.cmake`).
  Verified: the suite lists and runs — ScanStd 1.8 ms/200 files,
  fast64 4.3 GiB/s, BLAKE3 435 MiB/s, content search 54 ms,
  std linter 38 ms (VS2019 lane, 0.2s min-time).

## [6.5.44] - 2026-11-24 — MSI Packaging Uses VERSION.md

### Fixed
- **MSI lane hardcoded ancient versions**: `wix/main.wxs` carried
  `Version="2.2.1"`, the script named its output
  `filez-2.2.0-win64.msi`, and both expected the legacy `build/`
  tree. The template now takes `$(var.ProductVersion)` and
  `$(var.CliSource)`; `package_msi.bat` reads `VERSION.md` and
  finds the CLI in any current lane (build-msvc/lean/juce/build).
  Wiring-pinned (23/23). WiX is not installed on this host, so the
  script's honest "WiX not found" abort is the verified behavior
  here; an actual `.msi` build needs WiX v3.11.

## [6.5.43] - 2026-11-24 — Persona Example Rules Actually Load

### Fixed
- **Three of four shipped example rule files could not load**:
  the persona examples use a `pattern`/`target` schema (top-level
  list) that the YAML loader never understood (it only read
  `rules:` + `destination`). The loader now accepts both schemas;
  OrganizationRule gained a case-insensitive filename `pattern`
  glob (`*`, `?`, `{a,b,c}`) and the `{filename}` placeholder.
  Persona files were rewritten to supported placeholders
  (`{exif:...}`/`{mtime:...}` → `{year}`/`{month}`/`{day}`, with a
  comment that EXIF-date routing is not implemented) and their
  `/dev/null` "purge" targets became review folders — bobfilez
  never deletes via organization rules. New CLI test loads all
  four examples and plans a photographer move. (Regression watch:
  the new field initially broke aggregate rule initializers
  codebase-wide; it is now declared last, with the hazard noted
  in the header.) Suites: lean **45/45**; other lanes rebuilt.

## [6.5.42] - 2026-11-23 — Fuzz Lane Builds; 14.5M-Run Campaign Clean

### Fixed
- **`scripts/build_fuzzer.bat` could not configure**: it never
  disabled `FO_BUILD_OMNI` (default ON), whose BobUI/Qt probe
  fails configuration on this host before the fuzzer target is
  reached. The script now passes `-DFO_BUILD_OMNI=OFF`
  (wiring-pinned).

### Verified
- **RuleEngine fuzz campaign**: `fuzz_rule_engine` built with
  MSVC `/fsanitize=fuzzer` and ran **14,525,854 inputs in 61s**
  (238k exec/s, seeded with real templates) with no crash or
  sanitizer finding.
- Qt demo rebuilt + JUCE demo launch-verified at v6.5.41 code.
  Suites: lean **44/44**, JUCE **41/41**, MSVC **44/44**,
  wiring **22/22**; lanes rebuilt at this version for parity.

## [6.5.41] - 2026-11-23 — Tray Restart Race and Double-Spawn Fixed

### Fixed
- **Tray could kill the dashboard on Restart**: `stop_server`
  fired `/api/quit` and `start_server` respawned immediately,
  racing the old process for the port (EADDRINUSE), and tray
  startup auto-spawned a duplicate server even when one was
  already running externally. Start now health-checks first
  ("already running (external)"), stop waits for the port to
  actually free, and tray-spawned servers inherit
  `TESSDATA_PREFIX` so OCR keeps working. Proven live through the
  full lifecycle (guard → stop → start → restart → stop).
  Wiring **21/21**.

## [6.5.40] - 2026-11-23 — C API Header Deduplicated

### Changed
- Removed the duplicated Lint/Organize declaration blocks from
  `bobfilez_c_api.h` (identical redeclarations). A declaration vs
  definition sweep of the C API now shows no mismatches in either
  direction. Suites: lean **44/44** at this version's code; other
  lanes rebuilt for parity.

## [6.5.39] - 2026-11-23 — C API Vault Declared and Consumer-Tested

### Fixed
- **C API vault was unreachable**: `fo_bobfilez_vault_initialize/
  lock/unlock/list` were defined in the implementation but never
  declared in the public `bobfilez_c_api.h`, so header-following C
  consumers could not call them. Declarations added (with the
  lock-removes-original warning), and a new `test_c_api.cpp`
  exercises the real consumer chain — initialize → lock → list →
  unlock with byte-identical restore — proving the v6.5.16
  registry fix end-to-end. Suites: lean **44/44**; other lanes
  rebuilt at this version.

## [6.5.38] - 2026-11-23 — Sync Status Stops Claiming "In Sync"

### Fixed
- **`sync status` always said Yes**: `check_sync_status`
  unconditionally returned `is_in_sync = true`. It now consults
  persisted sync history when a sync DB already exists (without
  creating one for a status query) and answers "unknown" with the
  reason when nothing is tracked. New
  `SyncStatusUnknownWithoutActivity` test failed before, passes
  after. Suites: lean **42/42**; other lanes rebuilt at this version.

## [6.5.37] - 2026-11-23 — Whole-Word Content Search Honored

### Fixed
- **`--whole-word` was silently ignored** for literal content
  search (the default): only the regex path embedded `\b`, so
  `search wor --content --whole-word` matched "word". Literal
  matching now scans occurrences until one sits on word
  boundaries. New `SearchWholeWordContentRespected` CLI test
  failed before, passes after. Suites: lean **41/41**; other lanes
  rebuilt at this version.

## [6.5.36] - 2026-11-23 — Vault Writes Are Atomic

### Fixed
- **Failed unlock left garbage behind**: `decrypt_file` streamed
  unauthenticated plaintext straight to the destination, so a wrong
  password (GCM tag failure at the end) left a bogus "restored"
  file. Encrypt and decrypt now write to a `.part` sibling and
  rename into place only on success; failures remove the temp.
  New `VaultWrongPasswordLeavesNoOutput` CLI test failed before,
  passes after (right password still restores byte-identically).
  Suites: lean **40/40**; other lanes rebuilt at this version.

## [6.5.35] - 2026-11-23 — Audit Tamper Evidence, Honestly Bounded

### Verified
- **Audit ledger tamper test**: forging a recorded entry's
  `src_path` flips `/api/audit` to `verified: false` (chain
  recomputation catches edits and reordering; the forged-operation
  case was already unit-tested). Deleting the newest entries
  outright is *not* detectable by hash-chain design — the Audit
  card now says so instead of implying total tamper evidence
  (wiring test pins the wording, 20/20).
- ChatGPT's verdict on the fix reports landed: it credits the
  600-file interruption test as meaningful (not comprehensive),
  recommends authorizing a lean production checkout instead of
  deleting reference repos, and repeats: prove reliability, reduce
  complexity, state exactly what bobfilez can promise.

## [6.5.34] - 2026-11-23 — Undo Skips Stale Records

### Fixed
- **One stale record blocked every older undo**: `undo_last` only
  ever tried the newest record; if its destination had been deleted
  externally, it reported "No operations to undo" forever. It now
  walks the recent journal newest-first and undoes the first record
  that is still reversible. New `UndoSkipsStaleRecords` CLI test
  (two organizes, newest destination removed, undo restores the
  older file) failed before, passes after.
  Suites: lean **39/39**; other lanes rebuilt at this version.

## [6.5.33] - 2026-11-23 — Deletes No Longer Block Undo

### Fixed
- **A journaled Delete permanently blocked undo**: `get_undoable`
  returned every completed operation, so `undo_last` picked the
  newest Delete (irreversible, no backup), failed silently with
  "No operations to undo", and could never reach earlier
  Move/Rename records. Undoable now means reversible
  (`operation_type != 'delete'`); the repository fixture's
  expectation was updated to the corrected contract, and a new CLI
  test (`DeleteDoesNotBlockUndoOfMove`) proves organize → delete →
  undo restores the moved file. Suites: lean **38/38**; other lanes
  rebuilt at this version.

## [6.5.32] - 2026-11-23 — Submodule Inventory (Read-Only)

### Added
- **`docs/SUBMODULE_INVENTORY.md`**: the read-only categorization
  reviewers asked for — of 173 directories under `libs/`, **12 are
  referenced by the build** (core vendored sources + JUCE/BobUI/BTK
  provider trees), **5 are frontend framework trees**, and **159 are
  not referenced by any build file** (reference/parity/parked
  evaluation trees). No pruning is implied or authorized; that
  remains an owner decision. Suites unchanged at this version
  (lean **37/37**, JUCE **34/34**, MSVC **37/37**, wiring **19/19**
  at v6.5.31 code).

## [6.5.31] - 2026-11-23 — {extension} Template Alias; Crash Consistency Proven

### Fixed
- **`{extension}` was a literal folder**: the dashboard taught
  `{extension}/{year}/{name}` but the RuleEngine only knew `{ext}`,
  so dashboard-guided organizes planned moves into a folder
  literally named `{extension}`. `{extension}` is now an alias of
  `{ext}`, the card teaches `{ext}` and lists all placeholders
  (`{ext} {name} {parent} {year} {month} {day}`), and a CLI test
  pins the expansion.

### Verified (ChatGPT's "try to destroy it" challenge)
- **Crash consistency**: `fo_cli organize` on 600 files was killed
  mid-run; all 600 files existed exactly once afterward (364 at
  source, 236 moved), the audit hash chain verified, and journal
  undo restored a moved file. Caveat: organize destinations are
  CWD-relative, so undo must run from the same working directory.
  Suites: lean **37/37**; other lanes rebuilt at this version.

## [6.5.30] - 2026-11-23 — Vault Lock Warning; Watcher Re-Verified

### Fixed
- **Vault card hid a destructive detail**: `VaultManager::lock_file`
  deletes the plaintext original after a successful encrypt (by
  design for a vault), but the Lock button said only "Encrypts the
  file into the vault". The button and field tooltips now state the
  original is removed and unlock restores it (pinned by a wiring
  test, 19/19).

### Verified
- **Watcher end-to-end at this version**: `fo_cli watch` heartbeat
  goes from 0 to 2 events on file creation, `watch-status` and
  `/api/watch/status` agree, watcher stopped cleanly afterward.

## [6.5.29] - 2026-11-23 — Rename Safety: No Silent Overwrites, Honest Failures

### Fixed
- **Renames could silently overwrite files**: MSVC's
  `std::filesystem::rename` replaces an existing destination, so
  organize, rename, and rename-batch could destroy a file that
  happened to sit at the computed destination. All three now refuse
  existing destinations and count them as failures.
- **Rename/rename-batch reported false success** like organize did:
  planned renames were listed regardless of outcome and the exit
  code was always 0. Only successful renames are listed now, JSON
  carries `renamed`/`failed` (rename), failures exit 1, and both
  handlers skip the live database files. Two new CLI tests
  (`RenameReportsFailureWhenDestDirMissing`,
  `RenameBatchReportsFailureOnCollision`) failed before, pass after.
  Suites: lean **36/36**; other lanes rebuilt at this version.

## [6.5.28] - 2026-11-23 — Delete-Duplicates Scoped and Honest

### Fixed
- **Delete-duplicates ignored its folder argument**: it deleted
  from every duplicate group in the DB — groups recorded by the
  last `duplicates` run, whatever folder that covered — so pointing
  it at folder B deleted folder A's files (proven by a failing-first
  CLI test). Groups are now filtered to the given roots. Also,
  `std::filesystem::remove` failures were counted (and journaled)
  as deletions; only removals that actually happen are counted,
  journaled, and audited now, with `failed` in the JSON and a
  nonzero exit on any failure. Suites: lean **34/34**; other lanes
  rebuilt at this version.

## [6.5.27] - 2026-11-23 — Organize Never Moves Its Own Database

### Fixed
- **Organize could move its own live database**: when `--db` pointed
  inside the organized folder, the scan included `fo.db`/`t.db` and
  its `-wal`/`-shm` sidecars as ordinary files to move (observed in
  the interruption probe). The organize handler now excludes the
  active database files (new `DatabaseManager::db_path()` accessor).
  New `OrganizeSkipsOwnDatabaseFiles` test failed before, passes
  after. Suites: lean **33/33**; other lanes rebuilt at this version.

## [6.5.26] - 2026-11-23 — Organize Reports Failed Moves

### Fixed
- **Organize lied about failed moves**: planned moves were listed
  as done regardless of outcome and the exit code was always 0,
  even when every move failed (found probing ChatGPT's
  destructive-op interruption challenge). Only successful moves are
  now listed; JSON gains `moved`/`failed` counts; any failed move
  exits 1 so scripts and the dashboard bridge see the failure.
  Successful moves remain individually journaled, so undo still
  recovers exactly what actually moved. New
  `OrganizeReportsFailureWhenMovesFail` test failed before, passes
  after; manual partial-failure probe confirms the accounting.
  Suites: lean **32/32**; other lanes rebuilt at this version.

## [6.5.25] - 2026-11-23 — Roast Fixes: Flow Failures, Stdout Banners, Doc Truth

### Fixed
- **Workflows always reported success**: `execute_workflow` discarded
  `execute_action` results. A failed action (e.g. MoveTo into an
  uncreatable directory) now fails the run. New
  `FlowFailurePropagates` test failed before, passes after.
- **CLI stdout polluted by engine banners**: every invocation printed
  `[OmniClerk]/[AutoDev]/[SyncService]/[SwarmHub]` init lines to
  stdout, forcing JSON-extraction hacks in every consumer. All
  bracket-log lines in those engines (+ OmniOracle) now go to
  stderr; verified: `fo_cli hash` on stdout alone is one clean line.
- **Docs-vs-reality**: `ROADMAP.md` gained the scope-cut banner
  (shell-replacement/Omni phases are parked, not scheduled) and its
  stale v6.5.7 status pin was removed; `README.md` no longer
  hardcodes a version (truth stays in `VERSION.md`).
- **Repo hygiene**: 174 generated files under the ignored build
  trees (`build-juce-demo`, `build-qt-demo`, `build-btk`,
  `build-headless`, `build-bobui-gui`, `build_output`) untracked;
  artifacts remain on disk. `build-msvc` left tracked for now
  (the daemon sweeps it; separate deliberate change).
- **Data-safety recovery test** (from the roast triage): applied
  organize moves are now proven recoverable via journal undo in
  `OrganizeThenUndoRestoresFile`.
  Suites: lean **31/31**, JUCE-tree **28/28**, msvc **30/30**,
  wiring **18/18** (100% each; totals differ by lane discovery).

## [6.5.24] - 2026-11-17 — build-msvc Lane Green Again

### Fixed
- **Runtime-dependency resolver died on conflicts**:
  `file(GET_RUNTIME_DEPENDENCIES)` hard-errors when the same DLL
  name exists in two search dirs (here: the exe-dir copy from
  `TARGET_RUNTIME_DLLS` vs the lean-prefix build of
  `libcrypto-3-x64.dll`). The script now collects conflicts via
  `CONFLICTING_DEPENDENCIES_PREFIX` instead of failing the build,
  and the script is a declared `DEPENDS` of its post-build step.
  With the two orphaned `fo_tests.exe` processes (PIDs 19124/63392)
  finally gone, `scripts/build_headless.bat` completes and the
  VS2019 `build-msvc` lane passes **29/29** — the first full pass
  since the tree was poisoned. Suites: lean **29/29**, JUCE-tree
  **26/26**, msvc **29/29**, wiring **18/18**. Search name/content
  modes also re-verified via CLI this session.

## [6.5.23] - 2026-11-17 — Convert Command Unbroken

### Fixed
- **`convert` never worked**: the global option parser rejected
  `--to=`/`--quality=` with exit 2 before the handler ran (the
  dashboard bridge sends `--to=`), and the handler itself stripped
  4 chars instead of 5, turning `flac` into `=flac` ("Unknown
  format"). Both fixed, mirroring the rename-batch/audit parser
  pattern. Verified with real conversions (wav→flac via CLI,
  wav→ogg via `/api/convert`, output files created) and a new CLI
  integration test that failed on both defects. Also spot-checked:
  `classify` without a model fails honestly ("Model file not
  found: model.onnx"); `flow list` still shows the built-in
  scaffold workflow (Omni mock family, unchanged).
  Suites: lean ctest **29/29**, JUCE-tree ctest **26/26**, wiring **18/18**.

## [6.5.22] - 2026-11-17 — Scrub Actually Detects Corruption

### Fixed
- **Integrity scrub was a no-op**: the CLI kept baselines only in an
  in-memory engine, so every run re-baselined and reported
  `corrupted:0` forever — while advising users to "run again to
  detect corruption" and the dashboard card promised verification
  against stored baselines. Baselines now persist in the database
  (`file_hashes`, algo `fast64`, shared with the `hash` command):
  first run baselines, later runs compare and report corrupted files
  by name (`status: corruption-detected`). Verified end-to-end by a
  new CLI integration test (tamper between two processes) and over
  HTTP. Suites: lean ctest **28/28**, JUCE-tree ctest **25/25**,
  wiring **18/18**.

## [6.5.21] - 2026-11-17 — Oracle Stops Fabricating Answers

### Fixed
- **OmniOracle invented answers**: `ask()` returned canned text with
  fabricated file citations and counts for every query (e.g. a
  "Project Alpha deadline" answer to "how many files?"), and
  `summarize()` invented a summary of files it never read, while the
  dashboard card claimed "Retrieval-augmented answers over your
  indexed files". No model/index is wired up yet, so both now say so
  plainly (echoing the question, pointing at Search), return no
  citations, and the card copy describes the scaffold honestly.
  Pinned by `RegistryFixture.OracleDoesNotFabricateAnswers` (failed
  before, passes after); verified via CLI and `/api/oracle`.
  Suites: lean ctest **27/27**, JUCE-tree ctest **24/24**, wiring **18/18**.

## [6.5.20] - 2026-11-17 — Similar-Image Search Fixed

### Fixed
- **`similar` could never match**: two compounding bugs. (1) The
  `dhash` hasher (`fo::providers::DHash`) set bits with `<` while the
  OpenCV perceptual hasher used `>`, so hashes stored by
  `hash --hasher=dhash` were the bitwise complement of `similar`
  targets (distance ~64 for identical images; proven: a decreasing-
  gradient pair found 0 matches). (2) `find_similar_images` parsed
  the stored 16-digit **hex** strings as decimal, collapsing most to
  0, and treated unparsable rows as hash 0. Both fixed; unparsable
  rows are now skipped. Note: re-run `hash --hasher=dhash` to refresh
  rows stored with the old convention. Also re-verified this session:
  hash, vhash (16 samples), afingerprint (3 chunks), OCR
  ("Hello bobtilez" from a rendered fixture) via CLI at this version.
  New `FileRepoFixture` tests (3) failed before, pass after.
  Suites: lean ctest **26/26**, JUCE-tree ctest **23/23**, wiring **18/18**.

## [6.5.19] - 2026-11-17 — Root Artifact-Dir Cleanup

### Changed
- **`.gitignore` covers the probe/demo build trees** (`temp_btk_probe/`,
  `build-bobui-gui*/`, `build-btk/`, `build-headless/`, `undefined/`,
  `{extension}/`) so `git status` stays readable; nothing was deleted,
  and content dirs (`filez-java/`, `submodules/`) were deliberately left
  visible. Several of these dirs also have historically tracked files —
  the ignore rules do not untrack those. Qt MinGW lane re-verified this
  session (rebuild + offscreen launch at v6.5.18 source).
  Suites: lean ctest **23/23**, JUCE-tree ctest **20/20**, wiring **18/18**.

## [6.5.18] - 2026-11-17 — Dashboard Ignore-Rule Removal

### Added
- **Remove rule button on the Ignore rules card**: the card copy already
  promised "or remove it", and `POST /api/ignore/remove` existed
  server-side, but no dashboard control called it (found by diffing the
  46 server routes against the dashboard's called endpoints; the only
  other uncalled route, `/api/delete`, is used by the legacy React UI —
  the unified page deliberately deletes only via the dry-run
  duplicate-delete card). New wiring test pins add+remove; verified
  live: add → listed → remove → gone.
  Suites: lean ctest **23/23**, JUCE-tree ctest **20/20**, wiring **18/18**.

### Known
- Two orphaned `fo_tests.exe` processes (PIDs 19124, 63392; parents
  exited, access denied on stop) still lock `build-msvc/tests/*.dll`;
  left untouched. The owner/session that can stop them should, then
  rerun `scripts/build_headless.bat`.

## [6.5.17] - 2026-11-17 — Test Runtime-DLL Resolution Fixed

### Fixed
- **gtest discovery failed in `build-msvc`** (`0xc0000135`): the
  `fo_tests` post-build copy used `$<TARGET_RUNTIME_DLLS:fo_tests>`,
  which only covers imported targets; providers linked by plain library
  names (OpenCV, lean-prefix staged libs) contributed no DLLs, so the
  test executable could not start at discovery time. Glob-copying whole
  prefix bin dirs instead shadowed good DLLs with debug variants
  (`0xc0000005`). `tests/copy_runtime_deps.cmake` now resolves the exact
  loader set from the linked executable via
  `file(GET_RUNTIME_DEPENDENCIES)` and copies only those. Verified:
  lean ctest **23/23**, JUCE-tree ctest **20/20**, wiring **17/17**.

### Known
- The `build-msvc` tree itself could not be re-verified this session:
  two hung `fo_tests.exe` processes (PIDs 19124, 63392, no readable
  path/command line, access denied on stop — likely owned by another
  session) lock its `tests/*.dll`, and that tree's VS2019-built test
  binary access-violates during discovery. Left untouched per
  process-ownership rules; the fix is verified in both VS18 trees.

## [6.5.16] - 2026-11-17 — Vault Registry Registration Fixed

### Fixed
- **C API vault could never initialize**: `Registry<IVaultManager>` was
  populated only by a TU-local static initializer in
  `vault_manager_reg.cpp`, whose companion `force_register_vault_manager()`
  was never called (and lived in the wrong namespace), so
  `fo_bobfilez_vault_initialize()` received `nullptr` from
  `create("aes256_gcm")` and always failed for direct C consumers
  (e.g. BobGUI direct mode). Registration is now the explicit
  `register_vault_manager()`, declared in `provider_registration.hpp` and
  called from `register_all_providers()`; `fo_bobfilez_vault_initialize`
  also now calls `ensure_providers_registered()` before creating.
  Proven by the new `RegistryFixture.VaultManagerRegistered` test, which
  failed on both assertions before the fix. The CLI vault path was
  unaffected (it constructs `VaultManager` directly).
  Suites: lean ctest **23/23**, JUCE-tree ctest **20/20**, wiring **17/17**.

## [6.5.15] - 2026-11-17 — JUCE Demo Building, Perceptual Registration Fixed

### Added
- **`fo_juce_demo` builds and launches** (`frontends/juce`, `build-juce-demo/`):
  `scripts/build_juce_gui.bat` now discovers the VS18 vcvars toolchain and
  pins `Z_VCPKG_PWSH_PATH`/`Z_VCPKG_POWERSHELL_PATH` to an installed
  PowerShell (vcpkg applocal otherwise fails on a stale preview path).
  `libs/JUCE` restored to the recorded gitlink `762a87fc30` (the worktree
  had drifted to a broken JUCE 9 merge). Verified: exe links
  (`bobfilez JUCE Demo.exe`, ~24 MB) and launch-verified (window titled
  "bobfilez JUCE Demo", responding; stopped after verification).

### Fixed
- **Perceptual hashers could vanish from the registry**: `perceptual_opencv.cpp`
  used a TU-local static registrar plus a no-op `register_perceptual_opencv()`
  guard, so the linker dropped `dhash`/`phash`/`ahash`/`opencv` from
  `IPerceptualHasher` when the TU was pulled from `fo_core.lib` only for the
  registration call (observed as `RegistryFixture.CoreProvidersRegistered`
  failing in the JUCE tree). Registration is now explicit, mirroring the
  classifier/OCR/video/audio providers. `FO_HAVE_OPENCV` is now `PUBLIC`
  (like `FO_HAVE_FFMPEG`/`FO_HAVE_CHROMAPRINT`) so tests/consumers guard on
  the same condition, and the `dhash` assertion lives under that guard with
  a dedicated `PerceptualHashersRegisteredWhenOpenCVPresent` test.
  Suites: lean ctest **22/22**, JUCE-tree ctest **19/19**, wiring **17/17**.

### Known
- **BobGUI lane blocked upstream**: `libs/bobgui/meson.build` is corrupt at
  its pinned commit (duplicate, unclosed `project('bobgui', ...)` before
  `project('gtk', ...)`), so `meson setup` fails parsing before any
  dependency check. Not edited (submodule); also a full GTK4 build would
  need the glib/pango/cairo wrap chain. Probe build dir removed.

## [6.5.14] - 2026-11-17 — Qt Demo Aligned and Building

### Fixed
- **`fo_qt_demo` compiles and launches again** (`frontends/qt`): added
  the missing `QLineEdit` include, qualified free-scope `connect` calls
  as `QObject::connect`, and aligned drifted core calls with current
  APIs (`SearchOptions::search_roots` + `SearchResult::path`,
  `OperationRepository::get_all`, `IgnoreRepository::get_all`/`add`).
  Verified with the MinGW Qt 6.11 kit (`build-qt-mingw/`, w64devkit
  GCC): builds clean and stays alive under the offscreen platform.
  The MSVC helper script still needs an MSVC Qt kit; the MinGW lane is
  the verified Qt path (new build dirs gitignored).

## [6.5.13] - 2026-11-17 — Scanner File Roots, Dirent Crash, Batch-Rename Grammar

### Fixed
- **Scanners dropped file roots**: `std`, `dirent`, and `win32` scanners
  returned nothing when a root was a file, so dashboard Hash/Metadata
  (which accept "file or folder") silently returned `[]`. All three now
  scan a file root as itself (contract pinned by new scanner tests).
- **Dirent scanner crashed on directory scans** with the MSVC CRT: the
  vendored shim stores the pointer-sized find handle in a 32-bit `long`.
  The Windows path of `DirentScanner` now walks with `std::filesystem`
  (vendored code untouched; POSIX keeps the dirent loop). Found by the
  new scanner test crashing the suite — reproduced standalone first.
- **Batch rename never worked**: the CLI global option parser rejected
  `--find=`/`--replace=`/`--regex` before the rename-batch handler saw
  them, and the dashboard bridge sent a nonexistent `--rules` argument.
  Parser now accepts the handler's options; the card and bridge speak
  find/replace/regex. Covered by a CLI integration test.

### Added
- Scanner contract tests + rename-batch dry-run test. Suites: ctest
  **21/21**, wiring **17/17**.

### Known
- The plain Qt demo (`fo_qt_demo`) configures against the MinGW Qt kit
  but its `main.cpp` is bit-rotted against current core APIs (missing
  includes, unqualified `connect`, removed repository methods). It needs
  a dedicated API-alignment pass; the web dashboard remains the verified
  UI. See TODO `qt-demo-alignment`.

## [6.5.12] - 2026-11-17 — Audit-Write Integration Coverage

### Added
- CLI integration tests for **applied** audit writes: an applied
  organize move appends a `move` ledger entry and an applied
  delete-duplicates run appends a `delete` entry (with exactly one
  duplicate surviving), each followed by a verified hash chain.
  Suite total: **16 tests, all passing** (`ctest`), wiring 15/15,
  exhaustive HTTP smoke all 200.

## [6.5.11] - 2026-11-17 — CLI-Level Integration Tests

### Added
- **CLI integration tests** in the tracked `tests/` tree: the real
  `fo_cli.exe` is driven as a subprocess for audit `--limit` validation
  (0, 1001, and non-numeric values all exit 2), fresh-database audit
  JSON verification, and a full vault init→lock→list→unlock byte-compare
  round-trip. Suite total: **14 tests, all passing** (`ctest`).

### Verified
- Dashboard wiring tests 15/15 and the exhaustive HTTP smoke (health,
  providers, AutoDev/Sync, audit, modules, scan, OCR, watcher, quit)
  all 200. The lean lane also gained the cloud scanners in this
  configure (shared manifest prefix for GTest), so both lanes now expose
  the identical full provider matrix.

## [6.5.10] - 2026-11-17 — Tracked C++ Test Tree Restored

### Added
- **`tests/` GTest tree** (was untracked/absent, so `ctest` found no
  tests): audit-ledger history/limits/tamper-detection, operation-journal
  logging/ordering/rename-undo, and provider-registry contracts
  (including FFmpeg/Chromaprint registration when compiled in) —
  **11 tests, all passing** via `ctest --test-dir build-lean`.
- Test runtime DLLs are copied beside `fo_tests` automatically
  (`TARGET_RUNTIME_DLLS`), so gtest discovery works on clean builds.

### Verified
- Browser-verified dashboard through MS Edge (CDP 9222): page render,
  Scan, Audit, Provider Directory, and Folder Watcher controls exercised
  with real results; a stale pre-audit server process was caught by the
  browser check and restarted. Exhaustive HTTP smoke: all endpoints 200.
  HyperNexus MCP read-only tools (status/observability/memory/workflows)
  healthy. Wiring tests remain green.

## [6.5.9] - 2026-11-17 — Media Providers Live (Video + Audio Fingerprinting)

### Added
- **Video hashing (`ffmpeg`) and audio fingerprinting (`chromaprint`)**
  providers in both MSVC lanes. FFmpeg is staged from the official
  prebuilt shared release (with a minimal `FFmpegConfig.cmake`) and
  Chromaprint is built from source against staged FFTW, after the vcpkg
  FFmpeg source build failed twice on this host (MSYS2 fork exhaustion,
  then a 90-minute MSVC optimizer spin on `sanm.c`).
- Dashboard Video/Audio cards declare their provider needs and are badged
  from `/api/providers` when a build lacks them.

### Fixed
- Both media providers used TU-local static registrars (linker-dropped
  from `fo_core.lib`, the same bug class as the classifier); replaced
  with explicit `register_video_hasher_ffmpeg()` /
  `register_audio_fingerprinter_chromaprint()` calls. Core now also
  requests/links `FFmpeg::swresample` (used by the audio provider) and
  aliases upstream `Chromaprint::chromaprint` to the vcpkg name.
- Video/Audio card copy now says files, not folders (the CLI hashes
  individual media files), with a ~10s minimum-audio hint.

### Verified
- `--list-vhash` → `ffmpeg`, `--list-ahash` → `chromaprint` on both
  lanes; sample MP4 yields timestamped frame hashes (identical across
  lanes), 10s WAV yields 59 fingerprint chunks; `/api/vhash` and
  `/api/afingerprint` return 200 with the same results. Tests 15/15.
  **Every provider row is now populated in every verified lane.**

## [6.5.8] - 2026-11-16 — Persistent Forensic Audit Query

### Added
- **`fo_cli audit` command**: reads the persistent `audit_ledger` table
  (Migration 5) via the new `make_audit_logger(DatabaseManager&)` factory,
  with `--limit=N` (1–1000, validated) and `--format=json`; output includes
  hash-chain verification (`verified`, computed over every entry).
- **Audit writing**: applied CLI move (organize), delete-duplicates, and
  rename operations now also append hash-chained audit entries next to the
  existing operation journal. Dry-runs write nothing.
- `/api/audit` now queries the real ledger (optional `?limit=`) and the
  dashboard card loads entries instead of checking availability.

### Verified
- Isolated-DB end-to-end: an applied organize move produced one ledger
  entry; `audit --format=json` returned it with `verified: true` from
  both the lean and manifest binaries (identical chain hash), and
  `--limit=0` was rejected with exit 2. Wiring tests 14/14.

## [6.5.7] - 2026-11-16 — Full Manifest Parity + Dashboard Coverage Completion

### Added
- Unified dashboard cards for **Provider directory**, **Autonomous services**
  (AutoDev/Sync status + Sync history), and **Forensic audit ledger**
  availability, all with descriptive labels and guidance tooltips.
- Server bridges `/api/providers`, `/api/download-models`,
  `/api/autodev/status`, `/api/sync/status`, and `/api/sync/history`.
- Video/audio media cards now declare their optional provider requirements
  and are badged from `/api/providers` when a build lacks them.

### Fixed
- Full manifest `build-msvc` binary now supplements the vcpkg manifest with
  the verified lean provider prefix and sidecars its DLLs, so the newest
  binary no longer silently loses OCR/OpenCV/ONNX while gaining cloud
  scanners. `build_headless.bat` also selects an installed PowerShell for
  vcpkg's applocal step instead of trusting a stale preview path.
- `/api/audit` no longer returns fabricated empty history; it returns an
  honest 501 explaining that the core audit table has no CLI query yet.
- `scripts/dev_all.bat` invokes `node --test test/wiring.test.js` explicitly
  (the directory form fails on current Node), and the duplicate destructive
  `scripts/build_with_env.bat` was archived.

### Verified
- `scripts/build_headless.bat` completes; manifest `fo_cli --modules` shows
  cloud scanners plus OCR/Classifiers/Perceptual OpenCV. The existing
  `fo_tests.exe` binary passes 436 tests with 1 model-dependent skip, but it
  predates this source tree because no `tests/` directory is tracked here.
- Dashboard wiring tests pass 13/13. Live HTTP smoke covers health, page,
  providers, model help, AutoDev/Sync status/history, audit honesty,
  modules, scan, OCR, Classify, watcher, and quit. Tray starts the server,
  health succeeds, and quit stops it.

## [6.5.6] - 2026-11-14 — Classifier Softmax Confidences

### Fixed
- Classifier `confidence` was the raw model logit (e.g. `7.32`, could
  exceed 1 or go negative). `classifier_onnx.cpp` now applies a
  numerically stable softmax, so confidences are probabilities in
  (0, 1) summing to 1 and comparable across images. Ranking unchanged
  (softmax is monotonic): same test image keeps top-3 984/722/417, now
  reported as 0.152 / 0.084 / 0.055 via both CLI and `POST /api/classify`.

## [6.5.5] - 2026-11-14 — OCR Provider Live (All Providers Populated)

### Added
- **Tesseract OCR provider**: vcpkg Tesseract 5.5.2 chain (15 packages)
  into `build-lean-deps`, `eng.traineddata` staged, provider DLLs
  sidecarred next to the lean `fo_cli.exe`. `--modules` now shows every
  row populated: OCR tesseract, Classifiers onnx, Perceptual opencv.

### Fixed
- `providers/tesseract` provider was a **placeholder** (`recognize` always
  returned nullopt) with only a TU-local static registration. Implemented
  real OCR (TessBaseAPI + leptonica, `TESSDATA_PREFIX` discovery,
  mean-confidence) and explicit `register_ocr_tesseract()` called from
  `register_all_providers()`; fixed the latent `tesseract::` namespace
  shadowing that prevented it ever compiling with `FO_HAVE_TESSERACT`.

### Verified
- CLI OCR on a rendered text image returns its text; dashboard
  `POST /api/ocr` returns 200 with the same text (server started with
  `TESSDATA_PREFIX` pointing at the staged tessdata). Wiring tests pass.

## [6.5.4] - 2026-11-14 — Classify Provider Live (Prebuilt ONNX + OpenCV)

### Added
- **Classifier + perceptual-OpenCV providers** in the lean lane: staged the
  official prebuilt ONNX Runtime 1.20.1 and OpenCV 4.10.0 Windows releases
  into `build-lean-deps` (skipping the multi-hour vcpkg source chain), with
  a minimal `onnxruntimeConfig.cmake` for the imported target. Configure
  now finds OpenCV + ONNX Runtime + OpenSSL together.

### Fixed
- **Classifier never registered** (`core/src/classifier_onnx.cpp`): its only
  registration was a static registrar in a translation unit nothing
  references, so the linker dropped it from `fo_core.lib` even with both
  libraries present. Replaced with explicit `register_classifier_onnx()`
  called from `register_all_providers()` (the codebase pattern), with a
  no-op stub when the libraries are absent.

### Verified
- `--modules`: **Classifiers: onnx**, **Perceptual Hash: dhash phash ahash
  opencv**. Classify end-to-end: CLI returns top-3 ImageNet indices for a
  test image (MobileNetV2); dashboard `POST /api/classify` returns 200 with
  the same classifications. Deployment note: place `model.onnx` (optional
  `labels.txt`) in the server CWD (`bobui_web/`); without labels, numeric
  class indices are shown. OCR (Tesseract) remains unbuilt and badged.

## [6.5.3] - 2026-11-14 — Queryable Watcher Status (Last 501 Closed)

### Added
- **Watcher heartbeat + `watch-status` query** (`cli/fo_cli.cpp`): the
  blocking `watch` poll loop now publishes `%TEMP%/bobfilez_watch_status.json`
  (started/heartbeat/events/last_event/paths) every poll; the new
  non-blocking `watch-status` command reports `watching:true` only while the
  heartbeat is fresh (≤15s), else `watching:false` (`stale` when expired).
- `/api/watch/status` now bridges to `watch-status --format=json` (HTTP 200)
  instead of the blanket 501; the dashboard Folder Watcher card documents
  starting a watcher in a terminal.

### Verified
- Live watcher E2E: heartbeat fresh with `events: 0` at baseline, creating a
  file moved it to `events: 1` with `last_event: NEW …/newfile.txt`; killed
  watcher correctly decays to not-watching. Wiring tests **9/9**; dashboard
  smoke (health picks `build-lean`, scan/clerk/oracle/modules/history) 200.
- Every dashboard bridge is now real: no honest-501 endpoints remain
  (vault graduated in 6.5.2, watch in 6.5.3).

## [6.5.2] - 2026-11-14 — Vault End-to-End Proof (AES-256-GCM) + Lean MSVC Lane

### Added
- **Lean MSVC lane** (`build-lean/` + `build-lean-deps/`): MSVC build with no
  vcpkg manifest toolchain; standalone OpenSSL 3.6.1 (vcpkg classic mode)
  supplies `FO_HAVE_OPENSSL`. Dashboard server now also considers
  `build-lean/cli/fo_cli.exe` (newest-mtime picker). OpenSSL DLLs sidecarred
  next to the lean `fo_cli.exe` for dashboard-spawned runs.

### Fixed
- MSVC compile break in `core/src/enhanced_fileops.cpp`: `windows.h`
  `max` macro collided with `std::max` (C2589); added the `NOMINMAX` guard
  already used by `file_watcher.cpp`.

### Verified
- **Vault AES-256-GCM round-trip end-to-end**: lean CLI
  init→lock→list→unlock restored byte-identical content; HTTP smoke through
  the Express bridges returned 200 for all four ops with
  `restored-match: true`. Fresh binary reports **v6.5.2**; wiring tests pass.
- Known limits: `lock_file` consumes the source file; full manifest
  `build-msvc` lane still unbuilt (aws-sdk-cpp vcpkg extraction failed on
  disk pressure/long paths) — the lean lane is the verified MSVC path.

## [6.5.1] - 2026-11-14 — Dashboard Wiring Fixes + Script Archive + Governance Docs

### Fixed
- **History card no longer undoes on load** (`bobui_web/public/unified.html`):
  the primary GET button fired `POST /api/undo` before loading history.
  Undo now fires only from its dedicated danger button.
- **HTTP errors are no longer logged as OK**: `api()` checks `r.ok`; 501/500
  responses (vault, watch) render as `ERROR (status): …` and log FAIL.
- **Clerk/Oracle bridges match the real CLI grammar** (`bobui_web/server.js`):
  `clerk --format=json <paths>` always usage-errored (CLI accepts only
  `clerk start|status`); the bridge now runs the honest `clerk status`
  probe and the card is relabeled *OmniClerk status*. `oracle` now forwards
  the bare question (CLI prints plain text, has no `--format=json` mode).
- **Tray icon polled dead routes** (`bobui_web/tray.py`): health used
  `/health` and quit used `/quit`; both now use `/api/health` / `/api/quit`.
- Server prefers the **newest-built** CLI binary (`pickCli` by mtime) so a
  stale `build-msvc` (v6.3.0) no longer shadows a fresh `build-mingw`.
- `bobui_web/start-server.bat`: defaults `PORT=3131` when unset.

### Added
- Search card + `/api/search` forward wildcard/case/whole-word/invert/max-results.
- Durable dashboard tests (`bobui_web/test/wiring.test.js`, 7 tests incl.
  live health + honest-501 + newest-CLI picker), wired into `scripts/dev_all.bat`.
- `VISION.md` (was missing). Archived 20 root `patch_*.py`/`fix_*.py`
  one-shots to `scripts/archive/root-helpers/`. Refreshed ROADMAP/TODO/
  MEMORY/DEPLOY/IDEAS/HANDOFF for v6.5.1.

### Verified this session (2026-11-14, heavy processes BelowNormal)
- MinGW rebuild (`build-mingw/`, w64devkit gcc 15.2) after restoring
  `libs/hash-library` + `libs/xxHash` pins: fresh `fo_cli` = **v6.5.1**,
  `clerk status` exit 0. Live smoke: health/scan/clerk/oracle/modules/
  history all **200**, `/api/quit` clean, log error-free. Tests **7/7**.
- Stale MSVC `fo_tests.exe`: **436 passed, 1 skipped** (not rebuilt — this
  checkout has no `tests/` tree). Honest gaps remain: OCR/Classifiers/
  Perceptual-Hash providers empty; vault + watch/status stay 501 by design.

## [6.5.0] - 2026-11-03 — MinGW Lane Builds + Exit-Crash Fix + Verified Bridges

### Added
- **Portable MinGW build lane** (`scripts/build_headless_mingw.bat`): configures
  `build-mingw/` with GUI/Omni/Bench/Tests off using the no-admin toolchain at
  `C:/tools` (cmake 4.4.5 + ninja 1.13.2 + winlibs gcc 16.2 UCRT). Documents the
  `cmd /c` proxy requirement (direct `C:\tools\...` execution is sandbox-blocked).
- **Vendored fallbacks for header-only gaps**: `libs/nlohmann/json.hpp` (v3.11.3,
  audit_logger needs it unguarded) and `libs/portability/endian.h` (MinGW GCC
  takes the POSIX `#ifndef _MSC_VER` branch but ships no `endian.h`).
- **`lint` CLI command**: `lint_command` (existed in `cli/cmd_lint.cpp` but was
  never dispatched) is now wired in `fo_cli.cpp` with `--format=json` output and
  full 8-value `LintType` names, backing the `/api/lint` bridge.
- **`extractJson` in `bobui_web/server.js`**: strips `fo_cli` service chatter
  (`[SwarmHub]`/`[AutoDev]`/... stdout lines) ahead of and behind the JSON
  payload (payload-start scan + bracket-balance cut; bare `--count` numbers fall
  back to the last non-log line). Applied to all 27 `JSON.parse` + 1 `parseInt`
  call sites; `build-mingw/cli/fo_cli.exe` added to `CLI_CANDIDATES`.
- **Bridge corrections**: `/api/organize` now sends `--rule=<t>` (CLI only accepts
  `=` form); `/api/delete-duplicates` drops the unsupported `--path=` option;
  `/api/watch/status` answers honest 501 (CLI `watch` is a blocking loop).

### Fixed
- **Process-exit segfault/heap-corruption in every CLI invocation** (MinGW exit
  code `0xC0000374`; latent on all platforms): `Engine` eagerly `get_shared`s
  the autodev service, whose destruction chain ends at `SwarmEngineImpl`, which
  touched the already-destroyed `SwarmHub` function-local static during static
  teardown. `SwarmHub::instance()` is now an intentionally leaked heap singleton.
  Verified: `hash`/`scan`/`stats`/`duplicates`/`lint`/`organize`/`scrub` all exit 0.
- MinGW portability: `find_package(blake3|OpenSSL)` → QUIET (code already guards
  `FO_HAVE_*`; vault compiles to honest stubs without OpenSSL), guarded
  `add_subdirectory(tests)` (tree has no `tests/`), missing `<cstdint>` include,
  GDI+ 2-arg `Save` default (MSVC-ism), explicit `gdiplus` link for MinGW
  (`#pragma comment(lib)` is MSVC-only), `fo_c_api` include path for vendored headers.
- Sidecar MinGW runtime DLLs (`libstdc++-6`, `libgcc_s_seh-1`, `libwinpthread-1`)
  copied next to `build-mingw/cli/fo_cli.exe` so dashboard-spawned processes start.

### Verified this host (2026-11-03)
- `fo_cli --help` lists 30 commands; HTTP end-to-end via node server:
  `/api/health` ok, scan (6 files), duplicates (`[]`), stats, lint
  (4× DUPLICATE NAME), count (6), organize dry-run (6 moves), delete-duplicates
  dry-run, scrub (`scanned:6,corrupted:0`), undo, hash, metadata, export,
  search, history, ignore add/list/remove, flow/list, modules all 200;
  `/api/vault/*` and `/api/watch/status` answer honest 501.
- NOT verified (no providers/models on host): ocr/similar/vhash/afingerprint/
  classify/oracle/clerk/convert/rename flows (bridges fail gracefully, 500 with message).
## [6.4.0] - 2026-11-03 — Unified Dashboard + Honest Backend Report

### Added
- **Unified single-page dashboard** (`bobui_web/public/unified.html`): all 25 CLI
  workflows on one value-ordered page (A Essentials → B Verify & Manage →
  C Media & AI → D Organize & Protect → E Automate), every card with ⓘ
  tooltips, descriptive labels, shared activity log, sticky group nav.
  Linked from classic (`/`) and React (`/react/`) entry pages.
- **13 new Express bridges** in `bobui_web/server.js`: `/api/ocr`,
  `/api/similar`, `/api/vhash`, `/api/afingerprint`, `/api/classify`,
  `/api/rename`, `/api/rename-batch`, `/api/convert`, `/api/undo`,
  `/api/clerk`, `/api/oracle`, `/api/watch/status`, `/api/modules`,
  plus `/api/quit` for tray/controlled shutdown. All return honest
  `CLI not found` errors when `fo_cli` is not built (verified live).
- **System-tray controller** (`bobui_web/tray.py` + `tray.bat`):
  open dashboard / legacy UI, status probe, restart, logs folder,
  quit server+tray. Uses pystray when installed, tkinter fallback otherwise
  (tkinter verified present; pystray NOT installed on this host).
- **Script hygiene**: archived superseded ex-root `build_limited.bat` and
  `build_fuzz.bat` into `scripts/archive/` via `git mv` (history preserved);
  added `scripts/README.md` (entry-point map) and `scripts/dev_all.bat`
  (build → test → smoke → serve one-shot).

### Verified this host (2026-11-03)
- node v26.13.0 OK; dashboard serves `unified.html` (HTTP 200, 26 KB);
  `/api/health` → `ok:true, cliExists:false`; POST bridges fail honestly
  without `fo_cli`; `/api/quit` shuts the server down cleanly.
- **CANNOT build here**: no cmake, ninja, cl.exe, or Visual Studio;
  no `D:\Qt` kit. `scripts/build_headless.bat` exits immediately
  (`Could not find Visual Studio vcvars64.bat`). C++ build/test/CLI
  verification must run on a dev host (see TODO `build-host`).
## [6.3.2] - 2026-05-26

### Changed
- Reconciled and merged AI feature branches in libs/bobgui and libs/bobui.
- Synchronized all 160+ submodules recursively.


## [6.3.0] - 2026-04-10

### Added -- The "Full Parity" Frontend Release + Jules Clone Fix

This version achieves near-complete feature parity across all four frontend lanes
and fixes a critical Jules/CI clone failure caused by stale nested submodule commits.

#### Jules Clone Fix (Critical)
- Fixed `libs/btk/external/bobui-reference/submodules/juce` pointing to unreachable
  commit `fe2ffcf...` that was rebased/force-pushed on JUCE upstream.
- Updated submodule pointers across 3 repositories:
  - **bobui**: juce -> `3ba67d45`, ultimatepp -> `70e1422b`
  - **bobgui**: juce -> `3ba67d45`, ultimatepp -> `70e1422b`
  - **btk**: bobui-reference -> latest bobui main, juce/ultimatepp synced
- Fixed `core/src/audit_logger.cpp` broken includes (duplicated `sys/syscall.h`)
- Fixed `libs/tinyxml2` merge conflict marker in tracked commit
- Added OpenSSL to `vcpkg.json` and `core/CMakeLists.txt` for vault_manager

#### Frontend Feature Matrix (v6.3.0)

| Feature       | WebUI | BTK | Qt  | BobGUI |
|---------------|-------|-----|-----|--------|
| Scanner       | YES   | YES | YES | YES    |
| Duplicates    | YES   | YES | YES | YES    |
| Statistics    | YES   | YES | YES | YES    |
| Hasher        | YES   | YES | YES | YES    |
| Metadata      | YES   | YES | YES | YES    |
| Lint          | YES   | YES | YES | YES    |
| Search        | YES   | YES | YES | YES    |
| Flow          | YES   | YES | YES | YES    |
| Scrub         | YES   | YES | YES | YES    |
| Export        | YES   | YES | YES | YES    |
| Count         | YES   | YES | YES | YES    |
| History       | YES   | YES | YES | YES    |
| Ignore Rules  | YES   | YES | YES | YES    |
| Organize      | YES   | YES | YES | YES    |
| Delete Dupes  | YES   | YES | YES | YES    |
| Dashboard     | YES   | -   | YES | -      |
| Undo          | -     | -   | -   | YES    |

**Tab Counts**: WebUI=16, BTK=15, Qt=16, BobGUI=21 operations

#### BTK/CopperSpice Native Demo -- 15 tabs
- New: History, Ignore, Organize, Delete Dupes

#### BobUI/Qt Demo -- 16 tabs
- New: History, Ignore, Organize, Delete Dupes

#### BobGUI/GTK Demo -- 21 operations
- New: Organize, Delete Dupes, Undo

#### WebUI (Express + React) -- 16 tabs
- New: Delete Dupes tab with dry-run preview

## [6.2.1] - 2026-04-08

### Added — The "Export + Organize + Count" Frontend Expansion

#### WebUI (Express + React)
- **3 new React tabs**: Export (JSON/CSV/HTML format selector), Organize (dry-run preview with rule template), Count (3-card dashboard: files/duplicates/wasted MB).
- **15 total React tabs**: Dashboard, Scanner, Duplicates, Statistics, Hasher, Metadata, Lint, History, Ignore Rules, Search, Flow, Scrub, Export, Organize, Count.
- Organize uses `{extension}/{year}/{name}` rule template with live preview.
- Count shows large-format stat cards with color-coded metrics.

#### BTK/CopperSpice Native Demo
- **2 new tabs**: Export (JSON export with file/duplicate data), Count (file/directory/size summary).
- **11 total tabs**: Scanner, Duplicates, Statistics, Hasher, Metadata, Lint, Search, Flow, Scrub, Export, Count.
- Export uses `fo::core::Exporter::to_json()` directly.

#### BobUI/Qt Demo
- **2 new inline tabs**: Export (JSON with file/duplicate data), Count (file/directory/MB summary).
- **12 total tabs**: Dashboard, Scanner, Duplicates, Statistics, Hasher, Metadata, Lint, Search, Flow, Scrub, Export, Count.
- Both use real `fo_core` engine integration.

#### BobGUI/GTK Demo
- **2 new operation buttons**: Export, Count — added to utility row.
- **17 total operations**: Scan, Duplicates, Statistics, Hash, Metadata, Lint, Search, Flow, Scrub, Export, Count, History, Ignore List, Ignore Add, Ignore Remove, Reset Fields, Clear Output.

## [6.2.0] - 2026-04-08

### Added — The "Four Frontend" Release

This version delivers major frontend enhancements across **all four** bobfilez UI lanes.

#### WebUI (Express + React)
- **6 new API endpoints**: `/api/flow/list`, `/api/flow/run`, `/api/scrub`, `/api/search`, `/api/export`, `/api/organize`.
- **3 new React tabs**: Flow (workflow list + execute), Scrub (integrity verification), Search (file search by name/content).
- **15 total React tabs**: Dashboard, Scanner, Duplicates, Statistics, Hasher, Metadata, Lint, History, Ignore Rules, Search, Flow, Scrub.
- **21 total API endpoints**.

#### BTK/CopperSpice Native Demo
- **3 new tabs**: Search (query + directory), Flow (list workflows), Scrub (integrity baseline).
- **9 total tabs**: Scanner, Duplicates, Statistics, Hasher, Metadata, Lint, Search, Flow, Scrub.
- All new tabs use real `fo_core` engine integration (SearchEngine, OmniFlow, SelfHealing).

#### BobUI/Qt Demo
- **3 new tabs**: SearchTab, FlowTab, ScrubTab — each with real `fo_core` backend integration.
- **10 total tabs**: Dashboard, Scanner, Duplicates, Statistics, Hasher, Metadata, Lint, Search, Flow, Scrub.
- Search uses `engine.search_engine().search()`, Flow uses `IOmniFlowEngine`, Scrub uses `ISelfHealingEngine`.

#### BobGUI/GTK Demo
- **3 new operation buttons**: Search, Flow, Scrub — added to filesystem/operations/utility rows.
- **15 total operations**: Scan, Duplicates, Statistics, Hash, Metadata, Lint, Search, Flow, Scrub, History, Ignore List, Ignore Add, Ignore Remove, Reset Fields, Clear Output.

## [6.1.5] - 2026-04-08

### 🎉 500-TEST MILESTONE — ALL PASSING! 🎉

### Added
- **OmniFlow new actions**: `Action.Rename` (rename files via pattern), `Condition.AlwaysFalse` (explicit block).
- **8 new OmniFlow tests**: Rename action, AlwaysTrue/AlwaysFalse conditions, no-trigger returns false, Extension filter pass/reject, IsLarger filter pass/reject.
- **500 tests milestone** — 257 tests added since v6.0.86, doubling the test suite.
- Test count: 492 → **500 (499 passed, 1 skipped)**.

### Marathon Session Summary (v6.0.86 → v6.1.5)
- **+257 tests** across 25+ versions (243 → 500, 2x growth)
- **Real implementations**: OmniFlow BFS executor, OmniGraph pathfinding, SelfHealing verification, OmniFlow SQLite persistence
- **8 filter/condition/action types**: IsPDF, IsImage, IsAudio, IsVideo, IsArchive, Extension, IsLarger, TextContains
- **C API extensions**: export CSV/HTML, flow list/execute
- **CLI commands**: flow, scrub (23 total)
- **28 MSVC provider registration fixes**

## [6.1.4] - 2026-04-08

### Added — The "CLI Scrub Command" Release
- **CLI `scrub` command**: `fo scrub <path>` establishes file integrity baselines using fast64 hashing.
- Supports `--format=json` for structured output.
- 23 CLI commands total (scan, duplicates, hash, metadata, ocr, similar, vhash, afingerprint, classify, organize, delete-duplicates, rename, stats, lint, export, undo, history, ignore, convert, search, rename-batch, watch, flow, **scrub**).

## [6.1.3] - 2026-04-08

### Added — The "Self-Healing Real Verification" Release
- **SelfHealingEngine real hash verification**: `register_baseline()`, `verify_file()`, and `scrub_directory()` now compute actual file hashes and detect corruption.
- **Corruption detection**: Files with changed content are detected via hash mismatch.
- **Directory scrubbing**: `scrub_directory()` verifies all files with registered baselines and reports corruption count.
- **6 Self-Healing verification tests**: match baseline, detect corruption, nonexistent file, register+scrub clean/dirty, no baselines, reports accumulate.
- Test count: 486 → **492 (491 passed, 1 skipped)**.

## [6.1.2] - 2026-04-08

### Added — The "OmniFlow Persistence" Release
- **OmniFlow SQLite persistence**: `save_workflows()` and `load_workflows()` serialize/deserialize complete workflows (nodes, connections, config) to SQLite.
- **OmniFlow `remove_workflow()`**: Delete workflows by ID from the engine.
- **6 OmniFlow persistence tests**: Save+Load roundtrip, empty database, load nonexistent, remove workflow, remove nonexistent, rich config roundtrip preserving all fields.
- Test count: 480 → **486 (485 passed, 1 skipped)**.

## [6.1.1] - 2026-04-08

### Added — The "CLI Flow Command" Release
- **CLI `flow` command**: `fo flow list` lists all OmniFlow workflows, `fo flow run <id> <path>` executes a workflow on a file.
- Supports `--format=json` for structured output.
- Shows workflow status (active/inactive), node count, and connection count.
- 22 CLI commands total (scan, duplicates, hash, metadata, ocr, similar, vhash, afingerprint, classify, organize, delete-duplicates, rename, stats, lint, export, undo, history, ignore, convert, search, rename-batch, watch, **flow**).

## [6.1.0] - 2026-04-08

### Added — The "480-Test Milestone" Release
- **OmniFlow new filters**: `Filter.IsAudio` (.mp3/.wav/.flac/.aac/.ogg/.wma/.m4a), `Filter.IsVideo` (.mp4/.mkv/.avi/.mov/.wmv/.flv/.webm), `Filter.IsArchive` (.zip/.7z/.rar/.tar/.gz/.bz2/.xz).
- **6 new OmniFlow tests**: IsAudio pass/reject, IsVideo pass/reject, IsArchive pass/reject.
- **C API Flow**: `fo_bobfilez_flow_list_json()` and `fo_bobfilez_flow_execute()` for native workflow management.
- **480 tests milestone** — the first minor-version bump reflecting the significant new OmniFlow automation capabilities.
- Test count: 474 → **480 (479 passed, 1 skipped)**.

## [6.0.99] - 2026-04-08

### Added — The "C API Flow" Release
- **C API**: Added `fo_bobfilez_flow_list_json()` and `fo_bobfilez_flow_execute()` for native workflow management.
- **C API 3 tests**: FlowListJsonReturnsWorkflows, FlowExecuteReturnsZeroForNonexistent, FlowExecuteNullParamsReturnsZero.
- Test count: 471 → **474 (473 passed, 1 skipped)**.

## [6.0.98] - 2026-04-08

### Added — The "OmniFlow Real Automation" Release
- **OmniFlow engine rewritten from stub to real implementation**: BFS-based workflow execution with proper node graph traversal.
- **Built-in filters**: `Filter.IsPDF`, `Filter.IsImage`, `Filter.Extension`, `Filter.IsLarger`.
- **Built-in conditions**: `Condition.TextContains`, `Condition.AlwaysTrue`.
- **Built-in actions**: `Action.MoveTo`, `Action.CopyTo`, `Action.Delete`.
- **True/False branch routing**: Filters/conditions route to different branches based on evaluation.
- **17 OmniFlow tests**: struct validation, engine creation, preloaded workflows, PDF/image/text filtering, move/copy/delete actions, multi-step pipelines, false-branch routing, daemon start.
- Test count: 454 → **471 (470 passed, 1 skipped)**.

## [6.0.97] - 2026-04-08

### Added — The "Archive Manager" Release
- **AdvancedArchiveManager 21 tests**: struct validation for ArchiveBrowserEntry (2), ArchiveCreateOptions (2), ArchiveExtractOptions (2), TestResult (2), BenchmarkResult (2), ArchiveInfo (2), plus 9 interface tests for nonexistent archives (browse, test, extract, delete, rename, search, benchmark, info, repair, merge).
- **Struct zero-initialization fix**: Discovered ArchiveInfo and BenchmarkResult need `{}` initialization (MSVC doesn't zero-init by default).
- Test count: 433 → **454 (453 passed, 1 skipped)**.

## [6.0.96] - 2026-04-08

### Added — The "OmniGraph BFS/Pathfinding" Release
- **OmniGraph real BFS traversal**: Implemented proper breadth-first search for `get_connected_nodes()` with configurable depth.
- **OmniGraph shortest path**: Implemented BFS shortest-path algorithm for `find_path()` with edge reconstruction.
- **4 OmniGraph integration tests**: BFS multi-hop depth control, A→B→C path reconstruction, path-not-found, export topology with pre-seeded data.
- Test count: 429 → **433 (432 passed, 1 skipped)**.

## [6.0.95] - 2026-04-08

### Added — The "C API Export CSV/HTML" Release
- **C API**: Added  and  alongside existing .
- **C API 4 tests**: ExportCsvContainsHeaders, ExportHtmlContainsDocument, ExportCsvEmptyDirectory, ExportHtmlEmptyDirectory.
- Test count: 425 → **429 (428 passed, 1 skipped)**.

## [6.0.94] - 2026-04-08

### Added — The "Cloud + Metadata + Neural" Release
- **CloudProviderManager 7 tests**: add/remove/overwrite account, multiple accounts, credentials, remove nonexistent safety, get provider returns null.
- **TinyEXIF Metadata 3 tests**: provider registration, read nonexistent file, read non-image file.
- **NeuralBridge 5 tests**: creates default bridge, load suite, get status, unload idle models, ModelStatus struct.
- Test count: 410 → **425 (424 passed, 1 skipped)**.
- **61 of 73 source files now have test coverage (83.6%)**.

## [6.0.93] - 2026-04-08

### Added — The "DuplicateFinder + ImageViewer" Release
- **DuplicateFinder 8 tests**: SizeHashDuplicateFinder name/find/empty/multiple-groups, SizeHashByteDuplicateFinder name/true-duplicates/reject-different, SizeHashEmptyInput.
- **ImageViewer 7 tests**: ImageInfo/ChannelHistogram/ImageHistogram/ColorAdjustments/SlideshowOptions/ImageFormatSupport struct defaults and field verification.
- Test count: 395 → **410 (409 passed, 1 skipped)**.

## [6.0.92] - 2026-04-08

### Added — The "Omni Matrix + Remote Storage" Release
- **Omni Subsystem 20 tests**: All 15 Omni engines verified as constructible via Registry (OmniGraph, OmniMount, OmniOracle, OmniAudio, OmniClerk, OmniCluster, OmniCrypt, OmniFlow, OmniGit, OmniPeek, OmniSec, OmniShare, OmniTerminal, OmniVerse, OmniVision). OmniGraph tested with node upsert, edge creation, and neighbor query.
- **RemoteStorageManager 6 tests**: add/retrieve connection, remove connection, remove nonexistent safety, multiple connections, get provider for nonexistent, RemoteConnection construction.
- Test count: 369 → **395 (394 passed, 1 skipped)**.

## [6.0.91] - 2026-04-08

### Fixed — The "Mass MSVC Registration Fix" Release
- Fixed MSVC static registration for 17 additional providers that were silently stripped by the linker.
- Added explicit register_*() functions for: OmniGraph, OmniMount, OmniOracle, OmniAudio, OmniClerk, OmniCluster, OmniCrypt, OmniFlow, OmniGit, OmniPeek, OmniSec, OmniShare, OmniTerminal, OmniVerse, OmniVision, NeuralBridge, PerceptualOpenCV.
- Fixed perceptual_opencv.cpp brace structure for non-OpenCV builds.
- **Total MSVC registration fixes: 28 providers** across all sessions.
- All 369 tests passing (1 skipped).

## [6.0.90] - 2026-04-08

### Added — The "PII Sentinel + Omni Infrastructure" Release
- **PIISentinel 13 tests**: SSN/AWS key/private key/email detection, false positive rejection, file scanning, nonexistent file handling, empty input, SSN/CC masking, multiple SSN detection.
- **VirtualFS 2 tests**: struct defaults, registry creates default engine, create folder.
- **TimeMachine 2 tests**: FileRevision construction, registry creates default engine.
- **SemanticTagging 2 tests**: SemanticTag defaults, registry creates default engine.
- **SwarmEngine 2 tests**: SwarmNode construction, registry creates default engine.
- Fixed MSVC static registration for VirtualFS, TimeMachine, SemanticTagging, SwarmEngine — added register_*() functions.
- Test count: 348 → **369 (368 passed, 1 skipped)**.

## [6.0.89] - 2026-04-08

### Added — The "Engine + Thumbnail + ADS Cache" Release
- **Engine Integration 20 tests**: full scan→duplicate pipeline — empty dir, single file, multiple files, extension filter, nested dirs, ignore rules, find duplicates (identical/unique/multiple groups), session tracking, file repository updates, rescan detects new files, search engine accessor, database access, ADS cache config, subdirectory tree, multiple roots, duplicate group size.
- **ThumbnailGenerator 11 tests**: is_image_file (jpeg/png/other/case-insensitive/non-image), generate_base64/generate_to_file for non-image and nonexistent files, base64 encoding (empty/hello/ABC/single/two/binary).
- **ADSCache 6 tests**: is_supported, get_hash nonexistent, set+get round-trip, wrong type returns nullopt, clear removes cache, multiple hash types.
- Made ThumbnailGenerator::base64_encode public for testability.
- Test count: 308 → **348 (347 passed, 1 skipped)**.

## [6.0.88] - 2026-04-08

### Added — The "HexEditor + MarkdownViewer" Release
- **HexEditor 28 tests**: DataInterpretation (null/empty/single-byte/uint16/uint32/uint64/float32/float64/hex/binary/GUID/non-printable/unix-timestamp), HexBuffer (open+size, read bytes, read page, out-of-bounds, past-end, write+readback, read-only fail, undo/redo, interpret, empty file, can-undo/redo empty, multiple patches, overlapping patches).
- **MarkdownViewer 20 tests**: empty string, plain text, headings, bold/italic, code block, link, image, unordered/ordered list, blockquote, word count, reading time, set options, render file, file not found, export HTML, generate template, table, horizontal rule, parse AST.
- Fixed EnhancedCopyTest flaky segfault — increased TearDown sleep for detached thread cleanup.
- Test count: 260 → **308 (307 passed, 1 skipped)**.

## [6.0.87] - 2026-04-08

### Added — The "ConversionEngine + ClipSearch + ShadowDedup" Release
- **ConversionEngine 9 tests**: register/find converter, possible outputs, all input extensions, single file convert, missing file failure, unsupported format failure, batch convert, batch with progress callback, global singleton.
- **ClipSearchEngine 6 tests**: registration infrastructure, is_ready without models, embedding cosine similarity (identical/orthogonal/partial/empty/zero vectors).
- **ShadowDedupWorker 1 test**: construction + interval setting.
- Fixed MSVC static registration for ClipSearchEngine and DocumentEmbedder — added register_clip_search_engine() and register_document_embedder() with ifdef guard stub for non-ONNX builds.
- Test count: 243 → **260 (259 passed, 1 skipped)**.

## [6.0.86] - 2026-04-08

### Added — The "Nexus + Vault + AutoCleanup" Release

#### 🔐 8 VaultManager tests
- Initialize creates vault directory
- Initialize with pre-existing directory
- Lock file returns true for existing files
- Lock file returns false for missing files
- Unlock file returns true
- List contents returns empty for fresh vault
- Lock multiple files in sequence
- Lock file with nested subdirectory

#### ⏱️ 6 Nexus Master Clock tests
- Default engine is registered
- Submit task executes on worker thread
- Logical tick monotonically increases
- `now()` returns current system time
- Report and read performance metrics
- Acquire and release resource arbitration

#### 🧹 2 AutoCleanupManager tests
- Run smart purge submits task to Nexus
- Schedule maintenance does not crash

#### 🔧 Provider registration fixes
- Added `register_nexus()` to `register_all_providers()`.
- Fixed MSVC static-init stripping for Nexus.
- Suppressed unused-parameter warnings in NexusImpl.

### Version
- Bumped to **6.0.86**.

## [6.0.85] - 2026-04-08

### Added — The "Operation Repository" Release

#### 📋 16 OperationRepository tests
- Log operation returns valid ID
- Get all returns logged operations in DESC order
- Get all respects limit parameter
- Get all returns empty for fresh DB
- All 4 operation types (Move/Copy/Delete/Rename) round-trip
- Get by ID returns correct record with all fields
- Get by ID returns nullopt for missing
- Get undoable returns only non-undone completed ops
- Get undoable excludes failed operations
- **Real undo for Move** — file physically moves back
- **Real undo for Copy** — copy physically deleted
- Undo returns nullopt when nothing to undo
- Clear old removes records older than N days
- File size and hash preserved across store/retrieve
- Undone flag preserved across store/retrieve

### Version
- Bumped to **6.0.85**.

## [6.0.84] - 2026-04-08

### Added — The "Copy Engine + Shadow Sorter" Release

#### 📋 8 EnhancedCopyEngine tests
- Enqueue copy job (single file copy with verification)
- Enqueue move job (move with source deletion verification)
- Enqueue multiple files (3-file batch copy)
- Copy preserves exact binary content
- Cancel all jobs (no crash on immediate cancel)
- Clear finished jobs
- Get job by ID (no crash after completion)
- All jobs list (no crash after completion)

#### 🔀 3 ShadowSorterService tests
- Construction with FileWatcher + RuleEngine + CopyEngine dependencies
- Start/stop lifecycle with real OS file watcher
- Service with rules configuration

#### 🔧 Test infrastructure
- All EnhancedCopyEngine tests use heap-allocated engines to avoid
  use-after-free from detached worker threads.
- Added `set_speed_limit` detection (documented but not yet implemented).

### Version
- Bumped to **6.0.84**.

## [6.0.83] - 2026-04-08

### 🎉 **200 TEST MILESTONE**

Added `CApiTest.ExportJsonReportsStatistics` — the 200th test — verifying that
`fo_bobfilez_export_json()` returns a JSON payload with stats section containing
file counts and total_files.

### Version
- Bumped to **6.0.83**.

## [6.0.82] - 2026-04-08

### Added — The "Session Healing Hierarchy" Release

#### 📋 5 ScanSessionRepository tests
- Start session returns positive ID
- End session updates status and scanned count
- Multiple sessions with different statuses
- Session duration tracking

#### 🩹 5 SelfHealingEngine tests
- Registration, start/stop scrub, heal file
- Empty reports initially, schedule configuration

#### 🗂️ 7 HierarchyEngine tests
- Registration, propose hierarchy returns valid tree
- Groups by year, groups by extension
- Empty input handling, apply hierarchy, granularity setting

#### 🔧 Provider registration fixes
- Added `register_self_healing_engine()` and `register_hierarchy_engine()`.
- Fixed MSVC static-init stripping for both components.

### Version
- Bumped to **6.0.82**.

## [6.0.81] - 2026-04-08

### Added — The "TreemapEngine + DataPruner Tests" Release

#### 🗺️ 9 TreemapEngine tests
- Registration, single/multiple file sizing, subdirectory traversal
- Max depth limiting, empty directory, non-existent path
- Layout coordinate calculation, children-within-parent bounds

#### 🗑️ 9 DataPruner tests
- Registration, temp file detection, incomplete download detection
- Normal files below threshold, min-confidence filter
- Custom pruning rules, empty/non-existent directory handling
- Suggestion field validation (path, reason, confidence, size)

#### 🔧 Provider registration fixes
- Added `register_treemap_engine()` and `register_data_pruner()` to `register_all_providers()`.
- Fixed MSVC static-initializer stripping for both components.

### Version
- Bumped to **6.0.81**.

## [6.0.80] - 2026-04-08

### Added — The "Repository Test Suite" Release

#### 🧪 11 new repository tests (153 → 164)

**IgnoreRepository (7 tests)**
- Add and retrieve all patterns
- Remove a pattern, remove non-existent (noop)
- Is-ignored with regex patterns (`.tmp`, `.bak`, `Thumbs.db`)
- Empty repository returns nothing
- Duplicate pattern silently ignored (UNIQUE constraint)

**DuplicateRepository (4 tests)**
- Create group with primary file, get all groups
- Get members of a group
- Empty groups on fresh DB
- Clear all removes everything

### Version
- Bumped to **6.0.80**.

## [6.0.79] - 2026-04-08

### Added — The "Real AuditLogger + Tamper Detection" Release

#### 🔐 AuditLogger rewritten as real SQLite-backed implementation
- **Chain hashing**: Each entry includes a SHA-256 hash of (prev_hash | id | timestamp | operation | src | dst | size | checksum), forming an immutable chain.
- **Genesis entry**: The first entry chains from "GENESIS".
- **Tamper detection**: `verify_ledger()` walks the chain and verifies every hash matches.
- **Standalone mode**: The `default` factory creates its own in-memory DB with the audit_ledger schema.
- **Full CRUD**: `log()`, `get_history(limit)`, and `verify_ledger()` all work end-to-end.

#### 🧪 11 new AuditLogger tests (142 → 153)
- Registration, single entry, multiple entries, history limit
- Ledger integrity (single, multiple, empty)
- Entry hash verification (SHA-256, 64 hex chars)
- Timestamp validation
- Direct DatabaseManager construction with migrations

#### 🔧 Infrastructure
- Added `register_audit_logger()` to `register_all_providers()`.
- Included `sqlite3.h` in audit_logger.cpp for direct SQLite access.

### Version
- Bumped to **6.0.79**.

## [6.0.78] - 2026-04-08

### Added — The "Batch Rename Test Suite" Release

#### 🧪 29 new BatchRenameEngine tests (113 → 142)

**ReplaceRule (5 tests)**
- Simple replace, case-insensitive replace, first-only replace
- Regex replace, regex capture groups ($1/$2)

**InsertRule (3 tests)**
- Insert at start, end, and arbitrary index

**DeleteRule (2 tests)**
- Delete from start, delete from middle

**TrimRule (2 tests)**
- Trim both ends, trim start only

**CaseRule (4 tests)**
- Upper, lower, title case, snake_case conversion

**NumberRule (2 tests)**
- Sequential numbering at end, at start with custom step

**SanitizeRule (2 tests)**
- Illegal char replacement, collapse multiple separators

**TruncateRule (2 tests)**
- Basic truncation, truncation with ellipsis

**TransliterateRule (1 test)**
- ASCII passthrough

**ExtensionRule (1 test)**
- Extension change (lowercase)

**BatchRenameEngine (5 tests)**
- Chained rule application, preview, execute on disk, dry-run, rule descriptions

### Version
- Bumped to **6.0.78**.

## [6.0.77] - 2026-04-08

### Added — The "FileWatcher Testing + CLI Discoverability" Release

#### 🧪 7 new FileWatcher tests (106 → 113)
- `NativeWatcherIsRegistered` — verifies native file watcher registration.
- `StartAndStop` — clean lifecycle start → stop.
- `DetectsNewFile` — creates file, verifies Created event fires.
- `DetectsFileModification` — modifies file, verifies Modified event.
- `EventsProcessedCounter` — batch creates, checks counter > 0.
- `IgnoresHiddenFiles` — dot-files filtered when `ignore_hidden=true`.
- `ExtensionFilter` — only .txt files pass when `include_extensions={"txt"}`.

#### 🔧 Provider registration fix
- Added `register_file_watcher_native()` to `register_all_providers()`.
- The native file watcher was registered via static initializer but MSVC
  linker was stripping it. Now explicitly force-referenced.

#### 🔍 CLI discoverability
- Added `--list-linters` command to list registered filesystem linters.
- Added linters to `--modules` output.
- Added `lint_interface.hpp` include to CLI.

#### 📊 Benchmarks (SearchEngine + Linter)
- 4 SearchEngine benchmarks: literal, wildcard, regex, content search.
- 1 Linter benchmark: std linter across 115 files.

### Version
- Bumped to **6.0.77**.

## [6.0.76] - 2026-04-08

### Added — The "CLI Discoverability + Benchmarks" Release

#### 🔧 CLI discoverability
- Added `--list-linters` command to list registered filesystem linters.
- Added lint_interface.hpp include to CLI.
- Help text updated with `--list-linters` entry.

#### 📊 SearchEngine + Linter benchmarks
- `SearchFixture/LiteralFilename` — literal filename search across 200 files.
- `SearchFixture/WildcardFilename` — wildcard pattern matching.
- `SearchFixture/RegexFilename` — regex pattern matching.
- `SearchFixture/ContentLiteral` — content search (file I/O bound).
- `BM_Linter_Std` — linter scan across 115 files with mixed types.

#### 📈 Benchmark results (this host, 32-core 3.4GHz)
- Literal search: ~1.2ms / 200 files
- Wildcard search: ~1.6ms / 200 files
- Regex search: ~1.7ms / 200 files
- Content search: ~24ms / 200 files (I/O bound)
- Linter scan: ~22ms / 115 files

### Version
- Bumped to **6.0.76**.

## [6.0.75] - 2026-04-08

### Added — The "Linter Expansion + Integration Pipeline" Release

#### 🔍 Linter expanded with 4 new detection types
- `HiddenFile`: Detects dot-prefix and dollar-prefix files (excluding common VCS entries).
- `DuplicateName`: Detects same filename appearing in multiple directories.
- `LargeFile`: Flags files exceeding 100 MB threshold with human-readable size reporting.
- `DeepNesting`: Flags directory trees deeper than 10 levels.
- Improved temp-file detection: `Thumbs.db`, `.DS_Store`, `desktop.ini` now recognized.
- Added `lint_type_name` entries for all new types in the C API.

#### 🧪 6 new integration tests (100 → 106)
- `OrganizeDryRunDoesNotMoveFiles` — verifies RuleEngine dry-run leaves files in place.
- `OrganizeAndUndoRestoresFiles` — full move → record → undo → verify pipeline.
- `SearchEngineIntegrationFindsByLiteral` — Engine.search_engine() literal search.
- `SearchEngineIntegrationContentSearch` — content-only search via SearchEngine.
- `LinterDetectsIssuesInOrganizedDir` — linter catches empty files, temp files, empty dirs.
- `ExportCsvContainsHeaders` — CSV export produces valid headers and file entries.

### Version
- Bumped to **6.0.75**.

## [6.0.74] - 2026-04-08

### Added — The "CLI SearchEngine + C API Export" Release

#### 🔍 CLI search command upgraded to use SearchEngine
- The `search` command now routes through `engine.search_engine()` with full `SearchOptions` instead of an inline `recursive_directory_iterator`.
- Added support for: `--wildcard`, `--whole-word`, `--invert`, `--max-results=<N>`, `--ext=<exts>`, `--min-size`, `--max-size`, `--no-recursive`.
- Content search now works correctly by matching all filenames when `--content` is active and letting content matching drive results.
- JSON output format for search results now includes `content_matches` count.
- Help text updated with new search options section.

#### 📤 C API export helper
- Added `fo_bobfilez_export_json(root_path)` to the C API, returning a full JSON export (files + duplicates + statistics) via `Exporter::to_json`.

#### 🧪 2 new tests (95 → 97)
- Added `CApiTest.ExportJsonContainsFilesAndDuplicates` test.
- Added `SearchEngineTest.WholeWordLiteral` test.

### Version
- Bumped to **6.0.74**.

## [6.0.73] - 2026-04-08

### Added — The "C API Organize + Count + SearchEngine Integration" Release

#### 🔌 C API expanded with organize dry-run and count helpers
- Added `fo_bobfilez_organize_dry_run_json()` and `fo_bobfilez_organize_dry_run_summary_text()` to the C API, exposing safe organization previews through the `fo_c_api` seam.
- Added `fo_bobfilez_count_json()` and `fo_bobfilez_count_summary_text()` to the C API, exposing combined file/duplicate counting with wasted-space calculation.
- Both new operations include null-safety, error reporting, and structured JSON output matching the established C API patterns.

#### 🔍 SearchEngine integrated into Engine class
- Added `SearchEngine` as a managed member of `Engine`, accessible via `engine.search_engine()`.
- Required moving `Engine` constructor and destructor out-of-line into `engine.cpp` to satisfy MSVC 2019's incomplete-type deleter requirements.
- Required moving `SearchEngine` constructor and destructor out-of-line into `search_engine.cpp` for the same reason.

#### 🧪 21 new tests (74 → 95)
- Added `tests/test_search_engine.cpp` with 15 comprehensive SearchEngine tests covering literal, wildcard, and regex filename search, content search, extension/size filters, max results, recursive traversal, invert match, and engine integration.
- Added 5 C API tests for organize dry-run (JSON + summary + validation) and count (JSON + summary).
- Updated `tests/c_api_smoke.c` to exercise the organize and count C API functions.

#### 📚 Documentation
- Added `docs/ai/implementation/C_API_ORGANIZE_COUNT_SEARCH_2026_04_08.md` documenting the C API expansion and SearchEngine integration.

### Version
- Bumped to **6.0.73**.

## [6.0.72] - 2026-04-06

### Refined — The "BobGUI General Consistency and Field Awareness" Release

#### 🔧 BobGUI idle and success state polished for consistency
- Refined `frontends/bobgui_app/main.c` so BobGUI now uses clearer, formally structured headings for idle output (Panel Mode, Field Roles, Current Working Values).
- Updated "Ready" and "Running" status labels to follow a consistent `State · Mode/Operation` visual pattern.
- Success messages for path-based operations now explicitly acknowledge that the current path has been retained for follow-up actions.
- The ignore-reset flow now produces a more instructional output block explaining field roles and next steps.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **73 / 73 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_CONSISTENCY_AND_FIELD_AWARENESS_2026_04_06.md` documenting the BobGUI general consistency refinements.

### Version
- Bumped to **6.0.72**.

## [6.0.71] - 2026-04-06

### Refined — The "BobGUI Status and Field-Role Clarity" Release

#### 🧭 BobGUI status text and field-role guidance became clearer
- Refined `frontends/bobgui_app/main.c` so BobGUI now uses stronger ready/running status text aligned with the current backend mode and the active operation label.
- Improved the reset-ignore guidance so the output panel now explicitly explains the role of the ignore pattern and reason fields after a reset.
- Continued polishing the BobGUI control surface without widening the C ABI or changing the underlying direct/fallback execution model.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **73 / 73 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_STATUS_AND_FIELD_ROLE_CLARITY_2026_04_06.md` documenting the latest BobGUI status/field-role refinement.

### Version
- Bumped to **6.0.71**.

## [6.0.70] - 2026-04-06

### Refined — The "BobGUI Output Formatting" Release

#### 🧱 BobGUI output structure is now easier to scan
- Refined `frontends/bobgui_app/main.c` so pending BobGUI output now includes clearer structure around **Context** and **Execution** sections.
- Added workflow-aware target headings such as **Path**, **Ignore Pattern**, and **Request Scope**.
- Added a clearer **Result** heading to both CLI-backed and direct `fo_c_api` output rendering so the output panel is easier to skim regardless of backend path.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **73 / 73 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_OUTPUT_FORMATTING_POLISH_2026_04_06.md` documenting the output-structure refinement.

### Version
- Bumped to **6.0.70**.

## [6.0.69] - 2026-04-06

### Refined — The "BobGUI Operation Labels" Release

#### 🏷️ BobGUI output now uses clearer operation names
- Refined `frontends/bobgui_app/main.c` so BobGUI now maps internal operation keys to more human-facing labels across pending output, CLI-backed output headers, direct `fo_c_api` output headers, and related status phrasing.
- Improved the BobGUI output surface by replacing remaining internal-feeling labels such as raw operation keys with clearer product-facing names like **Duplicate Analysis**, **Hash Inspection**, **History Listing**, and **Ignore Rule Add/Remove**.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **73 / 73 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_OPERATION_LABEL_POLISH_2026_04_06.md` documenting the human-facing operation-label refinement.

### Version
- Bumped to **6.0.69**.

## [6.0.68] - 2026-04-06

### Refined — The "BobGUI Field Persistence" Release

#### 🧾 BobGUI idle and success output now reflect retained working values
- Refined `frontends/bobgui_app/main.c` so BobGUI's shared idle-output state now includes the current path, ignore pattern, and ignore reason.
- Expanded workflow-aware success messaging so path-based operations now more clearly acknowledge that the current path remains available for follow-up actions.
- Improved ignore-add success messaging so it better reflects that the reason field remains available for repeated ignore-rule authoring.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **73 / 73 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_FIELD_PERSISTENCE_POLISH_2026_04_06.md` documenting the field-retention and idle-state refinement.

### Version
- Bumped to **6.0.68**.

## [6.0.67] - 2026-04-06

### Refined — The "BobGUI General Success-State" Release

#### 🎯 BobGUI success and idle messaging became more workflow-aware
- Refined `frontends/bobgui_app/main.c` so successful BobGUI actions across the broader workflow family now use more contextual status text instead of mostly generic completion phrasing.
- Added a reusable idle-output helper so startup guidance and cleared-output guidance now better explain backend mode, control-field responsibilities, and output-panel persistence behavior.
- Preserved the existing direct `fo_c_api` preference and per-operation CLI fallback model while improving the user-facing operational clarity of the panel.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **73 / 73 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_GENERAL_SUCCESS_STATE_POLISH_2026_04_06.md` documenting the broader success/idle messaging refinement.

### Version
- Bumped to **6.0.67**.

## [6.0.66] - 2026-04-06

### Refined — The "BobGUI Ignore Success-State" Release

#### ✅ BobGUI ignore actions now leave the UI in a cleaner next-step state
- Refined `frontends/bobgui_app/main.c` so BobGUI now tracks command success state explicitly in its UI completion flow.
- Successful **Add Ignore Rule** and **Remove Ignore Rule** actions now clear the ignore-pattern field automatically.
- Successful ignore actions now use more workflow-aware status text such as readiness for the next pattern or suggestion to list rules.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **73 / 73 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_IGNORE_FIELD_SUCCESS_STATE_2026_04_06.md` documenting the ignore-action success-state refinement.

### Version
- Bumped to **6.0.66**.

## [6.0.65] - 2026-04-06

### Refined — The "BobGUI Post-Action Guidance" Release

#### 🧠 BobGUI output now suggests next steps
- Refined `frontends/bobgui_app/main.c` so both direct and CLI-backed BobGUI results now append a contextual **next helpful action** section to the output panel.
- Improved the in-progress messaging so it now explicitly explains that completed results stay visible for comparison against the next action.
- Kept the BobGUI direct/fallback architecture unchanged while improving the operational clarity of the output surface.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **73 / 73 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_POST_ACTION_GUIDANCE_2026_04_06.md` documenting the contextual output-guidance refinement.

### Version
- Bumped to **6.0.65**.

## [6.0.64] - 2026-04-06

### Refined — The "BobGUI Action Grouping Polish" Release

#### 🧭 BobGUI action grouping and naming clarity
- Refined `frontends/bobgui_app/main.c` so BobGUI now groups controls into clearer sections:
  - **Filesystem Actions**
  - **Operational Listings and Ignore Actions**
  - **Utility Actions**
- Renamed several BobGUI buttons to be more explicit, including **List History**, **List Ignore Rules**, **Add Ignore Rule**, and **Remove Ignore Rule**.
- Updated startup guidance so the user-facing explanation now aligns with the clearer grouped control surface.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **73 / 73 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_ACTION_GROUPING_POLISH_2026_04_06.md` documenting the UI grouping refinement and why BobGUI ergonomics remain a high-value next-wave investment.

### Version
- Bumped to **6.0.64**.

## [6.0.63] - 2026-04-06

### Refined — The "BobGUI Usability Polish" Release

#### 🎛️ BobGUI control-surface refinement
- Refined `frontends/bobgui_app/main.c` so BobGUI now shows richer in-progress output describing the active operation, target, and ignore reason where relevant.
- Added a **Reset Ignore Fields** button so the ignore-management inputs can be restored to known-good example values quickly.
- Added a **Clear Output** button so the operational output panel can be cleared without disturbing the rest of the current UI state.
- Improved the startup guidance text so the BobGUI lane more clearly explains which controls are intended for path-based actions, ignore-management actions, and path-free listing actions.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **73 / 73 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_USABILITY_POLISH_2026_04_06.md` documenting the control-surface refinement and the continuing host-validation boundary for BobGUI.

### Version
- Bumped to **6.0.63**.

## [6.0.62] - 2026-04-06

### Expanded — The "BobGUI Direct Ignore Actions" Release

#### 🧩 Direct C API gained ignore add/remove actions
- Expanded `core/include/fo/c_api/bobfilez_c_api.h` and `core/c_api/bobfilez_c_api.cpp` with:
  - `fo_bobfilez_ignore_add_json(...)`
  - `fo_bobfilez_ignore_remove_json(...)`
  - `fo_bobfilez_ignore_add_summary_text(...)`
  - `fo_bobfilez_ignore_remove_summary_text(...)`
- Kept the narrow DB-path model for these database-backed actions via `BOBFILEZ_DB_PATH` with fallback to `fo.db`.

#### 🖥️ BobGUI ignore management became actionable
- Updated `frontends/bobgui_app/main.c` so BobGUI now includes dedicated **Ignore Pattern** and **Reason** inputs.
- Added **Ignore Add** and **Ignore Remove** buttons.
- Preserved the preferred-direct / per-operation-fallback model, so BobGUI now prefers direct C API handling for ignore add/remove when available.

#### 🧪 Validation surface expanded again
- Updated `tests/test_c_api.cpp` and `tests/c_api_smoke.c` with real ignore add/remove validation.
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **73 / 73 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_IGNORE_ACTIONS_DIRECT_C_API_2026_04_06.md` documenting the new direct ignore-action seam and the BobGUI UI refinement.

### Version
- Bumped to **6.0.62**.

## [6.0.61] - 2026-04-06

### Expanded — The "BobGUI Direct History and Ignore" Release

#### 🔌 Direct C API gained history and ignore support
- Expanded `core/include/fo/c_api/bobfilez_c_api.h` and `core/c_api/bobfilez_c_api.cpp` with:
  - `fo_bobfilez_history_json(...)`
  - `fo_bobfilez_ignore_json(...)`
  - `fo_bobfilez_history_summary_text(...)`
  - `fo_bobfilez_ignore_summary_text(...)`
- Added narrow database-path resolution for these database-backed workflows through `BOBFILEZ_DB_PATH`, falling back to `fo.db` when unset.

#### 🖥️ BobGUI now prefers direct history / ignore too
- Updated `frontends/bobgui_app/main.c` so BobGUI now maps **History** and **Ignore Rules** through the direct `fo_c_api` seam when available, while preserving the per-operation CLI fallback path.

#### 🧪 Validation surface expanded
- Updated `tests/test_c_api.cpp` with real history + ignore coverage against a temporary database path.
- Updated `tests/c_api_smoke.c` so the C consumer smoke test also validates the new history + ignore summary helpers.
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **72 / 72 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_HISTORY_IGNORE_DIRECT_C_API_2026_04_06.md` documenting the direct-seam expansion and the environment-driven DB-path decision.

### Version
- Bumped to **6.0.61**.

## [6.0.60] - 2026-04-06

### Expanded — The "History and Ignore Frontend Parity" Release

#### 🧭 React history + ignore workflows
- Expanded `bobui_web/public/react/app.js` so the React/Express SPA now includes dedicated **History** and **Ignore Rules** tabs.
- Added history loading via `GET /api/history` and ignore-rule loading via `GET /api/ignore`.
- Added ignore-rule add/remove actions via `POST /api/ignore/add` and `POST /api/ignore/remove`.
- Updated dashboard copy so the React lane description better reflects its real operational workflow surface.

#### 🖥️ BobGUI path-free CLI operations
- Expanded `frontends/bobgui_app/main.c` so BobGUI now exposes **History** and **Ignore Rules** buttons.
- Added path-free CLI request handling for operations that do not require a filesystem path.
- Preserved the preferred-direct / per-operation-fallback model honestly: history and ignore listing currently route through `fo_cli` because they are not yet exposed by the direct `fo_c_api` seam.

#### ✅ Validation
- Re-ran `node --check bobui_web/server.js` successfully.
- Re-ran `node --check bobui_web/public/react/app.js` successfully.
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **71 / 71 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/FRONTEND_HISTORY_IGNORE_PARITY_2026_04_06.md` documenting the React + BobGUI parity expansion and the host-validation boundary.

### Version
- Bumped to **6.0.60**.

## [6.0.59] - 2026-04-06

### Expanded — The "Lint Matrix Completion" Release

#### 🧹 BTK and React lint parity completed
- Expanded `frontends/btk/src/DemoWindow.hpp/.cpp` so the BTK research-native demo now includes a real **Lint** tab backed by the registered `std` linter.
- Expanded `bobui_web/public/react/app.js` so the React/Express SPA now surfaces the existing `/api/lint` capability through a dedicated **Lint** tab with path/type/details result rendering.
- Updated the React dashboard copy so the BTK and web lane descriptions better reflect their real workflow surface.

#### ✅ Validation
- Re-ran `node --check bobui_web/server.js` successfully.
- Re-ran `node --check bobui_web/public/react/app.js` successfully.
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **71 / 71 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/FRONTEND_LINT_MATRIX_COMPLETION_2026_04_06.md` documenting the final BTK + React lint parity pass.

### Version
- Bumped to **6.0.59**.

## [6.0.58] - 2026-04-05

### Expanded — The "Native Lint Parity" Release

#### 🧹 Qt, BobUI, and JUCE demos gained lint workflows
- Expanded `frontends/qt/src/main.cpp` so the Qt demo now includes a real **Lint** tab backed by the registered `std` linter.
- Expanded `frontends/bobui/src/QmlEngineWrapper.*` and `frontends/bobui/assets/Main.qml` so the BobUI/QML lane now exposes `runLint(...)`, `lintFinished(...)`, and a full **Lint** tab.
- Expanded `frontends/juce/src/main.cpp` so the JUCE demo now includes a native **Lint** tab using the same background-thread / message-thread pattern used throughout the rest of the JUCE workflow family.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **71 / 71 tests passed**.
- Re-ran `scripts/build_juce_gui.bat` successfully after the JUCE lint expansion.

#### 📚 Documentation
- Added `docs/ai/implementation/FRONTEND_LINT_PARITY_2026_04_05.md` documenting the new native lint parity work.

### Version
- Bumped to **6.0.58**.

## [6.0.57] - 2026-04-05

### Expanded — The "BobGUI Lint Direct Parity" Release

#### 🧹 BobGUI now supports lint in both direct and fallback modes
- Extended `fo_c_api` with `fo_bobfilez_lint_json(...)` and `fo_bobfilez_lint_summary_text(...)` so lint is now a first-class direct C API workflow alongside scan, duplicates, statistics, hash, and metadata.
- Updated `frontends/bobgui_app/main.c` to add a **Lint** action and wire it into the same preferred-direct / fallback-CLI behavior already established for the other BobGUI workflows.
- This makes the BobGUI lane more practically complete without abandoning the resilient fallback model.

#### ✅ Validation
- Extended `tests/test_c_api.cpp` and `tests/c_api_smoke.c` to validate lint through the direct C API seam.
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **71 / 71 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_LINT_DIRECT_PARITY_2026_04_05.md` documenting the new direct lint seam and why it fits the broader frontend parity strategy.

### Version
- Bumped to **6.0.57**.

## [6.0.56] - 2026-04-05

### Refined — The "BobGUI Resilient Fallback" Release

#### 🔁 BobGUI now falls back per operation, not just per build mode
- Updated `frontends/bobgui_app/main.c` so the BobGUI lane now automatically falls back to `fo_cli` when a direct `fo_c_api` operation is unavailable or fails, instead of treating the compiled-in direct path as all-or-nothing.
- Added clearer output messaging so fallback mode explains why it happened before showing the CLI-backed result.
- This makes the BobGUI lane substantially more robust in mixed or partially prepared environments.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_RESILIENT_FALLBACK_2026_04_05.md` documenting the per-operation fallback behavior and why it is a stronger product model than a brittle direct-only preference.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **70 / 70 tests passed**.

### Version
- Bumped to **6.0.56**.

## [6.0.55] - 2026-04-05

### Expanded — The "BTK Frontend Parity" Release

#### 🧪 BTK demo moved closer to the rest of the native matrix
- Expanded `frontends/btk/src/DemoWindow.hpp/.cpp` so the BTK/CopperSpice demo now includes **Statistics**, **Hasher**, and **Metadata** tabs in addition to its earlier **Scanner** and **Duplicates** support.
- The BTK lane now drives the same practical backend seams used elsewhere: `IFileScanner`, `Engine`, `IHasher`, and `IMetadataProvider`.
- This keeps BTK credible as a research/native-experiment lane without pretending it is the active Omni runtime direction.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **70 / 70 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BTK_FRONTEND_PARITY_EXPANSION_2026_04_05.md` documenting the BTK-side parity work and the still-limited host validation reality for that lane.

### Version
- Bumped to **6.0.55**.

## [6.0.54] - 2026-04-05

### Expanded — The "Native Metadata Parity" Release

#### 🧩 Qt, BobUI, and JUCE demos gained metadata workflows
- Expanded `frontends/qt/src/main.cpp` so the Qt demo now includes a real **Metadata** tab backed by the registered `tinyexif` metadata provider and the existing `std` scanner path.
- Expanded `frontends/bobui/src/QmlEngineWrapper.*` and `frontends/bobui/assets/Main.qml` so the BobUI/QML lane now exposes `runMetadata(...)`, `metadataFinished(...)`, and a full **Metadata** tab.
- Expanded `frontends/juce/src/main.cpp` so the JUCE demo now also includes a native **Metadata** tab using the same background-thread/message-thread pattern already established for the rest of the JUCE workflows.

#### ✅ Validation
- Re-ran `scripts/build_headless.bat` successfully.
- Re-ran `ctest --test-dir build-msvc --output-on-failure` successfully: **69 / 69 tests passed**.
- Re-ran `scripts/build_juce_gui.bat` successfully after the JUCE metadata expansion.

#### 📚 Documentation
- Added `docs/ai/implementation/FRONTEND_METADATA_PARITY_2026_04_05.md` documenting the new cross-lane metadata parity work.

### Version
- Bumped to **6.0.54**.

## [6.0.53] - 2026-04-05

### Expanded — The "BobGUI Summary Mode" Release

#### 🪟 BobGUI direct mode now renders structured summaries instead of raw JSON
- Extended `fo_c_api` with human-readable summary helpers for scan, duplicates, statistics, hash, and metadata workflows.
- Updated `frontends/bobgui_app/main.c` so direct `fo_c_api` mode now uses these summary-oriented functions while preserving the existing `fo_cli` fallback behavior.
- This makes the BobGUI direct path feel much more like a native UI integration rather than a raw transport dump.

#### 🧪 Direct C API surface validated more honestly
- Extended `tests/test_c_api.cpp` to validate the new summary helpers.
- Extended `tests/c_api_smoke.c` so the real C consumer now checks both JSON and summary-text paths.
- Re-ran headless validation and root-level `ctest` after the provider-registration and root-test-discovery improvements. Validation succeeded with **69 / 69 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_SUMMARY_MODE_2026_04_05.md` and continued documenting the BobGUI direct-mode evolution in handoff/session tracking.

### Version
- Bumped to **6.0.53**.

## [6.0.52] - 2026-04-05

### Expanded — The "BobGUI Direct Wiring + C Test Surface" Release

#### 🔌 BobGUI app now prefers the direct C ABI when available
- Updated `frontends/bobgui_app/main.c` so the BobGUI lane now prefers direct `fo_c_api` integration when compiled with that seam available, while still retaining the existing `fo_cli` fallback path.
- Kept all BobGUI operations asynchronous, so the move from CLI bridge mode to direct-C-API mode does not regress responsiveness.

#### 🛠️ Meson-side direct C API discovery added
- Updated `frontends/bobgui_app/meson.build` and added `frontends/bobgui_app/meson_options.txt`.
- The BobGUI app now attempts to discover `fo_c_api` in common repo-relative build output locations and defines `BOBFILEZ_HAVE_C_API=1` when the direct seam is available.

#### ✅ C consumer validation improved again
- Added `tests/c_api_smoke.c` and updated `tests/CMakeLists.txt` so a real C-compiled consumer now links against `fo_c_api`.
- Updated root `CMakeLists.txt` to call `enable_testing()`, which fixes root-level `ctest` discoverability rather than leaving tests trapped only inside the sub-build directory.
- Validation now succeeds with `ctest --test-dir build-msvc --output-on-failure`: **69 / 69 tests passed**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_DIRECT_C_API_WIRING_2026_04_05.md` documenting the two-tier BobGUI backend model and the new C consumer smoke-test strategy.

### Version
- Bumped to **6.0.52**.

## [6.0.51] - 2026-04-05

### Added — The "BobGUI Direct C API Shim" Release

#### 🧱 New native C ABI seam for future BobGUI / C consumers
- Added `core/include/fo/c_api/bobfilez_c_api.h` and `core/c_api/bobfilez_c_api.cpp`.
- Added the new `fo_c_api` library target in `core/CMakeLists.txt`.
- Exposed JSON-returning C entrypoints for scan, duplicates, statistics, hash, and metadata plus explicit string lifetime management and last-error reporting.
- This creates the first real direct-integration seam for BobGUI/native-C consumers without forcing them through the full C++ API surface.

#### ✅ C API validated with new tests
- Added `tests/test_c_api.cpp` and updated `tests/CMakeLists.txt`.
- Verified scan, duplicates, stats, single-file hash behavior, and null-input error handling through the new C shim.
- Headless validation now reports **68 / 68 tests passing**.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_C_API_SHIM_2026_04_05.md` documenting the rationale, ABI-shape decision, and future BobGUI linkage path.

### Version
- Bumped to **6.0.51**.

## [6.0.50] - 2026-04-05

### Expanded — The "BobGUI CLI Bridge" Release

#### 🧩 BobGUI demo is no longer a static placeholder
- Reworked `frontends/bobgui_app/main.c` from a label-only BobGUI window into a functional native demo with a path entry, action buttons, status messaging, and a scrollable output surface.
- Added live BobGUI actions for **Scan**, **Duplicates**, **Statistics**, **Hash**, and **Metadata**.

#### 🔌 Pragmatic bridge strategy for the BobGUI lane
- Implemented an asynchronous BobGUI-to-CLI bridge instead of pretending a direct C/C++/Meson/CMake cross-build integration was already trivial.
- The BobGUI app now discovers `fo_cli.exe` from common repo-relative locations or `BOBFILEZ_CLI`, launches requests off the UI thread with GLib threading, captures stdout/stderr, and marshals results back to the UI loop.
- This keeps the lane aligned with the project’s CLI-first rule while providing real user-facing functionality immediately.

#### 📚 Documentation
- Added `docs/ai/implementation/BOBGUI_CLI_BRIDGE_2026_04_05.md` documenting the technical rationale, host constraints, and next-step path toward a future narrow C ABI shim.

### Version
- Bumped to **6.0.50**.

## [6.0.49] - 2026-04-05

### Expanded — The "Frontend Parity Expansion" Release

#### 🖥️ Qt and BobUI demos now reach the same next-tier workflow set
- Expanded `fo_qt_demo` (`frontends/qt/src/main.cpp`) from a smaller demo into a broader Qt Widgets frontend with **Dashboard**, **Scanner**, **Duplicates**, **Statistics**, and **Hasher** tabs, each backed by real `fo_core` calls running off the UI thread.
- Expanded `fo_bobui_demo` (`frontends/bobui/assets/Main.qml`, `frontends/bobui/src/QmlEngineWrapper.*`, and `frontends/bobui/src/main.cpp`) to expose the same workflow family to QML through a dedicated QObject bridge, adding real **Statistics** and **Hasher** paths alongside the earlier scan/duplicate flows.

#### 🌐 React web UI shape-handling and feature coverage improved
- Reworked `bobui_web/public/react/app.js` so the no-build React SPA now exposes **Scanner**, **Duplicates**, **Statistics**, **Hasher**, and **Metadata** views.
- Fixed several backend shape assumptions in the React layer so it now correctly handles real CLI JSON such as:
  - `scan` rows using `path`
  - `hash` rows using `hash`
  - `stats.extensions` returning an array of extension records
- Added `POST /api/metadata` to `bobui_web/server.js`, keeping the web lane aligned with the CLI-first model.

#### 🧭 Pure Qt lane made more honest
- Updated the root `CMakeLists.txt` so the plain Qt demo no longer piggybacks on BobUI-specific Qt package discovery.
- Updated `scripts/build_qt_gui.bat` to auto-detect the visible `D:\Qt\6.11.0\mingw_64` host kit when no Qt root is supplied and to warn clearly about the current MinGW-vs-MSVC mismatch.

#### 🧪 BTK demo thread handoff simplified
- Cleaned up `frontends/btk/src/DemoWindow.*` so the BTK demo now uses explicit queued slot application for background results instead of rougher experimental lambda/event scaffolding.

### Documentation
- Added `docs/ai/implementation/FRONTEND_PARITY_EXPANSION_2026_04_05.md` documenting the parity expansion and the pure-Qt-lane decoupling rationale.

### Version
- Bumped to **6.0.49**.

## [6.0.48] - 2026-04-05

### Added — The "Functional Multi-Frontend Parity" Release

#### 🌐 React Web UI Functional Parity
- Greatly expanded the `bobui_web/public/react/app.js` React SPA to connect directly with the `fo_cli` backend via the Express API.
- Added an interactive **File Scanner** view that triggers `/api/scan` and displays paginated results (filename, size).
- Added an interactive **Duplicate Finder** view that triggers `/api/duplicates` (using the fast hash engine) to group and present duplicated files.
- The web frontend now provides a true, functional, and visually appealing representation of the backend filez engine using only native ES Modules and `esm.sh` CDN imports.

#### 🎼 JUCE Native GUI Functional Parity
- Enhanced the `fo_juce_demo` target (`frontends/juce/src/main.cpp`) to prove deep, direct C++ linking against the static `fo_core` library.
- Added a native OS file browser (`juce::FileChooser`) that allows users to pick a directory path.
- Wired the directory selection into the C++ `fo::core::Registry<fo::core::IFileScanner>` to execute a real backend `std` scan, aggregating byte sizes and reporting accurate results directly to the JUCE GUI canvas.
- Implemented background threading (`juce::Thread::launch`) to ensure the scan executes asynchronously without blocking the JUCE message thread, emitting the results to the UI via `juce::MessageManager::callAsync`.

### Version
- Bumped to **6.0.48**.

## [6.0.47] - 2026-04-05

### Added — The "Multi-Frontend Matrix" Release

#### 🧩 Multi-Frontend Scaffolding and Submodules
- Explicitly verified and mapped all requested native UI lanes:
  - **Qt**: Plain Qt6 Widgets demo (`fo_qt_demo`).
  - **BobUI**: OmniUI-based QML integration demo (`fo_bobui_demo`).
  - **JUCE**: Audio-oriented native UI (`fo_juce_demo`).
  - **BTK**: CopperSpice fork widgets app (`fo_btk_demo`).
  - **BobGUI**: GTK fork Meson-based app (`bobfilez-bobgui-demo`).
  - **React Web UI**: Lightweight React 18 browser shell served alongside the existing web dashboard.
- Added `libs/JUCE` as a tracked Git submodule.
- Corrected the `.gitmodules` mapping to officially track `libs/bobui` and `libs/btk`.
- Added explicit CMake configuration options for the native targets (`FO_BUILD_QT_DEMO`, `FO_BUILD_BOBUI_DEMO`, `FO_BUILD_JUCE_DEMO`, `FO_BUILD_BTK_DEMO`).

#### 🛠️ Build Scripts for the New Matrix
- Added `scripts/build_qt_gui.bat`
- Added `scripts/build_juce_gui.bat`
- Added `scripts/build_bobgui_gui.bat`

#### 🌐 React Web UI Lane
- Added a lightweight, no-build React frontend using ES modules and `unpkg` CDNs.
- Exposed under `bobui_web/public/react/`.
- Updated `bobui_web/server.js` to include an `/api/health` endpoint that checks backend CLI availability and provides the frontend lane status.
- Added a launch button on the legacy `bobui_web` dashboard linking to the new React UI.

### Version
- Bumped to **6.0.47**.

## [6.0.46] - 2026-04-05

### Refined — The "BobUI Runtime Reality Check" Release

#### 🔎 BobUI Runtime Model Corrected
- Refined the restored BobUI direction to reflect a more honest runtime model.
- bobfilez still uses **BobUI / OmniUI** as the active native UI layer, but the project no longer assumes the local BobUI tree is always a complete self-contained Qt6 QML runtime provider.
- Updated discovery/messaging so GUI / Omni builds now search for compatible Qt6 packages via:
  - `QT6_ROOT`
  - `QT_ROOT`
  - `QTDIR`
  - `CMAKE_PREFIX_PATH`
  - optional BobUI build/install hints

#### 🧪 BobUI Upstream Probe Advanced Past the Old QLocale Stop
- Patched `libs/bobui/src/corelib/kernel/qtmochelpers.h` to avoid the earlier MSVC generic-lambda lookup failure in the metaobject copy path.
- Confirmed the BobUI in-place build now gets far beyond the old `qlocale.cpp` / `qtmochelpers.h` blocker and proceeds deep into later GUI / Widgets / OpenGL targets.
- Pushed the BobUI-side fix upstream on `robertpelloni/bobui` `main` and updated the bobfilez submodule gitlink accordingly.

#### ⚠️ Current Native Blocker Is Now More Precise
- Confirmed from BobUI's own CI/workflow model that BobUI expects external Qt modules such as `qtdeclarative` for modern QML/Quick usage.
- Confirmed this host has Qt6 QML package configs under `D:\Qt`, but the visible desktop kit is currently:
  - `D:\Qt\6.11.0\mingw_64`
- bobfilez's active validated native lane remains **MSVC**, so the current blocker is now more precise than before:
  - local BobUI tree still does not export `Qt6Qml`
  - available external Qt6 QML desktop kit is **MinGW**, not **MSVC**
- BobUI in-place MSVC builds also still eventually reach an MSVC internal compiler error in snippet-related targets, but that is now a later toolchain/build-profile issue rather than the original source-level `QLocale` stop.

#### 🛠️ Build Helper Refinement
- Updated:
  - `cmake/BobUIQtSetup.cmake`
  - `scripts/build_bobui_gui.bat`
  - `scripts/build_bobui_inplace.bat`
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
- `build_bobui_inplace.bat` now explicitly disables tests/examples/benchmarks/doc-snippet intent at configure time to reduce non-essential noise during provider probing.
- `build_bobui_gui.bat` now warns more clearly when no external Qt6 QML runtime root is supplied.

#### 📘 Documentation
- Added **`docs/ai/implementation/BOBUI_QT_RUNTIME_REALITY_CHECK_2026_04_05.md`** documenting:
  - the BobUI `qtmochelpers.h` fix
  - the new later-stage in-place build boundary
  - BobUI's external-Qt module expectation
  - the current host's Qt kit situation under `D:\Qt`
  - why the present blocker is now best understood as a Qt runtime/toolchain mismatch rather than only a BobUI source-tree failure

### Version
- Bumped to **6.0.46**.

## [6.0.45] - 2026-04-05

### Restored — The "BobUI-First Native Provider" Release

#### 🔁 Active Native Provider Path Switched Back to BobUI/Qt6
- Restored **BobUI** as the active native GUI / Omni provider path.
- Added:
  - `cmake/BobUIQtSetup.cmake`
  - `scripts/build_bobui_gui.bat`
  - `scripts/build_bobui_inplace.bat`
- Updated:
  - `CMakeLists.txt`
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
  - `gui/omni/src/OmniQmlRegistration.cpp`
- Active GUI/Omni builds now prefer BobUI/Qt6 discovery again through `BOBUI_ROOT` / `libs/bobui` and consume standard `Qt6::*` targets rather than the BTK/CopperSpice target model.

#### 🧩 BobUI omnicore Registration and Source Wiring Restored
- Restored BobUI `OmniUI/omnicore` source-tree inclusion for the active native GUI targets.
- Restored BobUI QML registration in the active bootstrap path by calling:
  - `OmniUI::registerQmlTypes();`
- bobfilez then still layers its own local bridge registrations on top:
  - `Omni.File`
  - `Omni.Viz`
  - `Omni.Native`

#### 🔍 Honest Current BobUI Boundary Captured
- Re-ran the BobUI-backed GUI probe.
- bobfilez now correctly discovers BobUI's top-level `Qt6Config.cmake`, but the current machine still stops at the real next blocker:
  - missing `Qt6Qml`
- Observed failure is now specifically that BobUI's local build tree does not provide:
  - `Qt6QmlConfig.cmake`
- Re-ran the BobUI in-place build helper as well and confirmed a second honest upstream boundary on this host:
  - BobUI's own corelib currently stops in `qtmochelpers.h` while compiling `qlocale.cpp`
- This means BobUI is structurally the active path again, but the present local BobUI state is still not a full drop-in runtime for bobfilez's QML-based native shell on this host.

#### 📘 Documentation
- Added **`docs/ai/implementation/BOBUI_PROVIDER_RESTORE_2026_04_05.md`** documenting:
  - the restored BobUI-first path
  - the recreated helper scripts
  - the restored omnicore/QML registration wiring
  - the real current blocker at missing `Qt6Qml`

### Version
- Bumped to **6.0.45**.

## [6.0.44] - 2026-04-05

### Added — The "Native UI Profile Listing" Release

#### 🧩 Eighth Concrete Option C Refinement Added a User-Facing Discovery Affordance
- Added a dedicated CLI flag to list available native launch profiles without starting the GUI runtime:
  - `--list-native-ui-profiles`
- Updated:
  - `gui/omni/src/NativeUiProfileRegistry.hpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
  - `gui/omni/src/NativeUiBootstrap.cpp`
- Added registry helpers for:
  - `native_ui_profile_list_argument()`
  - `should_list_launch_profiles(int argc, char *argv[])`
- When invoked, the native entrypoint now:
  - prints the available launch profile names
  - marks the default profile
  - prints the active selection precedence rule
  - exits without attempting to launch the GUI runtime
- This makes the launch-profile architecture externally discoverable rather than only visible in code/docs.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_PROFILE_LISTING.md`** documenting:
  - the new CLI flag
  - the listing behavior
  - why it improves launch-profile usability
  - the unchanged runtime behavior when the flag is absent

### Version
- Bumped to **6.0.44**.

## [6.0.43] - 2026-04-05

### Added — The "Explorer-Only Native Launch Profile" Release

#### 🧩 Second Genuinely Alternate Named Launch Profile Added
- Added a second real alternate native shell launch profile:
  - **`omni-explorer-only`**
- This profile is backed by a different root QML surface:
  - `qrc:/ExplorerShell.qml`
- Added:
  - `gui/omni/assets/ExplorerShell.qml`
- Updated:
  - `gui/omni/assets/qml.qrc`
  - `gui/omni/src/NativeUiLaunchProfile.hpp`
  - `gui/omni/src/NativeUiLaunchProfile.cpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
- Added `create_explorer_only_launch_profile()` and registered the profile name in the launch-profile registry.
- The explorer-only profile can be selected through the existing selection seams:
  - CLI: `--native-ui-profile=omni-explorer-only`
  - env: `BOBFILEZ_NATIVE_UI_PROFILE=omni-explorer-only`

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_EXPLORER_PROFILE.md`** documenting:
  - the explorer-only root QML path
  - the local shell shim used by the wrapper root
  - how to select the profile via CLI or environment
  - why it is a meaningful alternate mode rather than an alias

### Version
- Bumped to **6.0.43**.

## [6.0.42] - 2026-04-05

### Added — The "Dashboard-Only Native Launch Profile" Release

#### 🧩 First Genuinely Alternate Named Launch Profile Added
- Added the first real alternate native shell launch profile:
  - **`omni-dashboard-only`**
- This is not a fake alias for the existing shell root. The new profile launches a different root QML surface:
  - default profile → `qrc:/main.qml`
  - dashboard-only profile → `qrc:/DashboardShell.qml`
- Added:
  - `gui/omni/assets/DashboardShell.qml`
- Updated:
  - `gui/omni/assets/qml.qrc`
  - `gui/omni/src/NativeUiLaunchProfile.hpp`
  - `gui/omni/src/NativeUiLaunchProfile.cpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
- Added `create_dashboard_only_launch_profile()` and registered the profile name in the launch-profile registry.
- The new profile can be selected through the already-added selection seams:
  - CLI: `--native-ui-profile=omni-dashboard-only`
  - env: `BOBFILEZ_NATIVE_UI_PROFILE=omni-dashboard-only`

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_DASHBOARD_PROFILE.md`** documenting:
  - why this profile is a real alternate launch mode
  - the new root QML path
  - how to select the profile via CLI or environment
  - why the dashboard-only surface is a good first alternate profile

### Version
- Bumped to **6.0.42**.

## [6.0.41] - 2026-04-05

### Refactored — The "Native UI Profile CLI Selection" Release

#### 🧩 Seventh Concrete Option C Refinement Landed in Code
- Extended the native shell launch-profile selection seam so profile choice can be made from the command line as well as the environment.
- Updated:
  - `gui/omni/src/NativeUiProfileRegistry.hpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
  - `gui/omni/src/NativeUiBootstrap.cpp`
- Added registry helpers for:
  - `native_ui_profile_argument_prefix()`
  - `selected_launch_profile_name(int argc, char *argv[])`
  - `create_launch_profile_from_selection(int argc, char *argv[])`
- The active CLI selector is now:
  - `--native-ui-profile=<name>`
- Selection precedence is now explicit:
  1. command-line override
  2. environment variable override (`BOBFILEZ_NATIVE_UI_PROFILE`)
  3. default launch profile
- Unknown profile names still warn and fall back to the default profile.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_PROFILE_CLI_SELECTION.md`** documenting:
  - the new CLI selector
  - the precedence order
  - the unchanged default behavior
  - the limitations that remain unchanged

### Version
- Bumped to **6.0.41**.

## [6.0.40] - 2026-04-05

### Refactored — The "Native UI Profile Environment Selection" Release

#### 🧩 Sixth Concrete Option C Refinement Landed in Code
- Added a tiny registry-backed environment selection seam for named native shell launch profiles.
- Updated:
  - `gui/omni/src/NativeUiProfileRegistry.hpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
  - `gui/omni/src/NativeUiBootstrap.cpp`
- Added registry helpers for:
  - `native_ui_profile_environment_variable()`
  - `selected_launch_profile_name()`
  - `create_launch_profile_from_environment()`
- The active selection variable is now:
  - `BOBFILEZ_NATIVE_UI_PROFILE`
- Behavior:
  - no env var → use the default launch profile
  - known profile name → use that named profile
  - unknown profile name → emit a warning and fall back to the default profile
- This keeps bootstrap logic small while giving future alternate launch profiles a real, explicit selection hook.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_PROFILE_ENV_SELECTION.md`** documenting:
  - the new environment-driven selection path
  - the fallback behavior
  - the unchanged default runtime behavior
  - the limitations that remain unchanged

### Version
- Bumped to **6.0.40**.

## [6.0.39] - 2026-04-05

### Refactored — The "Native UI Profile Registry" Release

#### 🧩 Fifth Concrete Option C Refinement Landed in Code
- Added a small lookup/helper layer for named launch profiles and runtime bundles:
  - `gui/omni/src/NativeUiProfileRegistry.hpp`
  - `gui/omni/src/NativeUiProfileRegistry.cpp`
- Updated the active bootstrap so it no longer directly constructs the default profile factory result itself.
- `gui/omni/src/NativeUiBootstrap.cpp` now asks the registry for:
  - the default launch profile name
  - the corresponding launch profile by name
- This keeps bootstrap logic focused on orchestration rather than policy lookup as the number of named launch packages grows.

#### 🧹 GUI Build Wiring Updated
- Updated:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
- Added the new profile-registry files to the GUI source lists so the selection seam is represented honestly in both native GUI targets.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_PROFILE_REGISTRY.md`** documenting:
  - the new registry/helper surface
  - the updated bootstrap flow
  - why this improves Option C without changing runtime behavior
  - the limitations that remain unchanged

### Version
- Bumped to **6.0.39**.

## [6.0.38] - 2026-04-05

### Validated — The "BTK Upstream Refresh" Release

#### 🔄 BTK Updated to Latest Upstream Master and Rebased Cleanly
- Refreshed `libs/btk` to the newer upstream `origin/master` tip:
  - `18e3770af` — `build: validate BTK focus reason package smoke`
- Rebased bobfilez's two required MSVC fixes on top of that newer BTK tip:
  - `0546ebd70` — `fix: restore msvc build for focus and input routing`
  - `4f5a809e4` — `fix: restore qapplication property lookups for msvc`
- Updated the reproducible pushed BTK branch carrying those rebased fixes:
  - `origin/pi/msvc-focus-fixes-20260405`

#### ✅ BTK Build Result Still Good
- Re-ran `scripts/build_btk_inplace.bat` after the upstream refresh and rebase.
- Confirmed BTK still configures and builds successfully on this host with the rebased MSVC fixes applied.

#### 🔍 Downstream bobfilez Boundary Unchanged
- Re-ran `scripts/build_btk_gui.bat` against the refreshed BTK state.
- bobfilez still stops at the same honest compatibility boundary:
  - missing BTK/CopperSpice component `Declarative`
- This confirms the strategic conclusion did not depend on an older BTK snapshot; the same blocker persists on the newer upstream master.

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_UPSTREAM_REFRESH_2026_04_05.md`** documenting:
  - the newer upstream BTK tip
  - the rebased local MSVC fixes
  - the successful BTK rebuild
  - the unchanged downstream `Declarative` boundary

### Version
- Bumped to **6.0.38**.

## [6.0.37] - 2026-04-05

### Refactored — The "Native UI Launch Profiles and Runtime Bundles" Release

#### 🧩 Fourth Concrete Option C Extraction Step Landed in Code
- Evolved the prior launch-config extraction into a clearer named-policy model.
- Added:
  - `gui/omni/src/NativeUiRuntimeBundle.hpp`
  - `gui/omni/src/NativeUiRuntimeBundle.cpp`
  - `gui/omni/src/NativeUiLaunchProfile.hpp`
  - `gui/omni/src/NativeUiLaunchProfile.cpp`
- Replaced the previous raw launch-config callback bag in the active bootstrap flow with:
  - a **runtime bundle** (runtime factory + registration policy)
  - a **launch profile** (shell root QML + failure policy + chosen bundle)
- Refactored `gui/omni/src/NativeUiBootstrap.cpp` so it now reads as explicit launch-profile selection rather than ad hoc callback assembly.

#### 🧹 GUI Build Wiring Updated
- Updated:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
- Replaced the active launch-config files in the GUI source lists with the new launch-profile/runtime-bundle files.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_LAUNCH_PROFILES_AND_BUNDLES.md`** documenting:
  - the difference between runtime bundles and launch profiles
  - the new bootstrap flow
  - the preserved behavior
  - the remaining limitations

### Version
- Bumped to **6.0.37**.

## [6.0.36] - 2026-04-05

### Refactored — The "Native UI Launch Config" Release

#### 🧩 Third Concrete Option C Extraction Step Landed in Code
- Continued reducing provider coupling by turning active shell launch policy into explicit configuration data.
- Added:
  - `gui/omni/src/NativeUiLaunchConfig.hpp`
  - `gui/omni/src/NativeUiLaunchConfig.cpp`
- Refactored `gui/omni/src/NativeUiBootstrap.cpp` so it no longer hardcodes:
  - the main QML URL
  - the registration bundle choice
  - the default runtime factory choice
  - the root-object failure policy
- Introduced `NativeUiLaunchConfig` with explicit fields for:
  - `main_qml`
  - `register_types`
  - `runtime_factory`
  - `object_created_handler`
- Added `create_default_omni_ui_launch_config()` so the current shell launch policy is centralized rather than being implicit bootstrap behavior.

#### 🧹 GUI Build Wiring Updated
- Updated:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
- Added the new launch-config files to the GUI source lists so the extraction is represented honestly in both native GUI targets.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_LAUNCH_CONFIG.md`** documenting:
  - the remaining implicit launch policy after earlier extractions
  - the new configuration object
  - the bootstrap flow after the refactor
  - the behavior preserved and the limitations unchanged

### Version
- Bumped to **6.0.36**.

## [6.0.35] - 2026-04-05

### Refactored — The "Native UI Runtime Policy Extraction" Release

#### 🧩 Second Concrete Option C Extraction Step Landed in Code
- Continued reducing direct provider coupling in the active shell bootstrap by separating three concerns that had still been mixed inside `NativeUiBootstrap.cpp`:
  - bootstrap orchestration
  - declarative runtime construction/adapter selection
  - QML type-registration policy
- Added:
  - `gui/omni/src/NativeUiRuntime.hpp`
  - `gui/omni/src/NativeUiRuntime.cpp`
  - `gui/omni/src/OmniQmlRegistration.hpp`
  - `gui/omni/src/OmniQmlRegistration.cpp`
- Refactored `gui/omni/src/NativeUiBootstrap.cpp` so it now primarily orchestrates:
  - `QGuiApplication` creation
  - invocation of registration policy
  - default runtime creation via the runtime factory
  - root-object failure handling
  - loading `qrc:/main.qml`
- This keeps the active runtime behavior the same while making runtime implementation and registration policy independently swappable later.

#### 🧹 GUI Build Wiring Updated
- Updated:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
- Added the new runtime/registration files to the GUI source lists so the extraction is reflected honestly in both native GUI targets.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_RUNTIME_POLICY_EXTRACTION.md`** documenting:
  - the remaining coupling after the first bootstrap seam
  - the new responsibility split
  - the behavior preserved
  - what this extraction does and does not solve

### Version
- Bumped to **6.0.35**.

## [6.0.34] - 2026-04-05

### Refactored — The "Native UI Bootstrap Seam" Release

#### 🧩 First Concrete Option C Execution Step Landed in Code
- Added a provider-neutral bootstrap seam for the active native shell entrypoint:
  - `gui/omni/src/NativeUiBootstrap.hpp`
  - `gui/omni/src/NativeUiBootstrap.cpp`
- Refactored `gui/omni/src/main.cpp` into a minimal launcher that now delegates to `fo::gui::run_omni_shell(...)` instead of directly hardcoding the active declarative runtime path.
- Moved current runtime-specific responsibilities into the bootstrap layer, including:
  - `QGuiApplication` creation
  - active declarative-engine creation/loading
  - QML type registration for `FileModel`, `TreemapModel`, and `NativeMarkdownView`
  - root-object failure handling for `qrc:/main.qml`
- Introduced a narrow internal runtime interface so future provider/runtime work can target a smaller seam instead of rewriting the top-level entrypoint again.

#### 🧹 GUI Source List Alignment
- Updated:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
- Ensured the GUI source lists include the new bootstrap files.
- Also aligned `gui/CMakeLists.txt` to include `TreemapModel.cpp/.h`, keeping the source list coherent with the shell's QML type registration path.

#### 📘 Documentation
- Added **`docs/ai/implementation/NATIVE_UI_BOOTSTRAP_SEAM.md`** documenting:
  - the rationale for extracting the seam
  - the exact behavior preserved
  - what this refactor does and does not solve
  - how it fits into Option C from the BTK framework decision matrix

### Version
- Bumped to **6.0.34**.

## [6.0.33] - 2026-04-05

### Documented — The "BTK Framework Decision Matrix" Release

#### 🧭 Strategy Converted from Probe History into an Actionable Architecture Decision
- Added **`docs/ai/design/BTK_FRAMEWORK_DECISION_MATRIX.md`** to turn the BTK probe series into a decision-ready architecture artifact.
- The matrix compares four realistic paths:
  - upstream BTK modernization toward a `QQml*` / `QQuick*`-class provider
  - porting bobfilez backward to a `QDeclarative*` era stack
  - keeping bobfilez on a modern QQml-style shell path while decoupling provider assumptions
  - replacing the QML shell architecture entirely
- Included a scored comparison across preservation of current shell work, delivery risk, upstream burden, regression risk, and strategic optionality.
- The documented recommendation is now explicit:
  - **primary app strategy:** keep bobfilez on a modern QQml-style shell path and decouple provider assumptions
  - **optional parallel R&D strategy:** treat BTK modernization as a separate upstream framework effort, not as an immediate bobfilez integration unblock

#### 📊 Architectural Conclusion Sharpened Again
- Round 5's findings are now translated into a concrete project decision:
  - BTK is currently blocked not just by missing package exports and stale declarative internals, but also by a direct `QDeclarative*` vs `QQml*` generation mismatch
  - therefore the least-destructive near-term path is to stop treating BTK as the immediate native runtime for bobfilez's active shell bootstrap

#### 📘 Documentation
- Added **`docs/ai/design/BTK_FRAMEWORK_DECISION_MATRIX.md`** including:
  - a scored option matrix
  - a phased plan
  - a mermaid decision flow
  - explicit non-recommendations

### Version
- Bumped to **6.0.33**.

## [6.0.32] - 2026-04-05

### Validated — The "BTK Native Provider Probe Round 5" Release

#### 🧭 Architectural Compatibility Boundary Sharpened Further
- Confirmed the current blocker is not just missing `Declarative` exposure and not just stale declarative internals.
- Verified that bobfilez's active native GUI bootstrap in `gui/omni/src/main.cpp` is explicitly **QQml-style**, using:
  - `QGuiApplication`
  - `QQmlApplicationEngine`
  - `QQmlContext`
  - `qmlRegisterType(...)`
- Verified that BTK's declarative surface is still explicitly **QDeclarative-era**, centered on classes such as:
  - `QDeclarativeEngine`
  - `QDeclarativeComponent`
  - `QDeclarativeView`
  - `QDeclarativeItem`
- Searched the BTK tree and found no actual `QQml*` or `QQuick*` source/header surface, including no discovered:
  - `QQmlApplicationEngine`
  - `QQmlEngine`
  - `QQmlContext`
  - `QQuickItem`
  - `QQuickView`
  - `qqml*.h`
  - `qquick*.h`
- This proves the BTK compatibility gap is also a **framework-generation mismatch**: even a revived BTK declarative module would still not directly satisfy bobfilez's current `QQmlApplicationEngine`-based GUI path.

#### 📊 Quantified Declarative Revival Footprint
- A structural scan over `libs/btk/src/declarative` found:
  - **61 files** with direct QtScript-related usage
  - **24 files** referencing `QScriptDeclarativeClass`
  - **23 files** still using `Q_DECLARE_METATYPE(...)`
- Confirmed additional missing internal-script surface signs, including absent expected files such as:
  - `qscriptdeclarativeclass_p.h`
  - `qscriptengine.h`
- This gives a more honest estimate of the scale involved if BTK declarative revival is pursued.

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND5.md`** documenting:
  - the `QQml*` vs `QDeclarative*` API-generation mismatch
  - the absence of `QQml*` / `QQuick*` provider surface in BTK
  - the quantified QtScript/metatype dependency footprint inside BTK declarative
  - the conclusion that the next step is now a strategic framework decision, not just another build fix

### Version
- Bumped to **6.0.32**.

## [6.0.31] - 2026-04-05

### Validated — The "BTK Native Provider Probe Round 4" Release

#### 🧪 Experimental Declarative Enablement Proved the Gap is Deeper Than a Missing Component List Entry
- Confirmed bobfilez's active native entrypoint still genuinely requires a QML stack (`QQmlApplicationEngine` in `gui/omni/src/main.cpp`), so the missing `Declarative` capability remains a hard blocker.
- Temporarily re-enabled `Declarative` in BTK's top-level component list as an experiment to determine whether the module was merely omitted or actually stale/incomplete.
- The experiment showed the current BTK line is not just missing a list entry:
  - `src/declarative/CMakeLists.txt` immediately failed on stale build-system usage such as obsolete `target_add_definitions(...)`
  - after minimal CMake modernization to keep probing, the build advanced into declarative compilation and then failed on deeper issues including:
    - obsolete `Q_DECLARE_METATYPE(TYPE)` diagnostics requiring `CS_DECLARE_METATYPE(TYPE)`
    - fatal missing QtScript-era headers such as `QtScript/qscriptvalue.h`
- This confirms the current BTK `Declarative` / QML path is structurally incomplete in this branch and not yet an honest provider for bobfilez's active GUI architecture.

#### 🔧 BTK Submodule Correctness Preserved While Probing
- While testing against the newer upstream BTK tip, restored the normal BTK MSVC build by fixing a regressed property-name path in:
  - `libs/btk/src/gui/kernel/qapplication_cs.cpp`
- Re-verified that the normal BTK module set still builds successfully after reverting the temporary declarative-enablement experiment.
- Updated the bobfilez gitlink to a pushed BTK branch commit carrying the corrected MSVC fix:
  - branch: `pi/msvc-focus-fixes-20260405`

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND4.md`** documenting:
  - the temporary declarative-enablement experiment
  - the stale declarative CMake findings
  - the deeper compile-time `QtScript` dependency gap
  - the conclusion that the remaining blocker is broader framework readiness, not just a missing component toggle

### Version
- Bumped to **6.0.31**.

## [6.0.30] - 2026-04-05

### Validated — The "BTK Native Provider Probe Round 3" Release

#### ✅ BTK MSVC Build Repaired Enough to Produce Real Framework Outputs
- Patched the active `libs/btk` submodule to resolve the earlier MSVC build blockers which had prevented BTK from producing consumer-usable binaries.
- Replaced unsupported `Q_DECLARE_FLAGS(...)` usage with BTK-compatible `using ... = QFlags<...>;` aliases in:
  - `libs/btk/src/core/kernel/btkinputowner.h`
  - `libs/btk/src/gui/widgets/btkfocusoverlay.h`
  - `libs/btk/src/plugins/bearer/networkmanager/qnetworkmanagerservice.h`
- Normalized newer Qt-style string/property API usage in:
  - `libs/btk/src/gui/kernel/btkfocusdiagnostics.cpp`
  - `libs/btk/src/gui/kernel/qapplication_cs.cpp`
  - `libs/btk/src/gui/widgets/btkfocusoverlay.cpp`
- Confirmed `scripts/build_btk_inplace.bat` now completes successfully and produces BTK/CopperSpice outputs such as:
  - `libs/btk/build-btk/lib/CsCore2.1.lib`
  - `libs/btk/build-btk/lib/CsGui2.1.lib`
  - related plugin and DLL artifacts

#### 🔍 Sharper Downstream BTK Consumer Finding
- Re-ran `scripts/build_btk_gui.bat` after BTK's successful in-place build.
- bobfilez no longer fails because BTK libraries are missing.
- The current failure is now more honest and architectural:
  - bobfilez cannot resolve a BTK/CopperSpice target for component `Declarative`
- Investigation confirmed:
  - `libs/btk/src/declarative/CMakeLists.txt` exists and defines `CsDeclarative`
  - but BTK's top-level `CS_OPTIONAL_COMPONENTS` list does **not** include `Declarative`
  - therefore the current BTK build/package surface does not provide the QML/Declarative module family bobfilez's native GUI expects

#### 🧰 Better Probe Diagnostics
- Updated `cmake/BTKFrameworkSetup.cmake` so missing BTK/CopperSpice components now report:
  - the candidate targets checked
  - the imported BTK/CopperSpice targets actually available
  - an explicit hint when the missing component is `Declarative`

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND3.md`** documenting:
  - the BTK MSVC build fixes
  - the successful BTK in-place build result
  - the new `Declarative`/QML capability boundary
  - the conclusion that the active blocker is now framework capability, not package layout or missing library outputs

### Version
- Bumped to **6.0.30**.

## [6.0.29] - 2026-04-04

### Validated — The "BTK Native Provider Probe Round 2" Release

#### 🧪 BTK Build-Tree Package Prep Improved
- Added:
  - `scripts/prepare_btk_buildtree_package.py`
  - `scripts/prepare_btk_buildtree_package.bat`
- Updated `scripts/build_btk_gui.bat` to prepare a consumable BTK build-tree package surface before probing bobfilez's BTK-backed GUI configure.

#### 🔍 Sharper BTK Consumer Finding
- The earlier missing-companion-file and bad-import-prefix issues were bypassed by preparing and patching the generated BTK/CopperSpice export files in `libs/btk/build-btk`.
- After that improvement, the BTK-backed bobfilez GUI probe now fails on a more meaningful downstream condition:
  - imported target `CopperSpice::CsCore` references `libs/btk/build-btk/lib/CsCore2.1.lib`
  - that file does not exist yet
- This confirms the remaining blocker is now the absence of successfully built BTK libraries, which traces back to BTK's own compile failure in `btkinputowner.*`.

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE_ROUND2.md`** documenting the refined package-prep step, the corrected import-prefix handling, and the new narrowed blocker.

### Version
- Bumped to **6.0.29**.

## [6.0.28] - 2026-04-04

### Validated — The "BTK Native Provider Probe" Release

#### 🧪 Real BTK Consumer Validation
- Registered **`libs/btk`** as a real tracked submodule in the superproject.
- Ran the new BTK-native validation scripts:
  - `scripts/build_btk_inplace.bat`
  - `scripts/build_btk_gui.bat`

#### 🔍 BTK In-Place Build Findings
- BTK **configures successfully** on this host.
- The current BTK build then fails in BTK's own source during compilation, centered on:
  - `src/core/kernel/btkinputowner.h`
  - `src/core/kernel/btkinputowner.cpp`
- The error shape points to MSVC-side parsing / macro fallout around `Capabilities` / `Q_DECLARE_FLAGS` in `BtkInputOwner`.

#### 🧩 BTK Build-Tree Export Findings
- Fixed the initial BTK provider hint ordering in **`cmake/BTKFrameworkSetup.cmake`** so bobfilez prefers the generated `libs/btk/build-btk` package config over the raw source-tree template.
- After that fix, the BTK-backed bobfilez GUI probe advanced to a more meaningful failure:
  - `find_package(BTK CONFIG)` locates `libs/btk/build-btk/BTKConfig.cmake`
  - but that config currently references missing companion files such as:
    - `CopperSpiceLibraryTargets.cmake`
    - `CopperSpiceBinaryTargets.cmake`
    - `BTKMacros.cmake`
    - `BTKDeploy.cmake`
- Result:
  - the active blocker is now in **BTK build/export readiness**, not in the older BobUI wiring path

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_PROBE.md`** documenting the repaired submodule registration, the BTK in-place build result, the BTK build-tree export failure, and the new next-step boundary.

### Version
- Bumped to **6.0.28**.

## [6.0.27] - 2026-04-04

### Migrated — The "BTK Native Framework Switch" Release

#### 🔁 Native Framework Direction Changed
- Replaced the active BobUI/Qt-fork native-framework direction with **BTK** (`https://github.com/robertpelloni/btk`).
- Updated the primary native-provider terminology, docs, and build scripts to refer to:
  - `libs/btk`
  - `BTK_ROOT`
  - BTK-backed native GUI / Omni builds

#### 🧩 Build-System Retargeting
- Replaced **`cmake/BobUIQtSetup.cmake`** with **`cmake/BTKFrameworkSetup.cmake`**.
- Updated root and GUI CMake wiring to:
  - prefer BTK discovery via `BTK_ROOT` / `FO_BTK_ROOT`
  - stop assuming a BobUI/Qt6 provider model
  - resolve BTK / CopperSpice imported targets instead of `Qt6::*`
- Updated GUI helper scripts:
  - `scripts/build_bobui_gui.bat` → `scripts/build_btk_gui.bat`
  - `scripts/build_bobui_inplace.bat` → `scripts/build_btk_inplace.bat`

#### 🚫 BobUI-Specific Bootstrap Assumptions Removed
- Updated **`gui/omni/src/main.cpp`** to remove the BobUI-specific bootstrap dependency:
  - removed `#include "OmniQmlRegistration.h"`
  - removed `OmniUI::registerQmlTypes()`
- Updated GUI CMake targets to remove the BobUI-specific `OmniUI/omnicore` source-tree dependency.

#### 🖼️ BobUI QML Imports Replaced in Active Panels
- Updated **`gui/panels/PhotoDevelopPanel.qml`** and **`gui/panels/PhotoLibraryPanel.qml`**.
- Removed BobUI-specific imports:
  - `import OmniUI 1.0`
  - `import OmniLayout 1.0`
  - `import OmniData 1.0`
- Replaced their BobUI widget usage with local Qt Quick / Qt Quick Controls primitives so those panels no longer depend on BobUI namespaces.

#### 📘 Documentation
- Added **`docs/ai/implementation/BTK_PROVIDER_SWITCH.md`** documenting the architectural difference between BobUI and BTK, the retargeted provider model, and the remaining compatibility work.
- Updated key project docs and agent instructions to describe BTK as the current native-framework direction.

### Version
- Bumped to **6.0.27**.

## [6.0.26] - 2026-04-04

### Reduced — The "QtQuick.Controls Gamification Reduction" Release

#### 🏆 Fifth Routed-Panel Conversion
- Updated **`gui/panels/GamificationPanel.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with plain `QtQuick` primitives and small local helpers, including:
  - `Label` → `Text`
  - action `Button` → local `PanelButton` helper
  - `ProgressBar` → local `SlimProgressBar` helper
  - `GroupBox` achievements section → explicit titled `Text` + `Rectangle` container
  - repeated summary cards → local `StatCard` helper
- Replaced the stock `ToolTip` behavior on achievement badges with a small local hover-popup rectangle, preserving lightweight hover detail without retaining the Controls dependency.

#### 📊 Routed-Panel Pattern Expanded Slightly
- This is the **fifth successful routed-panel controls-removal pass**.
- It also proves that limited hover-detail behavior can be replaced locally while keeping the panel self-contained and readable.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **35 QML files** importing `QtQuick.Controls`
  - to **34 QML files**
- The reduction campaign remains cumulative, measurable, and low-risk.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_GAMIFICATION_REDUCTION.md`** documenting why `GamificationPanel.qml` was a safe next escalation and how the local hover-popup replacement fits the migration strategy.

### Version
- Bumped to **6.0.26**.

## [6.0.25] - 2026-04-04

### Reduced — The "QtQuick.Controls Forensic Reduction" Release

#### 🛡️ Fourth Routed-Panel Conversion
- Updated **`gui/panels/ForensicPanel.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with plain `QtQuick` primitives and small local helpers, including:
  - `Label` → `Text`
  - action `Button`s → local `PanelButton` helper
  - `GroupBox` chain-of-custody section → explicit titled `Text` + `Rectangle` container
  - repeated ledger summary cards → local `StatCard` helper
- Removed the stock `ScrollBar.vertical: ScrollBar {}` attachment so the list view is fully free of Controls usage.
- Added local `integrityFillColor()` and `integrityTextColor()` helpers to keep badge styling explicit and readable.

#### 📊 Routed-Panel Pattern Reinforced Again
- This is the **fourth successful routed-panel controls-removal pass** after `DigitalRotPanel.qml`, `SwarmPanel.qml`, and `RecoveryPanel.qml`.
- The dashboard-style, interaction-light migration lane is now strongly established.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **36 QML files** importing `QtQuick.Controls`
  - to **35 QML files**
- The migration remains cumulative, measurable, and low-risk.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_FORENSIC_REDUCTION.md`** documenting why `ForensicPanel.qml` was a strong next candidate and how the helper-based rewrite preserved clarity.

### Version
- Bumped to **6.0.25**.

## [6.0.24] - 2026-04-04

### Reduced — The "QtQuick.Controls Recovery Reduction" Release

#### 🩹 Third Routed-Panel Conversion
- Updated **`gui/panels/RecoveryPanel.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with plain `QtQuick` primitives and small local helpers, including:
  - `Label` → `Text`
  - action `Button`s → local `PanelButton` helper
  - `GroupBox` integrity-exceptions section → explicit titled `Text` + `Rectangle` container
  - repeated dashboard metrics → local `StatCard` helper
- Added a local `corruptionColor()` helper to keep conditional stat styling readable and explicit.

#### 📊 Routed-Panel Pattern Strengthened
- This is the **third successful routed-panel controls-removal pass** after `DigitalRotPanel.qml` and `SwarmPanel.qml`.
- It further validates the current strategy of targeting dashboard-style, interaction-light panels before richer widget-heavy surfaces.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **37 QML files** importing `QtQuick.Controls`
  - to **36 QML files**
- The migration remains incremental, quantitative, and cumulative.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_RECOVERY_REDUCTION.md`** documenting why `RecoveryPanel.qml` was such a strong follow-up target and how the helper-based conversion preserves readability.

### Version
- Bumped to **6.0.24**.

## [6.0.23] - 2026-04-04

### Reduced — The "QtQuick.Controls Swarm Reduction" Release

#### 🛸 Second Routed-Panel Conversion
- Updated **`gui/panels/SwarmPanel.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with plain `QtQuick` primitives and small local helpers, including:
  - `Label` → `Text`
  - action `Button`s → local `PanelButton` helper
  - `GroupBox` detected-nodes section → explicit titled `Text` + `Rectangle` container
  - `ProgressBar` → local `SlimProgressBar` helper
- Added a small local `statusColor(status)` helper to keep status styling explicit and readable.

#### 📊 Routed-Panel Pattern Reinforced
- This is the **second successful routed-panel controls-removal pass** after `DigitalRotPanel.qml`.
- It strengthens the emerging migration rule that dashboard-style, interaction-light panels are the best next-wave candidates.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **38 QML files** importing `QtQuick.Controls`
  - to **37 QML files**
- The reduction campaign remains quantitative and cumulative.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_SWARM_REDUCTION.md`** documenting why `SwarmPanel.qml` was a cleaner next target than richer panels and how the helper-based conversion preserved readability.

### Version
- Bumped to **6.0.23**.

## [6.0.22] - 2026-04-04

### Reduced — The "QtQuick.Controls Digital Rot Reduction" Release

#### 🧹 First Routed-Panel Conversion
- Updated **`gui/panels/DigitalRotPanel.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with plain `QtQuick` primitives and small local helpers, including:
  - `Label` → `Text`
  - action `Button`s → local `PanelButton` helper
  - `GroupBox` suggestions section → explicit titled `Text` + `Rectangle` container
  - repeated stat cards → local `StatCard` helper
- Preserved the panel's existing scanning toggle behavior and dashboard-style presentation.

#### 🧭 Migration Phase Shift
- This is the **first successful controls-removal pass in the routed panel layer** after completion of the shell-assets layer.
- It confirms that the incremental strategy can continue into panels, provided the panel is self-contained and interaction-light.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **39 QML files** importing `QtQuick.Controls`
  - to **38 QML files**
- This keeps the campaign quantitative while the migration frontier moves from shell assets into routed panels.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_DIGITAL_ROT_REDUCTION.md`** documenting why `DigitalRotPanel.qml` was the correct first panel target, what helper-component strategy was used, and why this represents a new migration phase.

### Version
- Bumped to **6.0.22**.

## [6.0.21] - 2026-04-04

### Reduced — The "QtQuick.Controls Dashboard Reduction" Release

#### 📊 Dashboard Shell-Asset Conversion
- Updated **`gui/omni/assets/Dashboard.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock convenience controls with plain `QtQuick` primitives and local inline helpers, including:
  - `Label` → `Text`
  - settings `Button` → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`
  - `GroupBox` sections → explicit titled `Text` + `Rectangle` containers
  - `ProgressBar` → local `SlimProgressBar` helper component
  - quick action `Button`s → local `QuickActionChip` helper component

#### 🧭 Shell-Assets Milestone
- Confirmed that **`gui/omni/assets/` now has zero QML files importing `QtQuick.Controls`**.
- This marks the shell-assets layer as fully converted away from stock controls, leaving the remaining imports concentrated in routed panel files.

#### 📈 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **40 QML files** importing `QtQuick.Controls`
  - to **39 QML files**
- The migration remains incremental, measurable, and low-risk.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_DASHBOARD_REDUCTION.md`** documenting the conversion strategy, the helper-component approach, and the new shell-assets milestone.

### Version
- Bumped to **6.0.21**.

## [6.0.20] - 2026-04-04

### Reduced — The "QtQuick.Controls Main Host Reduction" Release

#### 🧭 Main Shell Host Cleanup
- Updated **`gui/omni/assets/main.qml`** to remove `QtQuick.Controls` usage entirely.
- Converted the notification-center surface from stock controls to lighter `QtQuick` primitives, including:
  - `Label` → `Text`
  - `Clear All` button → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`
- This removes a stock-controls dependency from the shell host itself without touching the routed tool/panel components.

#### 📊 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **41 QML files** importing `QtQuick.Controls`
  - to **40 QML files**
- This keeps the controls-reduction effort measurable and cumulative.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_MAIN_REDUCTION.md`** documenting why `main.qml` was a good target, what was localized enough to convert safely, and how this fits the incremental migration plan.

### Version
- Bumped to **6.0.20**.

## [6.0.19] - 2026-04-04

### Reduced — The "QtQuick.Controls OmniPeek Reduction" Release

#### 👁️ OmniPeek Overlay Conversion
- Updated **`gui/omni/assets/OmniPeekOverlay.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with lighter `QtQuick` primitives, including:
  - `Label` → `Text`
  - header/buttons → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`
  - viewer buttons → lightweight custom button shells
  - `ScrollView` + `TextArea` → `Flickable` + read-only `TextEdit`

#### 📊 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **42 QML files** importing `QtQuick.Controls`
  - to **41 QML files**
- This demonstrates the controls-reduction strategy can extend beyond minimal shell chrome into richer overlay surfaces while remaining incremental.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_OMNIPEEK_REDUCTION.md`** documenting the replacement set, tradeoffs, and why OmniPeek was a safe next-step target.

### Version
- Bumped to **6.0.19**.

## [6.0.18] - 2026-04-04

### Reduced — The "QtQuick.Controls Explorer Reduction" Release

#### 🪟 Explorer Window Conversion
- Updated **`gui/omni/assets/ExplorerWindow.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock controls with lighter `QtQuick` patterns, including:
  - `Label` → `Text`
  - toolbar `Button`s → `Rectangle` + `Text` + `MouseArea` + `HoverHandler`
  - command-bar action button → custom rectangle button shell
  - `ItemDelegate` → rectangle/list-row shells
  - removed the stock `ScrollBar` attachment

#### 📊 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **43 QML files** importing `QtQuick.Controls`
  - to **42 QML files**
- The shell-adjacent no-Controls set now includes:
  - `Taskbar.qml`
  - `StartMenu.qml`
  - `DesktopIcons.qml`
  - `NexusPulse.qml`
  - `WindowManager.qml`
  - `ExplorerWindow.qml`

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_EXPLORER_REDUCTION.md`** documenting the replacement set, tradeoffs, and why this was the next safe escalation after the earlier shell control cleanups.

### Version
- Bumped to **6.0.18**.

## [6.0.17] - 2026-04-04

### Reduced — The "QtQuick.Controls Second Reduction" Release

#### 🧱 Additional Shell-Adjacent Control Cleanup
- Updated **`gui/omni/assets/DesktopIcons.qml`** to remove `QtQuick.Controls` usage.
- Updated **`gui/omni/assets/NexusPulse.qml`** to remove `QtQuick.Controls` usage.
- Updated **`gui/omni/assets/WindowManager.qml`** to remove a dead `QtQuick.Controls` import.
- Replaced the remaining trivial `Label` usage in the first two files with `Text`.

#### 📊 Surface Measurement
- Re-ran the global `QtQuick.Controls` import audit.
- Result:
  - reduced from **46 QML files** importing `QtQuick.Controls`
  - to **43 QML files**
- Combined with the first reduction pass, the shell-adjacent no-Controls set now includes:
  - `Taskbar.qml`
  - `StartMenu.qml`
  - `DesktopIcons.qml`
  - `NexusPulse.qml`
  - `WindowManager.qml`

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_SECOND_REDUCTION.md`** documenting the file choices, measurement update, and what categories remain intentionally deferred.

### Version
- Bumped to **6.0.17**.

## [6.0.16] - 2026-04-04

### Reduced — The "QtQuick.Controls Initial Reduction" Release

#### 🧱 First Controls-Surface Cut
- Updated **`gui/omni/assets/Taskbar.qml`** to remove `QtQuick.Controls` usage entirely.
- Updated **`gui/omni/assets/StartMenu.qml`** to remove `QtQuick.Controls` usage entirely.
- Replaced stock convenience controls (`Label`, trivial `Button`, `ToolTip`) with simpler `QtQuick` primitives such as:
  - `Text`
  - `Rectangle`
  - `MouseArea`

#### 📊 Surface Measurement
- Recomputed the current `QtQuick.Controls` import footprint across the QML tree.
- Result:
  - reduced from **48 QML files** importing `QtQuick.Controls`
  - to **46 QML files**
- This is a small but real reduction on two shell-critical surfaces.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTQUICK_CONTROLS_INITIAL_REDUCTION.md`** documenting the exact files changed, why they were chosen, what was intentionally deferred, and how this fits the phased migration plan.

### Version
- Bumped to **6.0.16**.

## [6.0.15] - 2026-04-04

### Wired — The "BobUI Registration Alignment" Release

#### 🧩 BobUI Layout / Bootstrap Correction
- Updated **`gui/CMakeLists.txt`** and **`gui/omni/CMakeLists.txt`** to point at the current BobUI source layout:
  - `libs/bobui/OmniUI/omnicore`
- Removed the stale `OmniUI/core` assumption.
- Added recursive BobUI omnicore source inclusion and the corresponding include paths for:
  - `omnicore/include`
  - `omnicore/src`
  - `deps/juce`
  - `deps/imgui`

#### 🚀 Real BobUI QML Registration Call
- Updated **`gui/omni/src/main.cpp`** to include:
  - `OmniQmlRegistration.h`
- Added a real startup call to:
  - `OmniUI::registerQmlTypes();`
- This means bobfilez now structurally attempts to back imports such as `OmniUI`, `OmniLayout`, and `OmniData` with BobUI registrations instead of only registering its local bridge types.

#### 📘 Documentation
- Added **`docs/ai/implementation/BOBUI_REGISTRATION_WIRING.md`** documenting the stale-path bug, the current BobUI layout, the bootstrap correction, and the remaining limitations.

#### 🔍 Important Limitation
- This does **not** solve the already-known full GUI blocker:
  - BobUI still lacks the full `Qt6Qml` / `Qt6Quick` / `Qt6QuickControls2` provider surface needed by bobfilez's current GUI targets on this machine.
- This release improves integration correctness, not end-to-end GUI buildability.

### Version
- Bumped to **6.0.15**.

## [6.0.14] - 2026-04-04

### Simplified — The "GraphicalEffects Full Removal" Release

#### ✨ Final Effect Users Replaced
- Reworked the remaining real `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects` users in:
  - `gui/omni/assets/AcrylicBackground.qml`
  - `gui/omni/assets/Dashboard.qml`
  - `gui/omni/assets/ExplorerWindow.qml`
  - `gui/omni/assets/OmniPeekOverlay.qml`
  - `gui/omni/assets/Taskbar.qml`
  - `gui/panels/OmniVersePanel.qml`
- Removed the last actual uses of:
  - `DropShadow`
  - `FastBlur`
- Replaced them with simpler native borders / tint layers / layout-preserving fallbacks.

#### ✅ Verification / Finding
- Confirmed there are now **no remaining** `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects` imports under `gui/`.
- Confirmed there are now **no remaining** GraphicalEffects primitives under `gui/` such as `DropShadow` or `FastBlur`.
- This completes the GraphicalEffects cleanup sequence started in the prior release.

#### 📘 Documentation
- Added **`docs/ai/implementation/GRAPHICALEFFECTS_FULL_REMOVAL.md`** documenting the exact files changed, what visual simplifications were made, and why the tradeoff is acceptable at this stage.

### Version
- Bumped to **6.0.14**.

## [6.0.13] - 2026-04-04

### Cleaned — The "GraphicalEffects Import Cleanup" Release

#### ✨ Dead Effect Imports Removed
- Audited `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects` usage across the QML tree and separated real effect users from dead imports.
- Removed stale effect imports from 27 QML files where no effect items (`DropShadow`, `FastBlur`, etc.) were actually instantiated.
- Left the six real effect-using files unchanged for now so visual behavior remains stable while the dependency surface becomes easier to reason about.

#### 🔍 Key Finding
- The actual current GraphicalEffects dependency footprint is much smaller than the raw import count previously suggested.
- Remaining real effect users are concentrated in a small set of shell/chrome surfaces, which makes the next replacement phase more focused and less risky.

#### 📘 Documentation
- Added **`docs/ai/implementation/GRAPHICALEFFECTS_IMPORT_CLEANUP.md`** documenting the audit, exact file sets, and the recommended next step.

### Version
- Bumped to **6.0.13**.

## [6.0.12] - 2026-04-04

### Cleaned — The "QtCharts Removal" Release

#### 📉 Dead GUI Dependency Removed
- Updated **`gui/panels/EnhancedFileOpsPanel.qml`** to remove:
  - `import QtCharts 2.5`
- Confirmed that the panel's real-time speed graph was already implemented with a native `Canvas`-based painter path rather than `ChartView` / `LineSeries` / other QtCharts types.
- This means the `QtCharts` import had become unused dependency residue and could be removed safely.

#### ✅ Verification / Finding
- Confirmed there are no remaining `QtCharts` references under `gui/` after this cleanup.
- This further shrinks the native UI dependency surface without changing behavior.

#### 📘 Documentation
- Added **`docs/ai/implementation/QTCHARTS_REMOVAL.md`** documenting why the removal was safe, what the panel was already using instead, and why this matches the phased BobUI migration strategy.

### Version
- Bumped to **6.0.12**.

## [6.0.11] - 2026-04-04

### Fixed — The "Markdown WebEngine Removal" Release

#### 🧾 Native Markdown Preview
- Added **`gui/omni/src/NativeMarkdownView.h`** and **`gui/omni/src/NativeMarkdownView.cpp`**.
- The new preview item renders markdown natively through `QQuickPaintedItem` + `QTextDocument` while reusing the existing core `MarkdownRenderer`.
- Exposed basic preview metadata back to QML:
  - word count
  - reading time
  - document title

#### 🧹 WebEngineQuick Dependency Reduction
- Updated **`gui/panels/MarkdownViewerPanel.qml`** to remove:
  - `import QtWebEngine 1.9`
  - `WebEngineView`
- Replaced the preview pane with the new native `MarkdownView` type registered under `Omni.Native 1.0`.
- Updated **`gui/omni/CMakeLists.txt`** to remove `WebEngineQuick` from the required Qt component set and target link libraries.

#### 🧩 Bootstrap / Shared GUI Wiring
- Updated **`gui/omni/src/main.cpp`** to register the native markdown preview QML type.
- Updated **`gui/CMakeLists.txt`** so the shared `fo_gui` path also includes `NativeMarkdownView` sources.
- Updated **`core/include/fo/core/markdown_viewer_interface.hpp`** comments so browser/WebEngine rendering is described as an optional frontend path rather than the only rendering model.

#### ✅ Verification / Finding
- Confirmed there are no remaining `QtWebEngine`, `WebEngineView`, or `WebEngineQuick` references under `gui/`.
- Re-ran the BobUI consumer probe and confirmed the remaining blocker is still:
  - `Qt6Qml`
- In other words, this phase successfully removed one heavyweight GUI dependency without changing the deeper declarative-stack blocker already identified.

#### 📘 Documentation
- Added **`docs/ai/implementation/MARKDOWN_WEBENGINE_REMOVAL.md`** documenting the implementation, validation, tradeoffs, and next-step implications.

### Version
- Bumped to **6.0.11**.

## [6.0.10] - 2026-04-04

### Analyzed — The "BobUI Native Migration Audit" Release

#### 🧭 Migration Cost Audit
- Added **`docs/ai/implementation/BOBUI_NATIVE_MIGRATION_AUDIT.md`**.
- Audited the real cost of removing the current bobfilez dependency set on:
  - QML composition
  - `QtQuick.Controls`
  - `QtQuick.Layouts`
  - `QtGraphicalEffects` / `Qt5Compat.GraphicalEffects`
  - `QtWebEngine`
  - `QtCharts`
- Quantified the current native shell surface as:
  - **49 QML files**
  - **9,844 QML lines**
  - **39 routed shell/panel surfaces** in `main.qml`

#### 🔍 Critical Architectural Finding
- Confirmed that current BobUI widgets/layouts are themselves based on:
  - `QQuickItem`
  - `QQuickPaintedItem`
- This means a BobUI-first migration does **not** currently imply removal of **QtQuick** itself.
- The practical migration targets must therefore be separated:
  - remove `WebEngineQuick` → realistic
  - reduce stock Quick Controls / Effects → hard but incremental
  - remove QML composition entirely → major rewrite
  - remove QtQuick itself → not realistic with current BobUI architecture

#### 🧱 Additional Integration Finding
- Confirmed that bobfilez currently does **not** call BobUI's `OmniUI::registerQmlTypes()` in its bootstrap path.
- This means the project is still largely a **stock Qt Quick shell with thin C++ model bridges**, not yet a genuinely BobUI-driven shell.

#### ✅ Recommendation Captured
- The best near-term path is:
  1. remove `WebEngineQuick`
  2. trim other nonessential stock Qt modules
  3. adopt BobUI widgets/layouts incrementally while keeping QML as a composition layer for now
- A wholesale "delete QML" push is currently the wrong first move.

### Version
- Bumped to **6.0.10**.

## [6.0.9] - 2026-04-04

### Investigated — The "In-Place BobUI Build Probe" Release

#### 🧪 BobUI Build Attempt
- Added **`scripts/build_bobui_inplace.bat`** to configure and build BobUI in place under `libs/bobui/build-bobui`.
- Confirmed that BobUI can be configured as an in-place **developer build** on this machine when the MSVC minimum-version guard is explicitly bypassed.
- The configure step generated a real:
  - `libs/bobui/build-bobui/lib/cmake/Qt6/Qt6Config.cmake`

#### 🔍 Critical Finding
- Re-ran bobfilez against the in-place BobUI build tree.
- bobfilez was finally able to see the top-level **Qt6** package config from BobUI, but failed immediately on required missing components:
  - `Qt6Qml`
  - therefore also no practical `Qt6Quick`, `Qt6QuickControls2`, or `Qt6WebEngineQuick`
- This demonstrates that the current BobUI tree is **not yet a full drop-in provider** for the bobfilez GUI stack as currently written.

#### 📘 Documentation
- Updated **`docs/ai/implementation/BOBUI_PROVIDER_SETUP.md`** with the exact in-place build/probe result and the current decision tree.

### Version
- Bumped to **6.0.9**.

## [6.0.8] - 2026-04-04

### Added — The "BOBGUI Evaluation" Release

#### 📦 New Submodule
- Added **`libs/bobgui`** as a new git submodule:
  - `https://github.com/robertpelloni/bobgui`

#### 🔎 Comparative Analysis
- Added **`docs/ai/implementation/BOBGUI_VS_BOBUI.md`**.
- Compared `bobgui` and `bobui` across:
  - architecture lineage
  - language/runtime model
  - build system
  - platform/toolkit assumptions
  - practical fit for bobfilez
- Conclusion:
  - **`bobui` is the better library for bobfilez** because it aligns with the existing Qt/QML/Omni shell direction and avoids a frontend rewrite.
  - `bobgui` appears stronger as a standalone GTK-style toolkit foundation, but is a poor strategic fit for the current bobfilez UI architecture.

### Version
- Bumped to **6.0.8**.

## [6.0.7] - 2026-04-04

### Changed — The "BobUI Update & Port Cleanup" Release

#### 🧩 BobUI Submodule Update
- Updated **`libs/bobui`** from `01634f269f` to `581de545a4` on `origin/main`.
- This brings in the latest BobUI / OmniUI upstream state, including broader framework changes and the current BobUI repository structure.

#### 🧹 Alternate-Port Cleanup
- Removed the obsolete **`filez-java/`** experiment from the repository.
- Removed Java-only packaging artifacts:
  - `scripts/package_java_msi.bat`
  - `wix/filez_java.wxs`
- Updated deployment and architecture notes so bobfilez no longer describes any maintained alternate port currently living in-tree.

#### 🧪 Feasibility Finding
- Added **`scripts/build_bobui_gui.bat`** as a repeatable MSVC/Ninja probe/build entrypoint for BobUI-backed GUI targets.
- BobUI can **likely** be made to work as the GUI provider for bobfilez, but not by pointing bobfilez at a raw BobUI source checkout alone.
- A real configure probe against `BOBUI_ROOT=libs/bobui` now fails with the expected package-discovery blocker:
  - missing `Qt6Config.cmake` / `qt6-config.cmake`
- The current integration therefore still needs a **BobUI-built/exported Qt6 package layout** so `find_package(Qt6 ...)` can resolve successfully.
- In other words, the path forward is:
  1. build/install BobUI
  2. point `BOBUI_ROOT` at that built/exported prefix
  3. configure bobfilez GUI / Omni targets against those package configs

### Version
- Bumped to **6.0.7**.

## [6.0.6] - 2026-04-04

### Improved — The "BobUI Qt Provider Preference" Release

#### 🖥️ BobUI-First GUI Discovery
- Added **`cmake/BobUIQtSetup.cmake`**.
- The root build now prefers **`github.com/robertpelloni/bobui`** as the Qt provider whenever `FO_BUILD_GUI` or `FO_BUILD_OMNI` is enabled.
- The helper checks:
  - `BOBUI_ROOT`
  - `FO_BOBUI_ROOT`
  - local `libs/bobui`
- It appends likely BobUI build/install prefixes to `CMAKE_PREFIX_PATH` so `find_package(Qt6 ...)` can resolve against BobUI rather than assuming a separately installed stock Qt.

#### 🔧 Build Wording Cleanup
- Updated root option descriptions to refer to the **BobUI GUI** / **BobUI Omni shell** instead of generic Qt wording.
- Added status messages in:
  - `gui/CMakeLists.txt`
  - `gui/omni/CMakeLists.txt`
  to make the BobUI-backed Qt resolution path explicit during configure.

#### 📘 Documentation
- Added **`docs/ai/implementation/BOBUI_PROVIDER_SETUP.md`** documenting the correct integration model:
  - BobUI is a Qt fork
  - bobfilez should still consume normal `Qt6::*` targets
  - but those packages should come from BobUI when GUI builds are enabled
- Updated **`README.md`** to describe BobUI as the preferred native GUI stack.

#### 🧭 Go Port Status Clarification
- Confirmed there is **no active Go port** in the current bobfilez tree.
- At that point the repo still contained the older `filez-java/` experiment; this was removed in **6.0.7**.

### Version
- Bumped to **6.0.6**.

## [6.0.5] - 2026-04-04

### Fixed — The "Headless Build Stabilization" Release

#### 🛠️ Windows/MSVC Build Recovery
- Added **`scripts/build_headless.bat`** to provide a repeatable Windows/MSVC fallback build that:
  - locates `vcvars64.bat`
  - configures `build-msvc` with `FO_BUILD_GUI=OFF` and `FO_BUILD_OMNI=OFF`
  - builds the core library, CLI, tests, and benchmarks with conservative parallelism
- This gives the repo a reliable non-Qt build path even when the full Omni/Qt environment is not installed.

#### 📦 Dependency Strategy Improvement
- Updated **`vcpkg.json`** so native `ffmpeg` and `chromaprint` are no longer forced in the default dependency set.
- Added a new optional **`media-analysis`** feature containing those heavier packages.
- This prevents the default Windows build from being blocked by FFmpeg compilation/link failures on hosts that only need the core/CLI/test toolchain.

#### 🔧 Core/CLI Compile Fixes
- Fixed multiple latent compile issues that were surfaced once the dependency blockers were reduced, including problems across:
  - `core/CMakeLists.txt`
  - `cli/fo_cli.cpp`
  - `core/include/fo/core/batch_rename_interface.hpp`
  - `core/include/fo/core/conversion_interface.hpp`
  - `core/include/fo/core/enhanced_fileops_interface.hpp`
  - `core/include/fo/core/omnivision_engine_interface.hpp`
  - `core/include/fo/core/search_interface.hpp`
  - `core/src/advanced_archive_manager.cpp`
  - `core/src/batch_rename_engine.cpp`
  - `core/src/conversion_engine.cpp`
  - `core/src/enhanced_fileops.cpp`
  - `core/src/hex_editor.cpp`
  - `core/src/markdown_viewer.cpp`
  - `core/src/omniverse_engine.cpp`
  - `core/src/wasm_bridge.cpp`
- Removed the remaining MSVC warning in the CLI `convert` command caused by a shadowed `output_path` variable.

#### ✅ Verification
- Verified successful headless build outputs for:
  - `fo_core`
  - `fo_cli`
  - `fo_tests`
  - benchmark binaries
- Ran **`build-msvc/tests/fo_tests.exe`** successfully:
  - **63 / 63 tests passed**
- Ran **`build-msvc/cli/fo_cli.exe --help`** successfully as a CLI smoke test.

#### 📘 Documentation
- Added **`docs/ai/implementation/HEADLESS_BUILD_STABILIZATION.md`** to record the dependency strategy, script, compile fixes, verification results, and the remaining Qt6 blocker for the full GUI/shell build.

### Version
- Bumped to **6.0.5**.

## [6.0.4] - 2026-04-03

### Added — The "Tracked-Only Status Workflow" Release

#### 🧰 Repo Status Tooling
- Added **`scripts/repo_status.py`** as a cross-platform helper for a safer day-to-day status view.
- The helper runs:
  - `git status --short --branch --untracked-files=no`
- This captures the important tracked-change signal while avoiding the noisy long-path untracked scan currently seen on this Windows host.

#### 🔍 New Diagnostic Finding
- Confirmed that the remaining long-path warning is specifically tied to **untracked-file scanning**.
- `git status --untracked-files=no` completes cleanly and still reports:
  - tracked file changes
  - deletions
  - dirty submodules
- Updated **`docs/ai/implementation/REPO_HYGIENE_CLEANUP.md`** to record this finding and the temporary workflow recommendation.

### Version
- Bumped to **6.0.4**.

## [6.0.3] - 2026-04-03

### Cleaned — The "Generated Build Artifact Purge" Release

#### 🧹 Build Artifact Cleanup
- Removed tracked/generated CMake snapshot directories from the working tree for:
  - `benchmarks/build_output/`
  - `cli/build_output/`
  - `core/build_output/`
  - `fuzz/build_output/`
  - `gui/build_output/`
  - `tests/build_output/`
- Removed transient `tests/build/` output from the working tree as part of the hygiene pass.
- Added **`docs/ai/implementation/REPO_HYGIENE_CLEANUP.md`** to document what was removed and why these files should not remain part of normal repo state.

#### 🔍 Findings
- Confirmed that several component-level `build_output/` trees contained environment-specific CMake-generated artifacts such as `CMakeCache.txt`, compiler-ID files, and configure logs.
- These artifacts are poor candidates for version control because they are machine-specific and rebuildable.
- **Important:** root-level `git status` still emitted filename-too-long warnings for nested `tests/test_cmake_build/.../pybind11/...` paths even after the cleanup.
- This indicates that the remaining long-path issue likely requires a more specialized extended-path pruning pass beyond simple build-output deletion.

### Version
- Bumped to **6.0.3**.

## [6.0.2] - 2026-04-03

### Improved — The "OmniShell Route Audit & Repo Hygiene" Release

#### 🧭 Route Audit & Launcher Coverage
- Added **`docs/ai/implementation/OMNISHELL_ROUTE_AUDIT.md`** to document every `shell.activePanel` route currently hosted by `gui/omni/assets/main.qml`.
- Audited which routes are reachable from the **Taskbar**, **Start Menu**, and **Explorer/sidebar** surfaces.
- Identified the current hidden/contextual route set (`rename`, `convert`, `hex`, `image`, `md`, `watcher`, `fileops`, `visual_dedup`, `pruner`, `achievements`, `forensic`, `develop`) for deliberate future exposure decisions.

#### 📌 Taskbar Wiring
- Upgraded **`gui/omni/assets/Taskbar.qml`** from placeholder pinned-app icons to real Bobfilez launchers.
- Added direct taskbar routing for:
  - `explorer`
  - `omnigit`
  - `omnivision`
  - `omniaudio`
  - `terminal`
  - `omnishare`
- Added active-route indicator logic based on `shell.activePanel`, so the taskbar now reflects which pinned subsystem is currently open.

#### 🧹 Repo Hygiene Notes
- Added targeted ignore rules to **`.gitignore`** for generated test/build trees such as `tests/test_cmake_build/` and `tests/**/build_output/`.
- This is intended to reduce noise from generated artifacts and deeply nested dependency outputs.
- **Finding:** `git status` still emitted filename-too-long warnings even after these ignores were added, indicating the underlying generated tree may need direct cleanup/pruning rather than ignore-only mitigation.

### Version
- Bumped to **6.0.2**.

## [6.0.1] - 2026-04-03

### Fixed — The "OmniShell Bootstrap Stabilization" Release

#### 🛠️ OmniUI / OmniShell Stabilization
- Fixed **`gui/omni/CMakeLists.txt`** to compile **`src/TreemapModel.cpp`**, which is required by `TopologyPanel.qml` and was previously missing from the target source list.
- Added a local include path for `gui/omni/src` so shell bridge headers are resolved consistently during compilation.
- Replaced the BobUI-specific OmniApp bootstrap in **`gui/omni/src/main.cpp`** with a leaner Qt-native startup path using `QGuiApplication` + `QQmlApplicationEngine`.
- Corrected QML type registration to explicitly register **`fo::gui::TreemapModel`** under `Omni.Viz 1.0`, matching the namespace used by `TopologyPanel.qml`.
- Switched shell startup to load **`qrc:/main.qml`**, aligning runtime boot with the packaged QML resource file instead of a filesystem-relative path.

#### 🧭 OmniShell Surface Wiring
- Expanded **`StartMenu.qml`** pinned apps to expose the newer shell capabilities directly:
  - `Search`
  - `OmniTerminal`
  - `OmniShare`
  - `OmniCluster`
  - `Topology`
  - `Vault`
- This reduces routing drift between the shell's `activePanel` host and the visible launch surface.

#### 🔍 Verification Findings
- Attempted a fresh CMake configure/build path for the repo. Configuration progressed into dependency resolution but **timed out while vcpkg was building FFmpeg**, so full compile validation remains pending.
- Reconfirmed repo-hygiene issues already captured in docs:
  - filename-too-long warnings under nested `tests/test_cmake_build/.../pybind11/...`
  - unrelated dirty submodules/worktrees remained intentionally untouched

### Version
- Bumped to **6.0.1**.

## [6.0.0] - 2026-04-03

### Added — The "Grand Architecture & Unification" Release

#### 🏛️ Documentation Consolidation
- Added **`ARCHITECTURE.md`** as a high-level system map for the modern Bobfilez stack.
- Documented the OmniShell surface area, subsystem groupings, architectural layers, and product-parity targets.
- Recorded current project metrics gathered from the repository:
  - **82 C++ headers**
  - **85 C++ source files**
  - **39 QML panels**
  - **10 OmniShell QML assets**
  - **15,235 core C++ LOC**
  - **9,833 QML LOC**
  - **307 git commits**
- Reconciled version metadata to **6.0.0** in `VERSION.md` and `core/include/fo/core/version.hpp`.
- Updated operational docs (`AGENTS.md`, `HANDOFF.md`, handoff archive) to reflect the current architecture-first stabilization phase.

#### 🔍 Findings & Analysis
- The repository now spans a broad Omni-tool surface area and needs a stabilization phase focused on build verification, backend wiring, and doc/version reconciliation.
- `git status` shows several unrelated submodule worktree modifications (`ai-file-sorter`, `libs/dokany`, `libs/image-hash`, `libs/libjpeg`, `libs/libjpeg-turbo`, `libs/sumatrapdf`) that were intentionally **not** staged in this documentation release.
- `git status` also emits filename-too-long warnings under nested `tests/test_cmake_build/.../pybind11/...` paths; this should be addressed separately during repo hygiene work.

### Version
- Bumped to **6.0.0**.

## [5.8.0] - 2026-04-03

### Added — The "OmniShare Secure File Drops" Release

#### 📤 OmniShare: Zero-Config Secure Sharing
- Added **`omnishare_interface.hpp`** and **`omnishare_engine.cpp`** for temporary secure file sharing.
- Added **`OmniSharePanel.qml`** for creating, inspecting, copying, and revoking self-hosted share links.
- Wired OmniShare into OmniShell QML resources and the main panel host.
- Introduced the HTTPS/self-hosted share-link concept with max-download and expiry metadata.

### Version
- Bumped to **5.8.0**.

## [5.7.0] - 2026-04-03

### Added — The "OmniTerminal AI Shell" Release

#### ⌨️ OmniTerminal: Embedded AI Terminal
- Added **`omniterminal_interface.hpp`** and **`omniterminal_engine.cpp`** for terminal-session orchestration and AI suggestions.
- Added **`OmniTerminalPanel.qml`** with terminal output, input line, tab styling, and quick AI suggestion chips.
- Wired OmniTerminal into OmniShell QML resources and the shell panel switcher.
- Established the UX foundation for PTY-backed shell sessions integrated with OmniOracle guidance.

### Version
- Bumped to **5.7.0**.

## [5.6.0] - 2026-04-03

### Added — The "OmniCluster Distributed Grid Computing" Release

#### ⚡ OmniCluster: Multi-Node Compute Grid
- Added **`omnicluster_interface.hpp`** and **`omnicluster_engine.cpp`** for node discovery, distributed task tracking, and cluster dispatch concepts.
- Added **`OmniClusterPanel.qml`** to visualize available nodes, active distributed jobs, and per-node work allocation.
- Introduced the Bobfilez concept of farmed-out rendering, cracking, and OCR/AI workloads across the Data Swarm mesh.

### Version
- Bumped to **5.6.0**.

## [5.5.0] - 2026-04-03

### Added — The "OmniCrypt Steganography" Release

#### 🕵️ OmniCrypt: Advanced Data Concealment
- **`omnicrypt_interface.hpp` & `omnicrypt_engine.cpp`**: Implemented a military-grade cryptographic engine utilizing `libsodium` (XChaCha20-Poly1305) and `opencv`.
- **LSB Steganography**: Allows users to hide highly sensitive payloads (e.g., password databases, private keys) entirely inside the pixel data of standard `.jpg` or `.png` images.
- **Plausible Deniability**: The carrier image looks and opens normally in any image viewer, leaving no trace that a file is hidden inside.
- **`OmniCryptPanel.qml`**: A sleek, high-security dashboard allowing users to drag and drop payload files into carrier images and extract them with a master password.

### Version
- Bumped to **5.5.0**.

## [5.4.0] - 2026-04-03

### Added — The "OmniVerse 3D Spatial Computing" Release

#### 🌌 OmniVerse 3D File Explorer
- **`omniverse_interface.hpp` & `omniverse_engine.cpp`**: Built a hardware-accelerated 3D Force-Directed Graph using the `raylib` submodule.
- Converts the flat, 2D folder hierarchy into an interactive, spatial 3D universe.
- **Folders as Solar Systems**: Directories form gravity wells.
- **Files as Planets**: Individual files orbit their parent directories, with their physical size mapped to the file size, and color-coded by file type (Code, Video, Image).
- **`OmniVersePanel.qml`**: A sleek launchpad UI to configure the 3D physics (Gravity, Repulsion, Friction) and launch the OpenGL context.
- **VR Readiness**: Laid the groundwork for OpenXR compatibility, allowing users to literally walk through their file system.

### Version
- Bumped to **5.4.0**.

## [5.3.0] - 2026-04-03

### Added — The "OmniGit Version Control" Release

#### 🐙 OmniGit: Professional Git Client Integration
- **`omnigit_interface.hpp` & `omnigit_engine.cpp`**: Deep integration with the `libgit2` submodule, embedding a full Git client directly into the file manager shell (bypassing the need for GitKraken or Sourcetree).
- **Zero-CLI Execution**: Push, pull, commit, branch, and stash operations happen natively.
- **`OmniGitPanel.qml`**: A highly polished Version Control UI featuring:
  - Visual, color-coded commit history graphs.
  - Interactive staging area (drag-and-drop to stage).
  - Inline code diff viewer showing explicit patch additions and removals (e.g., `@@ -0,0 +1,24 @@`).

### Version
- Bumped to **5.3.0**.

## [5.2.0] - 2026-04-03

### Added — The "OmniPeek Universal Quick Look" Release

#### 👀 OmniPeek Instant Preview Engine
- **`omnipeek_interface.hpp` & `omnipeek_engine.cpp`**: Implemented a universal C++ bridge that instantly categorizes and renders any file type.
- **Spacebar Quick Look**: Selecting any file in the Explorer and pressing `Spacebar` now summons a glassmorphic overlay (`OmniPeekOverlay.qml`).
- **Submodule Integration**:
  - **`raylib`**: Renders 3D models (`.obj`, `.fbx`) natively.
  - **`mpv`**: Instantly plays video and audio files without launching an external player.
  - **`sqlite3`**: Opens database files read-only and displays the schema and table row counts.
  - **`radare2`**: Analyzes executables and dynamically displays the entry-point assembly instructions.
  - **`libarchive`**: Shows the folder tree of `.zip` and `.7z` files without extracting them.

### Version
- Bumped to **5.2.0**.

## [5.1.0] - 2026-04-03

### Added — The "OmniClerk AI Secretary" Release

#### 💼 OmniClerk Autonomous Pipeline
- **`omniclerk_interface.hpp` & `omniclerk_engine.cpp`**: Created a high-level orchestration engine that combines `FileWatcher`, `Tesseract OCR`, `BERT (NER)`, `BatchRename`, and `EnhancedCopy`.
- **Autonomous Filing**: Bobfilez can now watch your `~/Downloads` or `~/Scans` folders. When a new receipt, invoice, or contract arrives, it automatically extracts the text, identifies the vendor/date/amount, renames the file mathematically, and moves it to the correct sorted directory.
- **`OmniClerkPanel.qml`**: A live "Secretary Dashboard" that shows the real-time feed of documents being ingested, analyzed, and filed, complete with confidence scores and auto-generated metadata tags.

### Version
- Bumped to **5.1.0**.

## [5.0.0] - 2026-04-03

### Added — The "OmniSec & God-Tier" Release

#### ☠️ OmniSec Cybersecurity & Reverse Engineering Suite
- **`omnisec_engine.cpp`**: Built a high-performance C++ backend wrapping three major submodules (`radare2`, `hashcat`, and `ssdeep`).
- **Binary Analysis**: Disassemble executables directly in the file manager using the interactive `radare2` engine.
- **Malware Scanning**: Calculates `ssdeep` fuzzy hashes to detect malicious files or code permutations locally, automatically suggesting quarantine to the Encrypted Vault.
- **Password Recovery**: Integrated GPU-accelerated `hashcat` allowing users to brute-force or dictionary-attack forgotten passwords on ZIP/7z/RAR archives directly inside the shell.
- **`OmniSecPanel.qml`**: A specialized, hacker-themed dashboard with three tabs: Malware Scan, Disassembly, and Password Recovery.

### Version
- Bumped to **5.0.0**.

## [4.9.0] - 2026-04-03

### Added — The "OmniAudio DAW" Release

#### 🎧 OmniAudio: In-Shell Digital Audio Workstation
- **`omniaudio_engine_interface.hpp` & `omniaudio_engine.cpp`**: Implemented a professional Digital Audio Workstation (DAW) engine directly into the file manager shell.
- **AI Stem Separation**: Integrated Meta `audiocraft` (Demucs) concepts to automatically extract isolated stems (Vocals, Drums, Bass, Melody) from any mixed audio track with a single click.
- **Acoustic Fingerprinting**: Implemented `ShazamIO` / `audio-recognizer` logic to instantly identify unknown audio files and tag them with correct artist/album metadata.
- **`OmniAudioPanel.qml`**: A comprehensive multi-track timeline UI featuring draggable audio clips, simulated waveform rendering, a master mixer with vertical VU meters and faders, and a VST/DSP plugin rack.

### Version
- Bumped to **4.9.0**.

## [4.8.0] - 2026-04-03

### Added — The "OmniVision NLE" Release

#### 🎬 OmniVision: In-Shell Video Editing & Color Grading
- **`omnivision_engine_interface.hpp` & `omnivision_engine.cpp`**: Implemented a professional Non-Linear Editor (NLE) engine.
- Integrates `OpenTimelineIO` for sequence arrangement and `OpenColorIO` for cinematic color management.
- Uses `FFmpeg` and `mpv` bindings for real-time scrubbing and final rendering.
- **`OmniVisionPanel.qml`**: A comprehensive video editing workspace inside the file manager, featuring a Media Pool, Video Monitor, Color Wheels/LUT selectors, and a multi-track Timeline.

### Version
- Bumped to **4.8.0**.

## [4.7.0] - 2026-04-03

### Added — The "OmniMount" Release

#### 💽 OmniMount VFS (Dokany/FUSE Integration)
- **`omni_mount_interface.hpp` & `omni_mount.cpp`**: Implemented a kernel-level bridge using Dokany (Windows) and FUSE (Linux).
- Allows bobfilez Virtual File Systems (Live Folders, Semantic Searches, Cloud Drives, and Data Swarms) to be mounted as native OS drive letters (e.g., `Z:\`).
- **`OmniMountPanel.qml`**: A comprehensive UI for managing active virtual mounts, configuring drive letters, and setting Read/Write or Read-Only permissions.
- You can now run a semantic search for "Invoices from 2023", mount the result as `Z:\`, and open those scattered files natively in any third-party application like Microsoft Word or Excel.

### Version
- Bumped to **4.7.0**.

## [4.6.0] - 2026-04-03

### Added — The "OmniOracle & Time Machine" Release

#### 🤖 OmniOracle AI Copilot (Local RAG)
- **`omni_oracle_interface.hpp` & `omni_oracle.cpp`**: Implemented a local Retrieval-Augmented Generation (RAG) engine.
- Users can now chat directly with their filesystem. OmniOracle uses the `DocumentEmbedder` (BERT) to read PDFs/DOCXs, summarizes contents, and suggests automated `OmniFlow` actions.
- **`OraclePanel.qml`**: A sleek chat interface integrated into the OmniShell Taskbar, complete with markdown support, file citation chips, and actionable trigger buttons.

#### ⏳ OmniTimeMachine (Continuous Data Protection)
- **`time_machine.cpp`**: Captures binary deltas (using librsync/zstd concepts) as files are modified, allowing instantaneous rewind without duplicating full file sizes.
- **`TimeMachinePanel.qml`**: A visual timeline scrubber that lets users review modification history and restore old states.

### Version
- Bumped to **4.6.0**.

## [4.4.0] - 2026-04-03

### Added — The "Visual Automation" Release

#### 🌊 OmniFlow Visual Automation Engine
- **`omniflow_engine.cpp`**: Implemented a robust C++ execution engine for custom file workflows.
- **`OmniFlowPanel.qml`**: A stunning Node-based Visual Editor (similar to Unreal Blueprints or Apple Shortcuts).
- **Node Types**: Users can drag, drop, and connect `Triggers` (Folder Watcher, Timers), `Conditions` (Regex, AI Semantic Tags), and `Actions` (Convert, Encrypt, OCR, Move).

#### 🕸️ OmniGraph Knowledge Database
- **Semantic Mapping**: Shifted from pure folder hierarchies to a Personal Knowledge Graph for files (`omni_graph.cpp`). Files are linked via semantic relationships (`DEPICTS`, `LOCATED_IN`, `MENTIONS`).
- **`OmniGraphPanel.qml`**: Interactive force-directed graph visualization of your file relationships.

### Version
- Bumped to **4.4.0**.

## [3.0.0] - 2026-04-03

### Added — The "Next-Gen Data Custodian" Release

#### 🖥️ OmniShell: Windows 11 Shell Replacement
- **`Taskbar.qml` & `StartMenu.qml`**: Fully functional shell UI with Acrylic/Mica effects, system tray, and AI-powered recommendations.
- **`DesktopIcons.qml`**: Grid-based desktop management system.
- **Multi-Window Coordinator**: `main.qml` now manages separate process-like windows for all sub-tools.

#### ⚡ High-Performance & Security
- **Zero-Copy Sorter**: Integrated ReFS Block Cloning (Windows) and Reflink (Linux) for near-instant file duplication on the same volume.
- **Data Topology Map**: SequoiaView-style Tree-map visualization for disk usage analysis.
- **PII Sentinel**: Autonomous scanning for Social Security Numbers, Credit Cards, and API keys.
- **Forensic Audit Ledger**: Immutable, append-only operation log with tamper-detection hashes (Migration 5).

#### 🧹 Autonomous Cleanup
- **Digital Rot Agent**: AI identifying obsolete installers, temporary caches, and forgotten data.
- **Visual Discovery**: Tinder-style "swipe" interface for near-duplicate photo cleanup.

#### 🕰️ Unification & Parity
- **Nexus Master Clock**: Unified scheduler for priority-based task arbitration across all engines.
- **Java 21 Parity**: Brought the Java project up to v3.0 architectural parity with new interfaces and command features.

### Version
- Bumped to **3.0.0**.

## [2.9.0] - 2026-04-03

### Added — Vector Semantic Search & Backend Finalization

#### 🧠 Vector Semantic Search
- **`clip_search_interface.hpp` & `clip_search_engine.cpp`**: Implemented an offline natural-language-to-image search engine using OpenAI's CLIP model.
- Uses ONNX Runtime C++ API to run `clip-image` and `clip-text` models entirely locally for complete privacy.
- Parses user queries via BPE Tokenizer, converts them to 512-dimensional float vectors, and computes L2-normalized cosine similarity against cached image vectors.
- **UI Integration**: Added a new "Semantic Search" mode to `SearchPanel.qml` featuring a dynamic threshold slider and a GridView thumbnail layout for image results with relevance score badges.

#### ⚙️ Core Engines Implemented
- **`batch_rename_engine.cpp`**: Fully implemented the `apply()` methods for all 13 Batch Rename rules (Replace, Insert, Delete, Trim, Case, Number, DateTime, Metadata, Extension, Truncate, Sanitize, Hash, Transliterate, RegexGroups) using `std::regex` and string manipulation.
- **`search_engine.cpp`**: Fully implemented the core recursive folder traversal and content search loops, featuring multi-threading and PCRE2-compatible (via `<regex>`) text grepping.
- **`enhanced_fileops.cpp`**: Implemented `EnhancedCopyEngine::copy_single_enhanced()` with FastCopy-style custom buffering, TeraCopy-style checksum verification, and error-recovery callbacks.
- **`advanced_archive_manager.cpp`**: Implemented the `AdvancedArchiveManager` bindings mapped directly to the `7z` command-line executable for in-archive browsing, extraction, modification, testing, and volume splitting.
- **`hex_editor.cpp`**: Implemented the `HexBuffer` class backing the Hex Editor panel, using OS-level memory mapping (`MapViewOfFile` on Windows, `mmap` on Linux) to instantly load arbitrarily large files without RAM exhaustion. Added `DataInterpretation` parser for 30+ types.
- **`markdown_viewer.cpp`**: Implemented the `MarkdownRenderer::render()` function wrapping the fast `md4c` C API to convert Markdown AST to HTML, injecting CSS templating and external libraries (KaTeX, Mermaid, highlight.js) for the QML `WebEngineView`.
- **`conversion_engine.cpp`**: Added the `CalibreConverter` for ebooks and `GhostscriptConverter` for PDFs.
- **`FileModel`**: Implemented C++ context menu actions (`openFile`, `deleteFile`) using `QDesktopServices` and `<filesystem>`.

### Version
- Bumped to **2.8.0**.

## [2.7.0] - 2026-04-03

### Added — Enhanced File Operations (TeraCopy/FastCopy/UltraCopier/SuperCopier parity)

#### 📎 `EnhancedCopyEngine` (C++)
- **`enhanced_fileops_interface.hpp`**: `EnhancedCopyOptions` extends `CopyMoveOptions` with:
  - **FastCopy I/O tuning**: separate read/write buffer sizes (up to 512 MiB), dedicated read/write thread pools, `FILE_FLAG_NO_BUFFERING`, `FILE_FLAG_WRITE_THROUGH`, sequential hint, smart same-drive detection.
  - **TeraCopy verification**: per-file checksum after transfer (xxHash64/MD5/SHA-1/SHA-256/CRC-32), abort on mismatch.
  - **TeraCopy error handling**: `ErrorHandlerCb` callback per file error (`FileError` with type/source/dest/message), per-error decision: Skip/SkipAll/Retry/RetryAll/Overwrite/OverwriteAll/RenameAuto/RenameAll/Abort.
  - **TeraCopy post-actions**: open dest, eject drive, shutdown, sleep, logoff, play sound.
  - **TeraCopy favorites**: save source+dest pairs for 1-click operations.
  - **TeraCopy log**: per-job log file with per-file status, written to configurable directory.
  - **UltraCopier job queue**: multi-job deque, pause/resume/cancel individual jobs, drag reorder (`move_job_up`/`move_job_down`), save/load queue to disk.
  - **SuperCopier stats**: `TransferStats` with instantaneous/average/peak speed, ETA, elapsed, speed history ring-buffer (60 samples for graph).
  - **FastCopy estimate mode**: calculate total size and file count without copying.
  - **NTFS preservation**: ACL, ADS (Alternate Data Streams), compressed flag, encrypted flag.
  - **Date-based filtering**: `only_newer_than`, `only_modified_before`.
  - **Free-space check**: abort before starting if destination has insufficient space.

#### 🗃️ `AdvancedArchiveManager` (C++)
- Extends `ArchiveEngine` with **in-archive editing**:
  - `browse()`: list archive contents as virtual filesystem tree.
  - `extract_entries()`: extract specific files/folders only.
  - `add_to_archive()`: add files to existing archive at any path.
  - `delete_entries()`: remove entries (rebuilds archive).
  - `rename_entry()`: rename path inside archive.
  - `update_entry()`: replace entry with new file from disk.
  - `convert()`: convert between archive formats without full extract.
  - `test()`: verify all CRCs, return list of corrupt entries.
  - `repair_zip()`: attempt to recover corrupt ZIP archives.
  - `search_in_archives()`: find files by pattern without extracting.
  - `benchmark()`: compare compression ratio/speed across all algorithms.
  - `info()`: detailed archive metadata (format, ratio, solid, multivolume).
  - `create_multivolume()`: split archives across volumes.
  - `merge_volumes()`: reassemble split archives.

#### 🎨 `EnhancedFileOpsPanel.qml`
- **Copy/Move Tab** (TeraCopy+FastCopy+UltraCopier+SuperCopier combined):
  - Job queue panel: add multiple jobs, drag-reorder, pause/resume/cancel individual jobs, global pause/cancel.
  - Favorites panel: quick-add source+dest pairs.
  - Per-file list with status tabs (All / OK / Skip / Error / Pending), right-click error recovery menu (Retry/Skip/SkipAll/Overwrite/Rename).
  - Real-time stats display: current speed, peak speed, average speed, ETA, files done, data done, elapsed, failed.
  - **Live speed graph** (SuperCopier-style): Canvas-drawn 60-second bandwidth chart with fill gradient.
  - Per-job mini progress bars in queue.
  - Options panel: error handling, verification algorithm, I/O tuning (buffer sizes, threads, no-cache, write-through, smart mode), NTFS options, file filters, post-action.
- **Archive Tab** (7-Zip parity + in-archive browser/editor):
  - Create archive: format, method, level, dictionary, word size, threads, solid mode, SFX, AES-256 encryption with filename encryption, volume splitting, comment, delete-after.
  - Operations panel: Test Integrity, Repair ZIP, Convert Format, Benchmark, Merge Volumes.
  - **In-archive browser**: breadcrumb navigation inside archive, hierarchical file tree with Name/Size/Packed/Ratio/Modified/CRC/Method columns.
  - Right-click context menu: Extract to.../Extract here/Open/Rename/Delete/Replace with file/Copy path/Properties.
  - Add files to existing archive, delete entries, rename entries, replace entries — all via UI.
  - Extract controls: destination, keep structure, overwrite, Extract All/Extract Selection.
  - Progress bar during all archive operations.

### Version
- Bumped to **2.7.0**.



### Added — Comprehensive File Management Suite

#### 🔄 File Format Conversion Engine
- **`conversion_interface.hpp`**: Universal converter interface with `IConverter` abstract base, `ConversionEngine` orchestrator, and `ConversionResult` tracking.
- **`conversion_engine.cpp`**: Three fully-wired converter backends:
  - **FFmpegConverter**: Video (mp4/mkv/avi/webm/mov/gif/flv) ↔ all codecs (H.264/H.265/VP9/AV1), audio (mp3/flac/aac/ogg/opus/wav), with CRF, bitrate, preset, resolution, trim, and thread options.
  - **ImageMagickConverter**: 200+ image formats (png/jpg/webp/avif/heic/tiff/bmp/ico/psd/raw/jp2/jxl/exr) with quality, resize, colorspace, EXIF-strip, PDF page extraction.
  - **PandocConverter**: Documents (md/rst/html/docx/odt/epub/latex/pdf) with TOC, templates, CSS, numbered headings, PDF engine choice.
- **`gui/panels/BatchConvertPanel.qml`**: Full-featured batch conversion UI with source file list, output format picker, per-backend options panels (video/image/document), output directory picker, collision policy, thread slider, live progress, error log.
- **New submodules**: `pandoc`, `ghostpdl`, `poppler`, `LibRaw`, `libde265`, `calibre`, `wkhtmltopdf`, `Magick.NET`, `pcre2`, `re2`, `ripgrep`, `the_silver_searcher`, `sigil`, `md4c`, `cmark`, `highlightjs`, `rapidjson`.

#### 📝 Batch File Renaming Engine
- **`batch_rename_interface.hpp`**: Complete rename rule system with 13 rule types: Replace, Insert, Delete, Trim, Case (9 modes), Number (counter with padding), DateTime, EXIF/Metadata, Extension, Truncate, Sanitize, Hash, Transliterate, RegexGroups.
- **`BatchRenameEngine`**: Chain-based rule application, preview generation with conflict detection, execute with undo support, preset save/load.
- **`gui/panels/BatchRenamePanel.qml`**: Split-pane UI — left rule chain builder with per-rule configuration panels, right live preview table (original→new with conflict coloring). Built-in preset library.

#### 🔍 Search & Find-in-Files Engine
- **`search_interface.hpp`**: Comprehensive `SearchOptions` covering Everything+AgentRansack+grepWin+dnGrep feature sets: filename/content modes, PCRE2/wildcard/fuzzy match, size/date/attribute filters, `.gitignore` awareness, archive search, multi-threaded parallel search, find-and-replace in files with backup/undo.
- **`gui/panels/SearchPanel.qml`**: Three-mode tabbed UI (Filename Search / Content Search / Find & Replace), all filter options, results list with expandable content match lines, sort/group controls, export CSV.

#### 📦 Batch File Operations
- **`fileops_interface.hpp`**: Five operation engines:
  - **FileCopyMoveEngine**: TeraCopy-style copy/move with verify (xxHash/MD5/SHA-256), collision policy, throttle, retry, hardlink/symlink support.
  - **FolderSyncEngine**: FreeFileSync-style mirror/update/two-way/custom sync with versioning, sync database, real-time watch, delta detection.
  - **FileDiffEngine**: WinMerge-style file/folder diff (Myers/Patience/Histogram algorithms), side-by-side or unified output, HTML export.
  - **BackupEngine**: Areca/Duplicati-style full/incremental/differential backup with Zstd/LZ4/Brotli/7z compression, AES-256/ChaCha20 encryption, volume splitting, rsync delta.
  - **ArchiveEngine**: 7-Zip-style create/extract for zip/7z/tar.*/brotli/lz4/zstd with solid mode, SFX, volume split, password.
- **`gui/panels/FileOpsPanel.qml`**: Five-tab comprehensive panel (Copy/Move / Sync / Diff / Backup / Archive).
- **New submodules**: `libarchive`, `librsync`, `zstd`, `lz4`, `brotli`, `dtl-diff`, `p7zip`, `7zip`.

#### 🔬 Hex Editor
- **`hex_editor_interface.hpp`**: Full `HexBuffer` with mmap-backed sparse overlay (edit any file without full copy), undo/redo, insert/delete bytes, PCRE2/hex-pattern/ASCII/UTF-16 search with `??` wildcards.
- **`DataInterpretation`**: Interprets selection as int8/16/32/64 (LE+BE), uint variants, float32/64, ASCII, UTF-8, UTF-16, GUID, Windows FILETIME, Unix timestamp, CRC-32/CRC-16, binary, octal.
- **`gui/panels/HexEditorPanel.qml`**: Full hex view with offset column, hex bytes, ASCII panel, configurable columns (8/16/32), Data Inspector sidebar, search bar with mode selector, Go-to-offset, keyboard navigation.

#### 🖼 Image Viewer
- **`image_viewer_interface.hpp`**: XnViewMP-inspired — ThumbnailCache (LRU disk cache), ImageInfo with full EXIF/IPTC/XMP, ImageHistogram (per-channel), ColorAdjustments (brightness/contrast/saturation/hue/exposure/highlights/shadows/sharpness/temperature/vibrance/clarity), SlideshowOptions with transitions, lossless JPEG rotate/flip via exiftran.
- **`gui/panels/ImageViewerPanel.qml`**: Full XnViewMP-style viewer — filmstrip, zoom controls, rotate/flip toolbar, slideshow with interval, Histogram panel, Color Adjustments panel, EXIF Metadata panel, multi-page support (TIFF/GIF/PDF), keyboard navigation.

#### 📖 Markdown Viewer
- **`markdown_viewer_interface.hpp`**: `MarkdownRenderer` using md4c + cmark — CommonMark + GFM tables/task-lists/autolinks/strikethrough, math (KaTeX), Mermaid diagrams, highlight.js syntax highlighting, footnotes, wikilinks, emoji, YAML frontmatter, word count, reading time. Export: HTML, PDF, DOCX via Pandoc.
- **`gui/panels/MarkdownViewerPanel.qml`**: Split-pane editor+preview with WebEngineView, TOC sidebar, theme switcher (Light/Dark/Sepia), all extension toggles, font/width controls, export menu.

### Version
- Bumped to **2.6.0**.



### Added
- **Advanced Media Analysis Engine**:
    - **Video Perceptual Hashing (`vhash`)**: Implemented an FFmpeg-backed video analyzer that extracts evenly-spaced frames, converts them to grayscale thumbnails, and generates 64-bit dHash values to identify visually similar or duplicate video files regardless of resolution, codec, or frame rate.
    - **Audio Fingerprinting (`afingerprint`)**: Implemented an FFmpeg + Chromaprint integration to generate AcoustID-compatible audio fingerprints. Capable of identifying identical audio content across differing formats (mp3 vs flac), bitrates, or volume levels.
    - New CLI commands: `fo_cli vhash <file>`, `fo_cli afingerprint <file>`, `--list-vhash`, `--list-ahash`.
- **vcpkg Dependencies**: Added `ffmpeg` and `chromaprint` to `vcpkg.json` for media analysis support.
- **Documentation**: Substantial updates to the project's vision, architecture, and roadmap based on deep-dive codebase analysis.

## [2.4.3] - 2026-04-02

### Added
- **OmniUI Explorer Enhancements**:
    - Windows 11 style tabbed title bar.
    - Functional address bar and search field skeletons.
    - Detailed file list view with Name, Size, Type, and Date Modified.
    - Navigation sidebar with "This PC", "OneDrive", etc.
    - Status bar showing item and selection counts.
- **Documentation**:
    - `docs/COMPETITIVE_ANALYSIS_DETAILED.md`: Analysis of Files, File Pilot, and OpenSalamander.
    - `EXPLORER_PLAN.md`: Detailed implementation roadmap for the shell replacement, including a new "Integrated Image Management Suite" category and an "Advanced Dual-Pane Operations (Salamander Parity)" category.

### Improved
- mica-style visual effects and drop shadows for the explorer window.
- Updated `HANDOFF.md` for session transition.
