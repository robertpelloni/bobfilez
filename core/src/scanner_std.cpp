#include "fo/core/interfaces.hpp"
#include "fo/core/registry.hpp"
#include <filesystem>
#include <algorithm>

namespace fo::core {

class StdFsScanner : public IFileScanner {
public:
    std::string name() const override { return "std"; }

    std::vector<FileInfo> scan(const std::vector<std::filesystem::path>& roots,
                               const std::vector<std::string>& include_exts,
                               bool follow_symlinks) override {
        std::vector<FileInfo> out;
        auto opts = std::filesystem::directory_options::skip_permission_denied;
        if (follow_symlinks) opts |= std::filesystem::directory_options::follow_directory_symlink;

        auto to_lower = [](std::string s){
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
            return s;
        };
        std::vector<std::string> norm_exts;
        norm_exts.reserve(include_exts.size());
        for (auto e : include_exts) {
            if (!e.empty() && e[0] != '.') e = "." + e;
            norm_exts.push_back(to_lower(e));
        }
        auto accept_ext = [&](const std::filesystem::path& p) {
            if (norm_exts.empty()) return true;
            auto ext = to_lower(p.extension().string());
            if (ext.empty()) return false;
            for (auto& e : norm_exts) if (ext == e) return true;
            return false;
        };

        for (auto& root : roots) {
            if (!std::filesystem::exists(root)) continue;
            std::error_code ec;
            for (std::filesystem::recursive_directory_iterator it(root, opts, ec), end; it != end; ) {
                std::filesystem::directory_entry de;
                try { de = *it; ++it; } catch (...) { ++it; continue; }
                if (!de.is_regular_file(ec)) continue;
                if (!accept_ext(de.path())) continue;
                FileInfo fi;
                fi.uri = de.path().string();
                fi.size = de.file_size(ec);
                std::filesystem::file_time_type ft = de.last_write_time(ec);
                if (!ec) fi.mtime = ft;
                out.push_back(std::move(fi));
            }
        }
        return out;
    }
};

// Static registration
static bool reg_scanner_std = [](){
    Registry<IFileScanner>::instance().add("std", [](){ return std::make_unique<StdFsScanner>(); });
    return true;
}();

void register_scanner_std() { (void)reg_scanner_std; }

} // namespace fo::core
