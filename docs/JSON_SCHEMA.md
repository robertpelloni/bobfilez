# CLI JSON Schema Contract (v1)

Every object-shaped `--format=json` payload from `fo_cli` carries
`"schema_version": 1` as its first field. Array roots
(scan, duplicates, hash, metadata, classify, search, history)
are bare arrays and are schema v1 by convention. The web
dashboard bridges (`bobui_web/server.js`) relay payloads
additively: unknown fields pass through to consumers.

## What bumps the version

Removing, renaming, or retyping an existing field is a breaking
change and bumps `schema_version`. Adding a field does not.

## Error envelope (bridges, since v6.5.81)

```json
{ "schema_version": 1, "error": "<human message>",
  "error_code": "FAILED", "retryable": false }
```

`error` stays a string because the legacy React lane renders it
raw; `error_code` is the stable machine-readable half
(`BAD_REQUEST` 400, `USAGE` 400, `FAILED` 422, `TIMEOUT` 504
retryable, `INTERNAL` 500). CLI exit code drives the mapping:
2 → 400, 1 → 422, timeout → 504, anything else → 500.

## Pinning

- CLI side: `JsonOutputsCarrySchemaVersion` (CLI integration
  tests) asserts the field on every object-shaped command.
- Dashboard side: golden wiring tests assert `schema_version`
  and key presence on `/api/stats` and `/api/organize`.

A version bump without matching bridge/test updates fails CI on
at least one side — that is the point of the contract.
