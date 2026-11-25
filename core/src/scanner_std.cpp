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
            // A file root scans as itself (Hash/Metadata accept files).
            if (std::filesystem::is_regular_file(root, ec)) {
                if (accept_ext(root)) {
                    FileInfo fi;
                    fi.uri = root.string();
                    fi.size = std::filesystem::file_size(root, ec);
                    auto ft = std::filesystem::last_write_time(root, ec);
                    if (!ec) fi.mtime = ft;
                    out.push_back(std::move(fi));
                }
                continue;
            }
            // Manual stack DFS, error codes throughout. The old
            // recursive_directory_iterator walk died with an
            // access violation when a subtree crossed MAX_PATH
            // mid-iteration (both the throwing ++it and the
            // increment(ec) forms fault inside the iterator on
            // this STL). Per-level directory_iterator fails
            // cleanly per subtree: an unenterable directory is
            // skipped and its siblings still scan.
            std::vector<std::filesystem::path> stack{root};
            while (!stack.empty()) {
                auto cur = stack.back();
                stack.pop_back();
                std::error_code dec;
                for (std::filesystem::directory_iterator it(cur, opts, dec), end;
                     it != end; it.increment(dec)) {
                    if (dec) { dec.clear(); break; }
                    const auto& de = *it;
                    std::error_code fec;
                    if (de.is_directory(fec) && !fec) {
                        if (!follow_symlinks && de.is_symlink(fec)) continue;
                        stack.push_back(de.path());
                        continue;
                    }
                    if (!de.is_regular_file(fec) || fec) continue;
                    if (!accept_ext(de.path())) continue;
                    FileInfo fi;
                    fi.uri = de.path().string();
                    fi.size = de.file_size(fec);
                    if (fec) continue;
                    std::filesystem::file_time_type ft = de.last_write_time(fec);
                    if (!fec) fi.mtime = ft;
                    out.push_back(std::move(fi));
                }
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
