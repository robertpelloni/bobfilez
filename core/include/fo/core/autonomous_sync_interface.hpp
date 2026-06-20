#pragma once
/// @file autonomous_sync_interface.hpp
/// @brief Core file synchronization routine for the autonomous development protocol.

#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <chrono>

namespace fo::core {

struct FileVersion {
    std::string version_id;
    std::chrono::system_clock::time_point timestamp;
    std::string checksum;
    std::string author;
};

struct SyncStatus {
    bool is_in_sync;
    std::vector<std::string> pending_uploads;
    std::vector<std::string> pending_downloads;
    std::string last_error;
};

class ISwarmEngine;

class IAutonomousSyncService {
public:
    virtual ~IAutonomousSyncService() = default;

    /// Provide the swarm engine for broadcasting updates
    virtual void set_swarm_engine(std::shared_ptr<ISwarmEngine> swarm) = 0;

    /// Set the storage path for sync metadata
    virtual void set_storage_path(const std::filesystem::path& path) = 0;

    /// Upload a file and track its version
    virtual bool upload_file(const std::filesystem::path& local_path, const std::string& remote_dest) = 0;

    /// Download a file and update local version tracking
    virtual bool download_file(const std::string& remote_path, const std::filesystem::path& local_dest) = 0;

    /// Get version history for a specific file
    virtual std::vector<FileVersion> get_version_history(const std::string& file_id) = 0;

    /// Check if local and remote are in sync
    virtual SyncStatus check_sync_status() = 0;

    /// Resolve a version collision
    virtual bool resolve_collision(const std::string& file_id, const std::string& preferred_version_id) = 0;

    /// Start the real-time sync daemon monitoring a local path
    virtual void start_sync_daemon(const std::filesystem::path& path) = 0;

    /// Validate the protocol in a live environment (self-test)
    virtual bool validate_live_environment(const std::filesystem::path& test_root) = 0;
};

} // namespace fo::core
