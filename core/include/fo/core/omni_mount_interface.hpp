#pragma once
/// @file omni_mount_interface.hpp
/// @brief Native OS Virtual Drive Mounting (Dokany/FUSE) for bobfilez.
///
/// Exposes bobfilez Virtual File Systems (Live Folders, Semantic Searches, 
/// OmniGraph queries) as native OS drives (e.g., Z:\ on Windows, or a mount
/// point on Linux/macOS). Allows third-party apps (Word, Photoshop) to 
/// interact with AI-generated file collections seamlessly.

#include <string>
#include <vector>
#include <memory>

namespace fo::core {

struct MountPoint {
    std::string id;
    std::string vfs_source;       // e.g., "vfs://RecentWork" or "semantic://invoices_2023"
    std::string os_mount_point;   // e.g., "Z:\\" or "/mnt/bobfilez/invoices"
    bool is_read_only;
    std::string status;           // "Mounted", "Unmounting", "Error"
    uintmax_t virtual_size_bytes; // Simulated size of the drive
};

class IOmniMount {
public:
    virtual ~IOmniMount() = default;

    /// Mount a VFS source to an OS mount point (Drive Letter or Path)
    /// This initiates the Dokany/FUSE background driver loop.
    virtual bool mount(const std::string& vfs_source, const std::string& os_mount_point, bool read_only = false) = 0;

    /// Unmount a currently active drive
    virtual bool unmount(const std::string& os_mount_point) = 0;

    /// Get all active mounts managed by this instance
    virtual std::vector<MountPoint> get_active_mounts() = 0;

    /// Check if the underlying kernel driver (Dokany/FUSE) is installed and available
    virtual bool is_driver_available() const = 0;
};

} // namespace fo::core
