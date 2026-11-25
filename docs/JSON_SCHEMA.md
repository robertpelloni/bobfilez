# CLI JSON Schema Contract (v2)

Every `--format=json` payload from `fo_cli` — and every JSON
string returned by the `fo_c_api` C ABI — is an object whose
first field is `"schema_version": 2`. Payloads that are
logically lists are wrapped:

```json
{ "schema_version": 2, "items": [ ... ] }
```

(scan, duplicates, hash, metadata, lint, classify, vhash,
afingerprint, search, history, ignore list, C API equivalents).
Bare array roots were schema v1 "by convention" and are gone:
a bare array can never gain a version field, so v2 wraps them
while the consumer set is still in-repo (reviewer round 6).
The web dashboard bridges (`bobui_web/server.js`) relay
payloads additively: unknown fields pass through to consumers.
The unified dashboard renders payloads verbatim; the legacy
React lane unwraps `items` in its fetch helpers.

## What bumps the version

Removing, renaming, or retyping an existing field — including
changing a payload's root shape — is a breaking change and
bumps `schema_version`. Adding a field does not. The v1 → v2
bump (v6.5.93) is the worked example: array roots changed
shape, so every consumer moved in the same commit.

## Error envelope (bridges, since v6.5.81; versioned 2 since v6.5.93)

```json
{ "schema_version": 2, "error": "<human message>",
  "error_code": "FAILED", "retryable": false }
```

`error` stays a string because the legacy React lane renders it
raw; `error_code` is the stable machine-readable half
(`BAD_REQUEST` 400, `USAGE` 400, `FAILED` 422, `TIMEOUT` 504
retryable, `INTERNAL` 500). CLI exit code drives the mapping:
2 → 400, 1 → 422, timeout → 504, anything else → 500. When a
failed run printed a JSON payload before dying (partial
success), the envelope carries it as `partial` so consumers
can show per-file results and offer undo for what landed.

## Pinning

- CLI side: `JsonOutputsCarrySchemaVersion` asserts the field
  on every object-shaped command; `JsonArrayRootsAreWrappedV2`
  asserts the wrapped shape (leading `{`, `schema_version`,
  `items`) on every array-shaped command.
- Dashboard side: golden wiring tests assert `schema_version`
  and key presence on `/api/stats` and `/api/organize`, and
  the wrapped shape on `/api/scan` and `/api/hash`.

A version bump without matching bridge/test updates fails CI on
at least one side — that is the point of the contract.
