#include "fo/core/interfaces.hpp"
#include "fo/core/registry.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <stack>
#include <system_error>

// dirent compatibility (Windows has a shim in libs/dirent)
#include <dirent.h>
#include <sys/stat.h>

namespace fo::core {

class DirentScanner : public IFileScanner {
public:
    std::string name() const override { return "dirent"; }

    std::vector<FileInfo> scan(const std::vector<std::filesystem::path>& roots,
                               const std::vector<std::string>& include_exts,
                               bool /*follow_symlinks*/) override {
        std::vector<FileInfo> out;

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

        std::vector<std::filesystem::path> stack;
        for (auto& r : roots) {
            if (!std::filesystem::exists(r)) continue;
            stack.push_back(r);
        }

        while (!stack.empty()) {
            std::filesystem::path cur = stack.back();
            stack.pop_back();

            DIR* dir = opendir(cur.string().c_str());
            if (!dir) continue;

            while (dirent* de = readdir(dir)) {
                const char* name = de->d_name;
                if (name[0] == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'))) continue;
                std::filesystem::path p = cur / name;

                struct stat stbuf{};
                if (stat(p.string().c_str(), &stbuf) != 0) continue;

                bool is_dir = (stbuf.st_mode & S_IFDIR) != 0;
                if (is_dir) {
                    stack.push_back(p);
                } else {
                    if (!accept_ext(p)) continue;

                    FileInfo fi;
                    fi.uri = p.string();
                    fi.size = static_cast<std::uintmax_t>(stbuf.st_size);

                    // Use std::filesystem for consistent file_clock timestamp
                    std::error_code ec;
                    auto ft = std::filesystem::last_write_time(p, ec);
                    if (!ec) fi.mtime = ft;
                    out.push_back(std::move(fi));
                }
            }
            closedir(dir);
        }
        return out;
    }
};

// Static registration
static bool reg_scanner_dirent = [](){
    Registry<IFileScanner>::instance().add("dirent", [](){ return std::make_unique<DirentScanner>(); });
    return true;
}();

void register_scanner_dirent() { (void)reg_scanner_dirent; }

} // namespace fo::core
