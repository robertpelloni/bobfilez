# Release Qualification — v6.5.124

Consolidated statement of what is demonstrated, what is
limited, and what remains an owner decision. Produced at
the reviewers' request as the audit's stopping condition:
after this document, development freezes until actual
user testing identifies additional requirements.

## Demonstrated guarantees (each with a regression test)

- **No silent data loss on any destructive path.**
  Organize/rename refuse existing destinations, unsafe
  names (reserved devices, ADS/illegal chars, trailing
  dot/space in any segment), and junctions escaping the
  root. Delete-duplicates re-verifies content at delete
  time and refuses read-only candidates. Convert refuses
  existing outputs and quote-bearing paths.
- **Crash safety at every journal/filesystem boundary.**
  Intent-before-move journaling with per-point fault
  injection (`FO_FAULT_AT`), startup reconciliation that
  verifies identity (size + content hash) before adopting
  a crashed move, per-row recovery that survives its own
  interruption, and cross-volume copy fallback.
- **Undo never acts on unverifiable evidence.**
  Identity ladder (size → file ID → mtime → content
  hash), malformed-journal refusal (empty/relative/
  self-referential/evidence-free records, NULL-safe
  readers), delete-batch refusal with explicit dismiss.
- **Vault is real encryption.** OpenSSL AES-256-GCM,
  PBKDF2-HMAC-SHA256 @100k, per-entry random salt+IV,
  tag-verified decrypt-to-temp, wrong-password verifier,
  traversal rejection, corruption/truncation/nonce/
  concurrency adversarial suite, env-supplied passwords.
- **Forensic trail.** Hash-chained audit ledger
  (`verified: true`), including recovered operations
  and failed vault attempts; partial batches report
  exactly what ran.
- **Contract discipline.** JSON schema v2 everywhere
  (`{schema_version, items}`), typed bridge errors,
  cross-origin/loopback dashboard boundary, bounded
  subprocess fan-out.
- **Repository hygiene.** Generated build markers and
  root DBs untracked; full-history secrets scan found
  no credential material (owner decision: **never
  purge** — closed).

## Test inventory (v6.5.124)

| Lane | Result |
| --- | --- |
| Lean (gtest) | 135 passed, 1 skipped |
| MSVC (gtest) | 135 passed, 1 skipped |
| JUCE (gtest) | 128 passed, 5 skipped |
| Dashboard wiring (node) | 33 passed |
| ctest (lean, independent runner) | 100% at last full run |

### Skipped tests — explicitly NOT demonstrated

- `DeleteDuplicatesKeepsRealFileOverSymlink`: requires
  file-symlink creation privilege this Windows host does
  not grant; runs where developer mode/admin allows.
- JUCE lane media skips (perceptual/audio providers
  absent in that lane by design): similar/afingerprint
  positive paths are demonstrated in lean/MSVC instead.
- `LockedConvertSourceFailsCleanly` skips in lanes
  without a working convert backend (guarded baseline).

## Known limitations (accepted, documented)

- Journal is a **trusted local artifact** — see
  `docs/SECURITY_MODEL.md`. Identity evidence is
  tamper resistance, not a MAC.
- Undo identity above the 64MB hash cap rests on
  size + file ID + mtime (same-file forged-mtime +
  forged-content is out of model at that size).
- fast64 identity is accidental-change detection, not
  cryptographic tamper proof.
- Dashboard bridge has **no authentication** beyond
  loopback binding + Host/Origin checks; any local
  process running as the user may call it.
- No vault password rotation API (re-init instead);
  legacy pre-verifier vaults list ungated.
- Schema v2 kept `error` a flat string for the legacy
  React lane; `error_code` is the machine half.

## Unresolved risks

- Submodule heads have drifted from recorded pins
  (161 of ~173 gitlinks) — the tree builds and tests
  green as-is, but the pins do not describe it.
- The running dashboard server predates the current
  `server.js`; restart it from the tray to pick up the
  access boundary and schema v2 envelopes.
- BobUI native lane cannot build (fork lacks
  `Qt6QmlConfig.cmake`); BTK lane is Qt4-era. Qt MinGW
  and JUCE are the verified native paths.

## Owner decisions (standing — do not touch)

Submodule pin reconciliation, lean production checkout
authorization, frontend consolidation, `upstream/master`
reconciliation (divergent, no merge base), versioning
policy, product-scope expansion, and the closed
history-purge question.
