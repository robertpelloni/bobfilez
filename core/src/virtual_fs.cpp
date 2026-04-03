/// @file virtual_fs.cpp
/// @brief Implementation of Dynamic Live Folders.

#include "fo/core/virtual_fs_interface.hpp"
#include "fo/core/search_interface.hpp"
#include "fo/core/registry.hpp"
#include <map>

namespace fo::core {

class VirtualFileSystemImpl : public IVirtualFileSystem {
    std::map<std::string, VirtualFolder> folders_;

public:
    VirtualFileSystemImpl() {
        // Add default dynamic folders
        create_folder("Recent Work", "ext:cpp;hpp;py;md dm:today");
        create_folder("Big Downloads", "path:Downloads size:>500mb");
        create_folder("Screenshots", "name:screenshot ext:png;jpg");
    }

    void create_folder(const std::string& name, const std::string& query) override {
        VirtualFolder vf;
        vf.name = name;
        vf.query = query;
        folders_[name] = vf;
    }

    std::filesystem::path resolve(const std::string& vfs_path) override {
        // vfs://Folder/File -> Real Path
        return ""; // Stub
    }

    std::vector<std::filesystem::path> list_folder(const std::string& name) override {
        if (folders_.count(name)) {
            // In a real implementation, we'd call SearchEngine::search(folders_[name].query)
            return {}; 
        }
        return {};
    }

    std::vector<VirtualFolder> get_all_folders() override {
        std::vector<VirtualFolder> res;
        for (auto& [n, f] : folders_) res.push_back(f);
        return res;
    }
};

static auto reg = []() {
    Registry<IVirtualFileSystem>::instance().add("default", []() {
        return std::make_unique<VirtualFileSystemImpl>();
    });
    return true;
}();

} // namespace fo::core
