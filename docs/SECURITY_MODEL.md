# Security Model

What bobfilez does and does not claim, stated plainly.
If a guarantee is not listed here, it is not made.

## Vault encryption

- **Library/algorithms**: OpenSSL EVP. AES-256-GCM
  (authenticated encryption), PBKDF2-HMAC-SHA256 at
  100,000 iterations with a per-entry 16-byte random
  salt, per-entry 12-byte random IV from `RAND_bytes`.
  No home-grown primitives.
- **Integrity**: GCM tag verified before any plaintext
  is renamed into place; decryption streams to a `.part`
  sibling and only a tag-valid file replaces the
  destination. Corrupt, truncated, tag-flipped, or
  metadata-mangled entries fail as authentication
  errors; `.part` residue is swept on vault open.
- **Password handling**: a `.vault_verify` HMAC verifier
  rejects wrong passwords before listing contents.
  Passwords may come from `FO_VAULT_PASSWORD` (password
  argument `-`) so they need not appear in argv, where
  any local user could read them from process lists.
- **Limitations, explicit**:
  - There is **no password rotation/change API**. Rotating
    means creating a new vault and re-locking files.
  - Legacy vaults created before the verifier existed
    stay ungated for listing (contents remain encrypted);
    the first password to arrive does not get to claim
    them. Re-init to gain the verifier.
  - The vault protects file *contents at rest* against
    readers who lack the password. It does not hide that
    the vault exists, entry sizes, or entry count.

## The operation journal is a trusted local artifact

`operation_log` (undo journal) is **not** treated as
untrusted input for authorization: it lives in the
user's own database, and a local actor who can write it
already has the user's file access. Undo therefore acts
on journal records as its authorization — with two
hardening layers that limit accidents and tampering
blast radius:

- Every record is identity-verified at act time
  (size, content hash, mtime, file ID): a record whose
  evidence no longer matches the file is refused.
- Records that are malformed (empty/relative/
  self-referential paths) or carry **no identity
  evidence at all** are refused, so a forged row
  cannot relocate an arbitrary file unless it also
  accurately describes that file's current identity —
  at which point the actor already had full local
  access. This is tamper *resistance*, not a MAC.
- The forensic audit ledger (separate table) IS
  hash-chained (SHA-256) and verifiable via `fo audit`.

Do not edit `operation_log` by hand and expect undo to
behave; that is outside the supported model.

## Dashboard bridge

The web bridge (`bobui_web/server.js`) performs
destructive filesystem operations and is therefore:

- bound to `127.0.0.1` only,
- rejects non-loopback `Host` headers and non-loopback
  `Origin` headers (when present) with 403,
- caps concurrent CLI subprocesses (excess = 429),
- relays typed errors (see `docs/JSON_SCHEMA.md`).

It has **no authentication** beyond loopback: any local
process running as you can call it. That is the stated
model for a single-user local tool; do not expose the
port beyond loopback, and do not claim it is
authenticated. Origin-less local clients (curl, the
test suite) are intentionally trusted under this model.

## What "verified" means in the audit ledger

`verify_identity`'s fast64 content hash (for files under
`--hash-limit-mb`, default 64) detects accidental change
and ordinary tampering; it is not a cryptographic
tamper proof against an attacker who can recompute it.
Above the cap, identity rests on size + file ID + mtime;
see `docs/JSON_SCHEMA.md` and the changelog for the
residual-risk discussion (v6.5.80–6.5.83).
