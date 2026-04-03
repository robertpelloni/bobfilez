/// @file advanced_archive_manager.cpp
/// @brief Implementation of AdvancedArchiveManager via 7-Zip CLI wrapper.

#include "fo/core/enhanced_fileops_interface.hpp"
#include <sstream>
#include <iostream>
#include <regex>

#ifdef _WIN32
#define popen  _popen
#define pclose _pclose
#endif

namespace fo::core {

static std::pair<int, std::string> run_7z(const std::string& cmd) {
    std::string output;
    // Assuming 7z is in PATH or bundled
    FILE* pipe = popen(("7z " + cmd + " 2>&1").c_str(), "r");
    if (!pipe) return {-1, "Failed to run 7z"};
    char buf[1024];
    while (fgets(buf, sizeof(buf), pipe)) output += buf;
    int rc = pclose(pipe);
    return {rc, output};
}

std::vector<ArchiveBrowserEntry> AdvancedArchiveManager::browse(const std::filesystem::path& archive, const std::string& password) {
    std::vector<ArchiveBrowserEntry> entries;
    std::string cmd = "l -slt \"" + archive.string() + "\"";
    if (!password.empty()) cmd += " -p\"" + password + "\"";

    auto [rc, out] = run_7z(cmd);
    if (rc != 0) return entries;

    // Parse 7z output
    std::istringstream iss(out);
    std::string line;
    ArchiveBrowserEntry current;
    bool reading_list = false;

    while (std::getline(iss, line)) {
        if (line.find("----------") == 0) {
            reading_list = !reading_list;
            continue;
        }
        if (reading_list) {
            if (line.empty()) {
                if (!current.path.empty()) {
                    entries.push_back(current);
                    current = ArchiveBrowserEntry{};
                }
            } else {
                if (line.find("Path = ") == 0) current.path = line.substr(7);
                else if (line.find("Size = ") == 0) current.size = std::stoull(line.substr(7));
                else if (line.find("Packed Size = ") == 0) current.compressed = std::stoull(line.substr(14));
                else if (line.find("Folder = ") == 0) current.is_dir = (line.substr(9) == "+");
                else if (line.find("CRC = ") == 0) current.crc = line.substr(6);
                else if (line.find("Encrypted = ") == 0) current.encrypted = (line.substr(12) == "+");
                else if (line.find("Method = ") == 0) current.compression_method = line.substr(9);
                // Simple parsing for modified date (e.g. Modified = 2024-07-10 14:32:00)
            }
        }
    }
    if (!current.path.empty()) entries.push_back(current);

    return entries;
}

bool AdvancedArchiveManager::extract_entries(const std::filesystem::path& archive,
                                             const std::vector<std::string>& entry_paths,
                                             const ArchiveExtractOptions& opts) {
    std::string cmd = "x \"" + archive.string() + "\" -o\"" + opts.dest_dir.string() + "\"";
    if (!opts.password.empty()) cmd += " -p\"" + opts.password + "\"";
    if (opts.overwrite) cmd += " -aoa";
    else cmd += " -aos"; // skip existing

    for (const auto& p : entry_paths) cmd += " \"" + p + "\"";

    auto [rc, out] = run_7z(cmd);
    return rc == 0;
}

bool AdvancedArchiveManager::add_to_archive(const std::filesystem::path& archive,
                                            const std::vector<std::filesystem::path>& files,
                                            const std::string&,
                                            const ArchiveCreateOptions& opts) {
    std::string cmd = "a \"" + archive.string() + "\"";
    if (!opts.password.empty()) {
        cmd += " -p\"" + opts.password + "\"";
        if (opts.encrypt_filenames) cmd += " -mhe=on";
    }
    if (opts.compression_level >= 0) cmd += " -mx=" + std::to_string(opts.compression_level);
    if (!opts.compression_method.empty()) cmd += " -m0=" + opts.compression_method;

    for (const auto& f : files) cmd += " \"" + f.string() + "\"";

    auto [rc, out] = run_7z(cmd);
    return rc == 0;
}

bool AdvancedArchiveManager::delete_entries(const std::filesystem::path& archive,
                                            const std::vector<std::string>& entry_paths,
                                            const ArchiveCreateOptions&) {
    std::string cmd = "d \"" + archive.string() + "\"";
    for (const auto& p : entry_paths) cmd += " \"" + p + "\"";
    auto [rc, out] = run_7z(cmd);
    return rc == 0;
}

bool AdvancedArchiveManager::rename_entry(const std::filesystem::path& archive,
                                          const std::string& old_path,
                                          const std::string& new_path) {
    std::string cmd = "rn \"" + archive.string() + "\" \"" + old_path + "\" \"" + new_path + "\"";
    auto [rc, out] = run_7z(cmd);
    return rc == 0;
}

bool AdvancedArchiveManager::update_entry(const std::filesystem::path& archive,
                                          const std::string& entry_path,
                                          const std::filesystem::path& new_file) {
    // 7z updates based on relative paths, so typically we'd recreate the file structure
    // temporarily or just use 'u' command.
    std::string cmd = "u \"" + archive.string() + "\" \"" + new_file.string() + "\"";
    auto [rc, out] = run_7z(cmd);
    return rc == 0;
}

bool AdvancedArchiveManager::convert(const std::filesystem::path& src_archive,
                                     const std::filesystem::path& dst_archive,
                                     const ArchiveCreateOptions& opts) {
    // Requires extracting to temp dir, then creating new archive
    auto temp_dir = std::filesystem::temp_directory_path() / generate_uuid();
    std::filesystem::create_directories(temp_dir);

    ArchiveExtractOptions ex_opts;
    ex_opts.dest_dir = temp_dir;
    if (!extract(src_archive, ex_opts)) return false;

    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(temp_dir)) {
        files.push_back(entry.path());
    }

    bool success = create(files, dst_archive, opts);
    std::filesystem::remove_all(temp_dir);
    return success;
}

AdvancedArchiveManager::TestResult AdvancedArchiveManager::test(const std::filesystem::path& archive, const std::string& password) {
    TestResult res;
    std::string cmd = "t \"" + archive.string() + "\"";
    if (!password.empty()) cmd += " -p\"" + password + "\"";
    
    auto [rc, out] = run_7z(cmd);
    res.ok = (rc == 0);
    if (!res.ok) res.error = "7z test failed";
    return res;
}

// Stubs for remaining archive/engine interfaces mapped to base class
bool ArchiveEngine::create(const std::vector<std::filesystem::path>&, const std::filesystem::path&, const ArchiveCreateOptions&, FileOpProgressCb) { return false; }
bool ArchiveEngine::extract(const std::filesystem::path&, const ArchiveExtractOptions&, FileOpProgressCb) { return false; }
std::vector<ArchiveEngine::ArchiveEntry> ArchiveEngine::list(const std::filesystem::path&) { return {}; }
bool ArchiveEngine::verify(const std::filesystem::path&, const std::string&) { return false; }

std::vector<std::pair<std::filesystem::path, ArchiveBrowserEntry>> AdvancedArchiveManager::search_in_archives(
    const std::vector<std::filesystem::path>&, const std::string&, bool) { return {}; }
std::vector<AdvancedArchiveManager::BenchmarkResult> AdvancedArchiveManager::benchmark(
    const std::vector<std::filesystem::path>&, const std::vector<std::string>&) { return {}; }
AdvancedArchiveManager::ArchiveInfo AdvancedArchiveManager::info(const std::filesystem::path&) { return {}; }

bool AdvancedArchiveManager::repair_zip(const std::filesystem::path&, const std::filesystem::path&) { return false; }
bool AdvancedArchiveManager::create_multivolume(const std::vector<std::filesystem::path>&, const std::filesystem::path&, int64_t, const ArchiveCreateOptions&, FileOpProgressCb) { return false; }
bool AdvancedArchiveManager::merge_volumes(const std::vector<std::filesystem::path>&, const std::filesystem::path&) { return false; }

} // namespace fo::core
