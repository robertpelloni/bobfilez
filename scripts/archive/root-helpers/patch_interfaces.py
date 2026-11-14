import re

with open("core/include/fo/core/interfaces.hpp", "r") as f:
    text = f.read()

vault_interface = """

// ---------------------------------------------------------
// Vault
// ---------------------------------------------------------
struct VaultEntry {
    std::string id;
    std::string original_name;
    uintmax_t size;
    int64_t added_at;
};

class IVaultManager {
public:
    virtual ~IVaultManager() = default;

    virtual bool initialize(const std::filesystem::path& vault_path, const std::string& password) = 0;
    virtual bool lock_file(const std::filesystem::path& file_path) = 0;
    virtual bool unlock_file(const std::string& vault_id, const std::filesystem::path& dest_path) = 0;
    virtual std::vector<VaultEntry> list_contents() = 0;
};
"""

text = text.replace("} // namespace fo::core", vault_interface + "\n} // namespace fo::core")

with open("core/include/fo/core/interfaces.hpp", "w") as f:
    f.write(text)
