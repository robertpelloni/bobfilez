#pragma once
/// @file omnishare_interface.hpp
/// @brief Zero-Config Secure File Sharing for bobfilez.
///
/// OmniShare generates temporary, encrypted download links for any file.
/// No cloud account needed. Uses a built-in HTTP server with end-to-end
/// encryption (libsodium) and auto-expiring links.

#include <string>
#include <vector>
#include <filesystem>

namespace fo::core {

struct ShareLink {
    std::string id;
    std::string url;            // e.g., "https://your-ip:8443/share/abc123"
    std::filesystem::path file;
    int max_downloads;
    int current_downloads;
    int expires_minutes;
    bool is_password_protected;
    bool is_active;
};

class IOmniShareEngine {
public:
    virtual ~IOmniShareEngine() = default;

    /// Create a temporary share link for a file
    virtual ShareLink create_share(const std::filesystem::path& file, int max_downloads = 5, int expire_mins = 60, const std::string& password = "") = 0;

    /// Revoke an active share link
    virtual bool revoke_share(const std::string& share_id) = 0;

    /// Get all active share links
    virtual std::vector<ShareLink> get_active_shares() = 0;

    /// Start the embedded HTTPS server
    virtual bool start_server(int port = 8443) = 0;
};

} // namespace fo::core
