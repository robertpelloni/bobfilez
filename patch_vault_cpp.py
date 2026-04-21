import re

with open("core/src/vault_manager.cpp", "r") as f:
    text = f.read()

# Replace std::vector<VaultManager::VaultEntry> with std::vector<VaultEntry>
text = text.replace('std::vector<VaultManager::VaultEntry>', 'std::vector<VaultEntry>')

with open("core/src/vault_manager.cpp", "w") as f:
    f.write(text)
