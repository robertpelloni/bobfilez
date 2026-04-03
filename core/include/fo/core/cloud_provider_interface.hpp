#pragma once
/// @file cloud_provider_interface.hpp
/// @brief Native Cloud Storage integration for bobfilez shell.
///
/// Integrates AWS S3, Google Drive, and Azure Blob Storage as first-class 
/// virtual drives within the OmniShell Explorer. Supports:
///   - Lazy-loading (only fetch metadata on-demand)
///   - Background sync / Offline caching
///   - Unified URI scheme (cloud://s3/bucket/path)

#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <memory>

namespace fo::core {

struct CloudAccount {
    std::string id;
    std::string provider; // "s3", "gdrive", "azure"
    std::string display_name;
    std::string region;
    std::map<std::string, std::string> credentials;
    bool connected = false;
};

class ICloudProvider {
public:
    virtual ~ICloudProvider() = default;

    /// Connect to the cloud service
    virtual bool connect(const CloudAccount& account) = 0;

    /// List directory contents (virtual)
    virtual std::vector<std::filesystem::path> list(const std::string& path) = 0;

    /// Download a file to local cache
    virtual bool download(const std::string& remote_path, const std::filesystem::path& local_dest) = 0;

    /// Upload a local file
    virtual bool upload(const std::filesystem::path& local_src, const std::string& remote_path) = 0;
};

class CloudProviderManager {
public:
    void add_account(const CloudAccount& account);
    void remove_account(const std::string& id);
    std::vector<CloudAccount> get_accounts();

    /// Get the provider for a specific account ID
    std::shared_ptr<ICloudProvider> get_provider(const std::string& account_id);
    
private:
    std::map<std::string, CloudAccount> accounts_;
};

} // namespace fo::core
