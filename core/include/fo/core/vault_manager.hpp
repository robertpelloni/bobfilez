#pragma once
/// @file vault_manager.hpp
/// @brief Encrypted "Vault" for sensitive files (PII).
///
/// Uses AES-256-GCM to encrypt/decrypt files moved to the vault.
/// Password-protected. Integrated with PII Sentinel to auto-secure data.

#include <string>
#include <filesystem>
#include <vector>

namespace fo::core {

class VaultManager {
public:
    /// Initialize vault at path with password
    bool initialize(const std::filesystem::path& vault_path, const std::string& password);

    /// Move a file into the vault (encrypts it)
    bool lock_file(const std::filesystem::path& file_path);

    /// Extract a file from the vault (decrypts it)
    bool unlock_file(const std::string& vault_id, const std::filesystem::path& dest_path);

    /// List all files currently in the vault
    struct VaultEntry {
        std::string id;
        std::string original_name;
        uintmax_t size;
        int64_t added_at;
    };
    std::vector<VaultEntry> list_contents();

private:
    std::filesystem::path vault_path_;
    std::string password_;

    // Generates a 256-bit key from the password and a salt using PBKDF2
    bool derive_key(const std::string& password, const unsigned char* salt, unsigned char* key) const;

    // Internal encryption function
    bool encrypt_file(const std::filesystem::path& input, const std::filesystem::path& output) const;

    // Internal decryption function
    bool decrypt_file(const std::filesystem::path& input, const std::filesystem::path& output) const;
};

} // namespace fo::core
