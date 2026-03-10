#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <system_error>

#include "fo/core/interfaces.hpp"
#include "fo/core/registry.hpp"

namespace fo::core {

static std::wstring to_wstring(const std::filesystem::path& p) {
    return p.wstring();
}

static std::filesystem::path join_path(const std::filesystem::path& a, const std::wstring& b) {
    std::filesystem::path r = a;
    r /= std::filesystem::path(b);
    return r;
}

class Win32Scanner : public IFileScanner {
public:
    std::string name() const override { return "win32"; }

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

        // Iterative DFS stack
        std::vector<std::filesystem::path> stack;
        for (auto& r : roots) {
            if (!std::filesystem::exists(r)) continue;
            stack.push_back(r);
        }

        WIN32_FIND_DATAW ffd;
        while (!stack.empty()) {
            std::filesystem::path cur = stack.back();
            stack.pop_back();

            std::wstring pattern = to_wstring(cur);
            if (!pattern.empty() && pattern.back() != L'\\' && pattern.back() != L'/') pattern += L"\\";
            pattern += L"*";

            HANDLE hFind = FindFirstFileExW(pattern.c_str(), FindExInfoBasic, &ffd, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
            if (hFind == INVALID_HANDLE_VALUE) {
                continue; // skip unreadable folders
            }

            do {
                const wchar_t* name = ffd.cFileName;
                if (name[0] == L'.' && (name[1] == 0 || (name[1] == L'.' && name[2] == 0))) continue; // . or ..

                bool is_dir = (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                std::filesystem::path p = join_path(cur, name);

                if (is_dir) {
                    stack.push_back(p);
                } else {
                    if (!accept_ext(p)) continue;

                    FileInfo fi;
                    fi.uri = p.string();
                    ULARGE_INTEGER sz; sz.LowPart = ffd.nFileSizeLow; sz.HighPart = ffd.nFileSizeHigh;
                    fi.size = static_cast<std::uintmax_t>(sz.QuadPart);

                    // Optimization: Convert FILETIME directly to file_time_type
                    // This avoids opening the file handle again via std::filesystem
                    ULARGE_INTEGER date_val;
                    date_val.LowPart = ffd.ftLastWriteTime.dwLowDateTime;
                    date_val.HighPart = ffd.ftLastWriteTime.dwHighDateTime;
                    
                    // MSVC std::chrono::file_clock uses 100ns ticks since 1601-01-01, matching FILETIME.
                    fi.mtime = std::chrono::file_clock::time_point(std::chrono::file_clock::duration(date_val.QuadPart));

                    out.push_back(std::move(fi));
                }
            } while (FindNextFileW(hFind, &ffd));

            FindClose(hFind);
        }
        return out;
    }
};

// Static registration
static bool reg_scanner_win32 = [](){
    Registry<IFileScanner>::instance().add("win32", [](){ return std::make_unique<Win32Scanner>(); });
    return true;
}();

void register_scanner_win32() { (void)reg_scanner_win32; }

} // namespace fo::core

#endif // _WIN32
