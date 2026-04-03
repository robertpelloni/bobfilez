#pragma once
/// @file remote_storage_interface.hpp
/// @brief Universal Remote Storage integration (SSH/SFTP, FTP, SMB) for bobfilez.
///
/// Provides a unified interface to access remote filesystems as if they were 
/// local virtual drives. Integrated into the OmniShell Explorer via the VFS.
/// Supported Protocols:
///   - SFTP (via libssh2)
///   - FTP / FTPS (via cURL)
///   - SMB / Samba (via libsmbclient)
///   - WebDAV (via cURL)

#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <memory>

namespace fo::core {

struct RemoteConnection {
    std::string id;
    std::string protocol; // "sftp", "ftp", "smb", "webdav"
    std::string host;
    int port;
    std::string username;
    std::string password;
    std::string root_path;
    bool is_active = false;
};

class IRemoteProvider {
public:
    virtual ~IRemoteProvider() = default;

    /// Establish connection to remote host
    virtual bool connect(const RemoteConnection& conn) = 0;

    /// Disconnect from remote host
    virtual void disconnect() = 0;

    /// List remote directory contents
    virtual std::vector<std::filesystem::path> list(const std::string& path) = 0;

    /// Download a remote file to local
    virtual bool download(const std::string& remote_path, const std::filesystem::path& local_dest) = 0;

    /// Upload a local file to remote
    virtual bool upload(const std::filesystem::path& local_src, const std::string& remote_path) = 0;
};

class RemoteStorageManager {
public:
    void add_connection(const RemoteConnection& conn);
    void remove_connection(const std::string& id);
    std::vector<RemoteConnection> get_connections();

    /// Get the provider for a specific connection ID
    std::shared_ptr<IRemoteProvider> get_provider(const std::string& conn_id);

private:
    std::map<std::string, RemoteConnection> connections_;
};

} // namespace fo::core
