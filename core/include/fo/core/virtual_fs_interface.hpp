#pragma once
/// @file virtual_fs_interface.hpp
/// @brief Dynamic "Live Folders" for bobfilez.
///
/// Virtual File System (VFS) allows presenting search results, rule-based 
/// categories, and "Shadow Copies" as standard directories.
/// Examples:
///   - "Recent Work" (Dynamic search for .cpp files modified in 24h)
///   - "Big Videos" (Dynamic filter for >1GB .mp4)
///   - "To Clean" (Output of Digital Rot Agent)

#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace fo::core {

struct VirtualFolder {
    std::string name;
    std::string query; // Everything-style query string
    std::vector<std::filesystem::path> static_files;
    bool is_dynamic = true;
};

class IVirtualFileSystem {
public:
    virtual ~IVirtualFileSystem() = default;

    /// Create a new virtual folder
    virtual void create_folder(const std::string& name, const std::string& query) = 0;

    /// Resolve a virtual path (e.g. "vfs://RecentWork/main.cpp") to its real location
    virtual std::filesystem::path resolve(const std::string& vfs_path) = 0;

    /// List contents of a virtual folder
    virtual std::vector<std::filesystem::path> list_folder(const std::string& name) = 0;

    /// Get all registered virtual folders
    virtual std::vector<VirtualFolder> get_all_folders() = 0;
};

} // namespace fo::core
