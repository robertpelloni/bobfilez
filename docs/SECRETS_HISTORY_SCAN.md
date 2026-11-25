# Secrets-in-History Scan (read-only, 2026-11-25)

Question (roast rounds 8/9/12): the earlier sweep covered the
current tree only. `fo.db`, `index.db`, and `metamcp.db` are
in git history — did the sweep establish "nothing to rotate"?

## Method

Every blob version of every `*.db` path ever committed was
extracted (`git rev-list --objects --all` + `cat-file`),
opened read-only as SQLite, and checked for:

- table inventory,
- row counts in credential-shaped tables
  (token/secret/api-key/auth/credential/password/config/
  session in the name),
- raw-blob high-signal credential patterns
  (`sk-…`, `ghp_…`, `AKIA…`, `Bearer …`, `api_key=…`,
  private-key blocks).

No values were printed or read beyond counts and pattern
hit counts. The purge decision itself is unchanged and
belongs to the repo owner; this document is the evidence.

## Results

- **metamcp.db** (1 version, 9.0 MB): every credential-bearing
  table is EMPTY — `api_keys` 0, `oauth_access_tokens` 0,
  `oauth_clients` 0, `oauth_authorization_codes` 0,
  `oauth_sessions` 0, `sessions` 0, `config` 0,
  `workspace_secrets` 0, `docker_sessions` 0. Bulk is
  `imported_sessions` (896), `imported_session_memories`
  (2851), `published_mcp_config_recipes` (400) — session
  content, not credentials. Zero pattern hits.
- **fo.db** (8 versions) and **bobui_web/fo.db** (14):
  scan/file tables only; zero pattern hits.
- **index.db**: the 0-byte empty blob. **locate.db**: 2
  bytes, not SQLite. **ignore.db**: `IGNOREWINDOWS` table
  (directory-listing data). **test_incremental.db**: scan
  tables only. Zero pattern hits anywhere.

## Conclusion and limits

No credential material found in any committed database:
the tables that would hold tokens were never populated in
history, and no known credential format appears in any
blob. "Nothing to rotate" is now established for the
database history by structure and pattern, not assumption.

Limits, stated plainly: the pattern set is high-signal, not
exhaustive, and `imported_session_memories` holds transcript
text that could contain incidental secrets pasted into
chats. Ruling that out would mean reading the content,
which this scan deliberately did not do. Whether to purge
9 MB of session history from a public repo remains the
owner's privacy call — it is not a credential emergency.
