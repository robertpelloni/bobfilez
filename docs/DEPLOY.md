# Deployment Guide

## v6.3.4 Autonomous & Distributed Staging

### Staging Script
A new staging script `scripts/deploy_staging.sh` has been added to automate the build-test-package cycle.

#### Usage:
```bash
bash scripts/deploy_staging.sh
```

### Environment Configuration
The following environment variables can be used to customize the autonomous ecosystem:

- `BOBFILEZ_SYNC_DB`: Path to the SQLite database for file version tracking (defaults to `sync.db`).
- `BOBFILEZ_CLI`: Override path to the `fo_cli` binary for UI bridges.
- `BOBFILEZ_NATIVE_UI_PROFILE`: Select a native UI launch profile (e.g., `omni-explorer-only`).

### Distributed Sync Nodes
When simulating multiple nodes on a single host for testing or staging:
1. Ensure each node has a unique local root path.
2. Provide a unique `BOBFILEZ_SYNC_DB` per instance to avoid lock contention.
3. Node updates are broadcast via the internal `SwarmHub` simulation (Real mDNS/UDP discovery planned for Phase 11).
