/// @file remote_storage.cpp
/// @brief Implementation of the Universal Remote Storage system.

#include "fo/core/remote_storage_interface.hpp"
#include <iostream>

namespace fo::core {

void RemoteStorageManager::add_connection(const RemoteConnection& conn) {
    connections_[conn.id] = conn;
    std::cout << "[Remote] Registered connection: " << conn.host << " (" << conn.protocol << ")\n";
}

void RemoteStorageManager::remove_connection(const std::string& id) {
    connections_.erase(id);
}

std::vector<RemoteConnection> RemoteStorageManager::get_connections() {
    std::vector<RemoteConnection> res;
    for (auto& [id, conn] : connections_) res.push_back(conn);
    return res;
}

std::shared_ptr<IRemoteProvider> RemoteStorageManager::get_provider(const std::string& conn_id) {
    // In a real implementation, this returns a specialized provider 
    // using libssh2 for SFTP or cURL for FTP/WebDAV.
    return nullptr;
}

} // namespace fo::core
