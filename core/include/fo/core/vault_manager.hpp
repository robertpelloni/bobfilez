#pragma once
/// @file vault_manager.hpp
/// @brief Encrypted "Vault" for sensitive files (PII).
///
/// Uses AES-256-GCM to encrypt/decrypt files moved to the vault.
/// Password-protected. Integrated with PII Sentinel to auto-secure data.

#include <string>
#include <filesystem>
#include <vector>
#include "fo/core/interfaces.hpp"

namespace fo::core {

class VaultManager : public IVaultManager {
public:
    /// Initialize vault at path with password
    bool initialize(const std::filesystem::path& vault_path, const std::string& password) override;

    /// Move a file into the vault (encrypts it)
    bool lock_file(const std::filesystem::path& file_path) override;

    /// Extract a file from the vault (decrypts it)
    bool unlock_file(const std::string& vault_id, const std::filesystem::path& dest_path) override;

    /// List all files currently in the vault
    std::vector<VaultEntry> list_contents() override;

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
