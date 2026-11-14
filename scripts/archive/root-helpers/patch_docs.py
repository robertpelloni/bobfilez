with open("CHANGELOG.md", "r") as f:
    text = f.read()

header_marker = "## [6.3.1] - "
if header_marker in text:
    idx = text.find(header_marker)
    idx_end = text.find("\n", idx)
    content_to_insert = """
### Added
- **BobUI Web Vault Support**: Extended the `bobui_web` NodeJS REST API server to support VaultManager operations.
  - POST `/api/vault/init`: Initialize a new encrypted vault
  - POST `/api/vault/lock`: Move and encrypt a file into the vault
  - POST `/api/vault/unlock`: Decrypt and restore a file from the vault
  - POST `/api/vault/list`: Retrieve the vault's current encrypted inventory
- **VaultManager C API**: Added native FFI boundaries for `VaultManager` (`fo_bobfilez_vault_*`) ensuring GUI decoupling.
"""
    text = text[:idx_end+1] + content_to_insert + text[idx_end+1:]
else:
    print("Warning: CHANGELOG marker not found")

with open("CHANGELOG.md", "w") as f:
    f.write(text)

with open("HANDOFF.md", "r") as f:
    text = f.read()

text += "\n\n### BobUI Web Vault API Integration\nThe `bobui_web` server has been updated with REST API endpoints bridging the VaultManager C++ logic to the web interface. Frontend engineers can now hit `/api/vault/init|lock|unlock|list` using `POST` passing `vaultPath` and `password`.\n"

with open("HANDOFF.md", "w") as f:
    f.write(text)
