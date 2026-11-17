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
            // A file root scans as itself (Hash/Metadata accept files).
            struct stat root_st{};
            if (stat(r.string().c_str(), &root_st) == 0 && (root_st.st_mode & S_IFREG)) {
                if (accept_ext(r)) {
                    FileInfo fi;
                    fi.uri = r.string();
                    fi.size = static_cast<std::uintmax_t>(root_st.st_size);
                    std::error_code ec;
                    auto ft = std::filesystem::last_write_time(r, ec);
                    if (!ec) fi.mtime = ft;
                    out.push_back(std::move(fi));
                }
                continue;
            }
            stack.push_back(r);
        }

#ifdef _WIN32
        // The vendored dirent shim stores the CRT find handle (pointer-sized
        // since VS2015) in a 32-bit long, which truncates it and crashes in
        // readdir with the modern MSVC CRT. Walk with std::filesystem here;
        // POSIX keeps the real dirent loop below.
        for (auto& root : stack) {
            std::error_code ec;
            for (std::filesystem::recursive_directory_iterator it(root, ec), end; it != end; it.increment(ec)) {
                if (ec) break;
                std::error_code fec;
                if (!it->is_regular_file(fec) || !accept_ext(it->path())) continue;
                FileInfo fi;
                fi.uri = it->path().string();
                fi.size = it->file_size(fec);
                auto ft = it->last_write_time(fec);
                if (!fec) fi.mtime = ft;
                out.push_back(std::move(fi));
            }
        }
        return out;
#endif

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
