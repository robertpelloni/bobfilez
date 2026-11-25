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
Error responses from the bridges move to a typed envelope
(`{schema_version, error: {code, message, retryable}}`) under
the same breaking-change rule.

## Pinning

- CLI side: `JsonOutputsCarrySchemaVersion` (CLI integration
  tests) asserts the field on every object-shaped command.
- Dashboard side: golden wiring tests assert `schema_version`
  and key presence on `/api/stats` and `/api/organize`.

A version bump without matching bridge/test updates fails CI on
at least one side — that is the point of the contract.
