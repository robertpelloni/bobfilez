/// @file omni_mount.cpp
/// @brief Implementation of the Native VFS Mounter (Dokany/FUSE wrapper).

#include "fo/core/omni_mount_interface.hpp"
#include "fo/core/registry.hpp"
#include <iostream>
#include <map>

// If FO_HAVE_DOKANY or FO_HAVE_FUSE are enabled, we include their headers
// #ifdef FO_HAVE_DOKANY
// #include <dokan/dokan.h>
// #elif defined(FO_HAVE_FUSE)
// #include <fuse.h>
// #endif

namespace fo::core {

class OmniMountImpl : public IOmniMount {
    std::map<std::string, MountPoint> active_mounts_;
    bool driver_loaded_ = false;

public:
    OmniMountImpl() {
        std::cout << "[OmniMount] Checking for OS-level virtual file system drivers (Dokan/FUSE)...\n";
        
        // Mock driver detection for now.
        driver_loaded_ = true; 
        
        // Add a fake mount for UI demonstration
        MountPoint m;
        m.id = "mnt-001";
        m.vfs_source = "semantic://invoices_2024";
#ifdef _WIN32
        m.os_mount_point = "Z:\\";
#else
        m.os_mount_point = "/mnt/bobfilez/invoices";
#endif
        m.is_read_only = false;
        m.status = "Mounted";
        m.virtual_size_bytes = 450000000; // 450 MB of search results
        
        active_mounts_[m.os_mount_point] = m;
    }

    bool mount(const std::string& vfs_source, const std::string& os_mount_point, bool read_only) override {
        if (!driver_loaded_) {
            std::cerr << "[OmniMount] Error: FUSE/Dokan driver not installed or unavailable.\n";
            return false;
        }

        std::cout << "[OmniMount] Mounting VFS: " << vfs_source << " to OS Path: " << os_mount_point << "\n";
        
        // In a real implementation:
        // We initialize DokanOptions and a DokanOperations struct that 
        // intercepts ZwCreateFile, ZwReadFile, etc., and redirects them
        // to the `IVirtualFileSystem` or `SearchEngine` via the Nexus.

        MountPoint m;
        m.id = "mnt-002";
        m.vfs_source = vfs_source;
        m.os_mount_point = os_mount_point;
        m.is_read_only = read_only;
        m.status = "Mounted";
        m.virtual_size_bytes = 100000000;
        
        active_mounts_[os_mount_point] = m;
        return true;
    }

    bool unmount(const std::string& os_mount_point) override {
        if (active_mounts_.count(os_mount_point)) {
            std::cout << "[OmniMount] Unmounting: " << os_mount_point << "\n";
            
            // Call DokanRemoveMountPoint or fuse_unmount
            
            active_mounts_.erase(os_mount_point);
            return true;
        }
        return false;
    }

    std::vector<MountPoint> get_active_mounts() override {
        std::vector<MountPoint> res;
        for (auto const& [path, m] : active_mounts_) res.push_back(m);
        return res;
    }

    bool is_driver_available() const override {
        return driver_loaded_;
    }
};

static auto reg = []() {
    Registry<IOmniMount>::instance().add("default", []() {
        return std::make_unique<OmniMountImpl>();
    });
    return true;
}();

} // namespace fo::core
