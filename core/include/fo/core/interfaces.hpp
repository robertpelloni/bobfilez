#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <optional>

namespace fo::core {

class IFileScanner {
public:
    virtual ~IFileScanner() = default;
    virtual std::string name() const = 0;
    virtual std::vector<FileInfo> scan(
        const std::vector<std::filesystem::path>& roots,
        const std::vector<std::string>& include_exts,
        bool follow_symlinks) = 0;
};

class IHasher {
public:
    virtual ~IHasher() = default;
    virtual std::string name() const = 0;
    virtual std::string fast64(const std::filesystem::path& p) = 0;
    virtual std::optional<std::string> strong(const std::filesystem::path& p) { (void)p; return std::nullopt; }
    virtual std::string strong_algo() const { return ""; }
};

class IMetadataProvider {
public:
    virtual ~IMetadataProvider() = default;
    virtual std::string name() const = 0;
    virtual bool read(const std::filesystem::path& p, ImageMetadata& out) = 0;
    virtual bool write(const std::filesystem::path& p, const ImageMetadata& in) { (void)p; (void)in; return false; }
};

struct DuplicateGroup {
    std::uintmax_t size = 0;
    std::string fast64;
    std::vector<FileInfo> files;
};

class IDuplicateFinder {
public:
    virtual ~IDuplicateFinder() = default;
    virtual std::string name() const = 0;
    virtual std::vector<DuplicateGroup> group(const std::vector<FileInfo>& files, IHasher& hasher) = 0;
};



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

} // namespace fo::core
