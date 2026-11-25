#include "fo/core/lint_interface.hpp"
#include "fo/core/registry.hpp"
#include <filesystem>
#include <regex>
#include <set>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>

namespace fo::core {

class StdLinter : public ILinter {
public:
    std::string name() const override { return "std"; }

    static constexpr std::uintmax_t DEFAULT_LARGE_FILE_THRESHOLD = 100ULL * 1024 * 1024; // 100 MB
    static constexpr int DEFAULT_DEEP_NESTING_THRESHOLD = 10;

    std::vector<LintResult> lint(const std::vector<std::filesystem::path>& roots) override {
        std::vector<LintResult> results;
        std::regex temp_regex(R"(\.(tmp|bak|swp|old|temp|sav|save)$)", std::regex::icase);

        // Collect all filenames for duplicate-name detection
        std::unordered_map<std::string, std::vector<std::filesystem::path>> name_index;

        for (const auto& root : roots) {
            if (!std::filesystem::exists(root)) continue;

            // Manual stack DFS with per-level error codes: the
            // throwing recursive walk died outright (exit 3) on
            // trees containing >260 paths, and the recursive
            // iterator form faults outright in the std scanner.
            // Unenterable subtrees are skipped; the rest lints.
            struct Frame { std::filesystem::path dir; int depth; };
            std::vector<Frame> stack{{root, 0}};
            while (!stack.empty()) {
                auto frame = stack.back();
                stack.pop_back();
                std::error_code dec;
                for (std::filesystem::directory_iterator it(frame.dir, std::filesystem::directory_options::skip_permission_denied, dec), end;
                     it != end; it.increment(dec)) {
                if (dec) { dec.clear(); break; }
                const auto& entry = *it;
                const auto& path = entry.path();
                std::error_code ec;
                int depth = frame.depth + 1;
                std::error_code sec;
                if (entry.is_directory(ec) && !ec && !entry.is_symlink(sec)) {
                    stack.push_back({path, depth});
                }

                // 1. Broken Symlinks
                if (entry.is_symlink()) {
                    if (!std::filesystem::exists(path)) {
                        results.push_back({path, LintType::BrokenSymlink, "Target does not exist"});
                    }
                    continue; 
                }

                // 2. Empty Files
                if (entry.is_regular_file()) {
                    auto fsize = entry.file_size(ec);
                    if (!ec && fsize == 0) {
                        results.push_back({path, LintType::EmptyFile, "File is empty"});
                    }

                    // 3. Temporary Files (by extension/pattern)
                    std::string filename = path.filename().string();
                    std::string ext = path.extension().string();
                    
                    if (std::regex_match(ext, temp_regex) || filename.starts_with("~$") ||
                        filename == "Thumbs.db" || filename == ".DS_Store" || filename == "desktop.ini") {
                        results.push_back({path, LintType::TemporaryFile, "Matches temporary/system file pattern (" + filename + ")"});
                    }

                    // 5. Hidden files (dot-prefix or dollar-prefix, excluding common VCS)
                    if ((filename.starts_with(".") || filename.starts_with("$")) &&
                        filename != "." && filename != ".." &&
                        filename != ".git" && filename != ".gitignore" &&
                        filename != ".gitmodules" && filename != ".gitattributes") {
                        results.push_back({path, LintType::HiddenFile, "Hidden/system file (" + filename + ")"});
                    }

                    // 7. Large files (> 100 MB)
                    if (!ec && fsize > DEFAULT_LARGE_FILE_THRESHOLD) {
                        std::string detail = "File is " + format_size(fsize) + " (> " + format_size(DEFAULT_LARGE_FILE_THRESHOLD) + ")";
                        results.push_back({path, LintType::LargeFile, detail});
                    }

                    // Index for duplicate-name detection
                    std::string lower_name = filename;
                    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
                    name_index[lower_name].push_back(path);
                }

                // 4. Empty Directories
                if (entry.is_directory(ec) && !ec) {
                    if (std::filesystem::is_empty(path, ec) && !ec) {
                        results.push_back({path, LintType::EmptyDirectory, "Directory is empty"});
                    }

                    // 8. Deep nesting (> 10 levels)
                    if (depth > DEFAULT_DEEP_NESTING_THRESHOLD) {
                        results.push_back({path, LintType::DeepNesting, "Nested " + std::to_string(depth) + " levels deep (threshold: " + std::to_string(DEFAULT_DEEP_NESTING_THRESHOLD) + ")"});
                    }
                }
                } // per-entry
            } // stack
        }

        // 6. Duplicate Names (same filename in different directories)
        for (const auto& [name, paths] : name_index) {
            if (paths.size() > 1) {
                // Only report if they're in different parent directories
                std::set<std::string> parent_dirs;
                for (const auto& p : paths) {
                    parent_dirs.insert(p.parent_path().string());
                }
                if (parent_dirs.size() > 1) {
                    std::string detail = "Same filename found in " + std::to_string(parent_dirs.size()) + " directories";
                    for (const auto& p : paths) {
                        results.push_back({p, LintType::DuplicateName, detail});
                    }
                }
            }
        }

        return results;
    }

private:
    static std::string format_size(std::uintmax_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        double val = static_cast<double>(bytes);
        int unit = 0;
        while (val >= 1024.0 && unit < 4) { val /= 1024.0; ++unit; }
        std::ostringstream out;
        if (unit == 0) out << bytes << " B";
        else out << std::fixed << std::setprecision(1) << val << " " << units[unit];
        return out.str();
    }
};

// Static registration
static bool reg_linter_std = [](){
    Registry<ILinter>::instance().add("std", [](){ return std::make_unique<StdLinter>(); });
    return true;
}();

void register_linter_std() { (void)reg_linter_std; }

} // namespace fo::core
