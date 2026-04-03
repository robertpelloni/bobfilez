#pragma once
/// @file time_machine_interface.hpp
/// @brief Continuous Data Protection (CDP) and File Versioning for bobfilez.
///
/// OmniTimeMachine tracks changes to files over time by computing binary deltas
/// (using librsync-style rolling checksums) and compressing them (zstd/lz4).
/// Allows users to rewind any file to a previous state instantly.

#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

namespace fo::core {

struct FileRevision {
    std::string revision_id;
    int64_t timestamp;
    uintmax_t file_size;
    uintmax_t delta_size;      // Size of the binary diff
    std::string checksum;      // Hash of the file at this point in time
    std::string trigger_cause; // e.g., "Auto-Save", "Manual Commit", "Before Batch Rename"
};

class ITimeMachine {
public:
    virtual ~ITimeMachine() = default;

    /// Enable continuous protection for a specific directory or file
    virtual void protect_path(const std::filesystem::path& path) = 0;

    /// Force a snapshot of a file's current state
    virtual bool commit_revision(const std::filesystem::path& file, const std::string& cause = "Manual") = 0;

    /// Get the history timeline of a specific file
    virtual std::vector<FileRevision> get_history(const std::filesystem::path& file) = 0;

    /// Reconstruct and restore a file to a specific revision
    virtual bool restore_revision(const std::filesystem::path& file, const std::string& revision_id, const std::filesystem::path& dest_path = "") = 0;

    /// Prune old revisions to free up disk space
    virtual void prune_history(int keep_days) = 0;
};

} // namespace fo::core
