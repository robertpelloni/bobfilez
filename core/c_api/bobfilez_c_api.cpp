#include "fo/c_api/bobfilez_c_api.h"

#include "fo/core/engine.hpp"
#include "fo/core/export.hpp"
#include "fo/core/interfaces.hpp"
#include "fo/core/lint_interface.hpp"
#include "fo/core/operation_repository.hpp"
#include "fo/core/provider_registration.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/rule_engine.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <map>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

thread_local std::string g_last_error;
std::once_flag g_provider_registration_once;

void set_last_error(const std::string& message)
{
    g_last_error = message;
}

void clear_last_error()
{
    g_last_error.clear();
}

char* duplicate_c_string(const std::string& value)
{
    auto* buffer = static_cast<char*>(std::malloc(value.size() + 1));
    if (buffer == nullptr) {
        set_last_error("Failed to allocate result buffer.");
        return nullptr;
    }

    std::memcpy(buffer, value.c_str(), value.size() + 1);
    return buffer;
}

std::filesystem::path parse_root_path(const char* root_path)
{
    if (root_path == nullptr || root_path[0] == '\0') {
        throw std::invalid_argument("A non-empty root path is required.");
    }

    return std::filesystem::path(root_path);
}

std::string parse_ignore_pattern(const char* pattern)
{
    if (pattern == nullptr || pattern[0] == '\0') {
        throw std::invalid_argument("A non-empty ignore pattern is required.");
    }

    return pattern;
}

std::string parse_optional_text(const char* value)
{
    if (value == nullptr) {
        return {};
    }

    return value;
}

std::string current_c_api_db_path()
{
#ifdef _WIN32
    char* env_db_path = nullptr;
    size_t length = 0;
    if (_dupenv_s(&env_db_path, &length, "BOBFILEZ_DB_PATH") == 0 && env_db_path != nullptr && env_db_path[0] != '\0') {
        std::string db_path(env_db_path);
        std::free(env_db_path);
        return db_path;
    }
    std::free(env_db_path);
#else
    const char* env_db_path = std::getenv("BOBFILEZ_DB_PATH");
    if (env_db_path != nullptr && env_db_path[0] != '\0') {
        return env_db_path;
    }
#endif

    return "fo.db";
}

fo::core::FileInfo create_local_file_info(const std::filesystem::path& path)
{
    fo::core::FileInfo file_info;
    file_info.uri = path.string();
    file_info.is_dir = std::filesystem::is_directory(path);
    if (!file_info.is_dir) {
        file_info.size = std::filesystem::file_size(path);
    }
    file_info.mtime = std::filesystem::last_write_time(path);
    return file_info;
}

std::vector<fo::core::FileInfo> scan_with_std_provider(const std::filesystem::path& root)
{
    auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
    if (!scanner) {
        throw std::runtime_error("Scanner provider 'std' not found.");
    }

    return scanner->scan({ root }, {}, false);
}

std::vector<fo::core::FileInfo> collect_files(const std::filesystem::path& root)
{
    if (std::filesystem::exists(root) && std::filesystem::is_regular_file(root)) {
        return { create_local_file_info(root) };
    }

    return scan_with_std_provider(root);
}

std::string make_scan_json(const std::vector<fo::core::FileInfo>& files)
{
    std::ostringstream out;
    out << "[\n";
    for (size_t i = 0; i < files.size(); ++i) {
        out << "  {\"path\": \"" << fo::core::Exporter::json_escape(files[i].uri)
            << "\", \"size\": " << files[i].size
            << ", \"is_dir\": " << (files[i].is_dir ? "true" : "false") << "}";
        if (i + 1 < files.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "]\n";
    return out.str();
}

std::string make_duplicates_json(const std::vector<fo::core::DuplicateGroup>& groups)
{
    std::ostringstream out;
    out << "[\n";
    for (size_t i = 0; i < groups.size(); ++i) {
        const auto& group = groups[i];
        out << "  {\"size\": " << group.size
            << ", \"fast64\": \"" << fo::core::Exporter::json_escape(group.fast64)
            << "\", \"files\": [\n";

        for (size_t j = 0; j < group.files.size(); ++j) {
            const auto& file = group.files[j];
            out << "    {\"path\": \"" << fo::core::Exporter::json_escape(file.uri)
                << "\", \"size\": " << file.size << "}";
            if (j + 1 < group.files.size()) {
                out << ",";
            }
            out << "\n";
        }

        out << "  ]}";
        if (i + 1 < groups.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "]\n";
    return out.str();
}

std::string make_stats_json(const std::vector<fo::core::FileInfo>& files)
{
    std::uintmax_t total_size = 0;
    int directory_count = 0;
    int file_count = 0;
    std::map<std::string, int> extension_counts;
    std::map<std::string, std::uintmax_t> extension_sizes;
    int buckets[5] = { 0, 0, 0, 0, 0 };
    const char* bucket_names[] = { "0-1KB", "1KB-1MB", "1MB-100MB", "100MB-1GB", "1GB+" };

    for (const auto& file : files) {
        if (file.is_dir) {
            ++directory_count;
            continue;
        }

        ++file_count;
        total_size += file.size;

        const auto dot = file.uri.find_last_of('.');
        const auto slash = file.uri.find_last_of("/\\");
        std::string extension = (dot != std::string::npos && (slash == std::string::npos || dot > slash))
            ? file.uri.substr(dot)
            : "(none)";
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        extension_counts[extension]++;
        extension_sizes[extension] += file.size;

        if (file.size < 1024ULL) {
            buckets[0]++;
        } else if (file.size < 1024ULL * 1024) {
            buckets[1]++;
        } else if (file.size < 100ULL * 1024 * 1024) {
            buckets[2]++;
        } else if (file.size < 1024ULL * 1024 * 1024) {
            buckets[3]++;
        } else {
            buckets[4]++;
        }
    }

    std::vector<std::pair<std::string, int>> sorted_extensions(extension_counts.begin(), extension_counts.end());
    std::sort(sorted_extensions.begin(), sorted_extensions.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.second > rhs.second;
    });

    std::ostringstream out;
    out << "{\n"
        << "  \"total_files\": " << file_count << ",\n"
        << "  \"total_directories\": " << directory_count << ",\n"
        << "  \"total_size\": " << total_size << ",\n"
        << "  \"total_size_human\": \"" << fo::core::Exporter::format_size(total_size) << "\",\n"
        << "  \"extensions\": [\n";

    const int extension_limit = std::min<int>(static_cast<int>(sorted_extensions.size()), 20);
    for (int i = 0; i < extension_limit; ++i) {
        out << "    {\"ext\": \"" << fo::core::Exporter::json_escape(sorted_extensions[i].first)
            << "\", \"count\": " << sorted_extensions[i].second
            << ", \"size\": " << extension_sizes[sorted_extensions[i].first] << "}";
        if (i + 1 < extension_limit) {
            out << ",";
        }
        out << "\n";
    }

    out << "  ],\n"
        << "  \"size_distribution\": {\n";

    for (int i = 0; i < 5; ++i) {
        out << "    \"" << bucket_names[i] << "\": " << buckets[i];
        if (i < 4) {
            out << ",";
        }
        out << "\n";
    }

    out << "  }\n"
        << "}\n";
    return out.str();
}

std::string make_hash_json(const std::vector<fo::core::FileInfo>& files)
{
    auto hasher = fo::core::Registry<fo::core::IHasher>::instance().create("fast64");
    if (!hasher) {
        throw std::runtime_error("Hasher provider 'fast64' not found.");
    }

    std::ostringstream out;
    out << "[\n";

    bool first = true;
    for (const auto& file : files) {
        if (file.is_dir) {
            continue;
        }

        if (!first) {
            out << ",\n";
        }
        first = false;

        out << "  {\"path\": \"" << fo::core::Exporter::json_escape(file.uri)
            << "\", \"hash\": \"" << fo::core::Exporter::json_escape(hasher->fast64(std::filesystem::path(file.uri)))
            << "\"}";
    }

    out << "\n]\n";
    return out.str();
}

std::string make_metadata_json(const std::vector<fo::core::FileInfo>& files)
{
    auto provider = fo::core::Registry<fo::core::IMetadataProvider>::instance().create("tinyexif");
    if (!provider) {
        throw std::runtime_error("Metadata provider 'tinyexif' not found.");
    }

    std::ostringstream out;
    out << "[\n";
    bool first = true;

    for (const auto& file : files) {
        if (file.is_dir) {
            continue;
        }

        fo::core::ImageMetadata metadata;
        if (!provider->read(std::filesystem::path(file.uri), metadata)) {
            continue;
        }

        if (!first) {
            out << ",\n";
        }
        first = false;

        out << "  {\"path\": \"" << fo::core::Exporter::json_escape(file.uri) << "\"";
        if (metadata.date.has_taken) {
            const auto time = std::chrono::system_clock::to_time_t(metadata.date.taken);
            std::tm tm_buffer;
#ifdef _WIN32
            localtime_s(&tm_buffer, &time);
#else
            localtime_r(&time, &tm_buffer);
#endif
            std::ostringstream timestamp;
            timestamp << std::put_time(&tm_buffer, "%Y-%m-%dT%H:%M:%S");
            out << ", \"taken\": \"" << timestamp.str() << "\"";
        }
        if (metadata.has_gps) {
            out << ", \"gps_lat\": " << metadata.gps_lat
                << ", \"gps_lon\": " << metadata.gps_lon;
        }
        out << "}";
    }

    out << "\n]\n";
    return out.str();
}

std::string format_taken_timestamp(const fo::core::ImageMetadata& metadata)
{
    if (!metadata.date.has_taken) {
        return "n/a";
    }

    const auto time = std::chrono::system_clock::to_time_t(metadata.date.taken);
    std::tm tm_buffer;
#ifdef _WIN32
    localtime_s(&tm_buffer, &time);
#else
    localtime_r(&time, &tm_buffer);
#endif
    std::ostringstream timestamp;
    timestamp << std::put_time(&tm_buffer, "%Y-%m-%d %H:%M:%S");
    return timestamp.str();
}

std::string make_scan_summary_text(const std::vector<fo::core::FileInfo>& files)
{
    std::uintmax_t total_size = 0;
    int directory_count = 0;
    int file_count = 0;
    std::vector<const fo::core::FileInfo*> sample_files;

    for (const auto& file : files) {
        if (file.is_dir) {
            ++directory_count;
            continue;
        }
        ++file_count;
        total_size += file.size;
        if (sample_files.size() < 20) {
            sample_files.push_back(&file);
        }
    }

    std::ostringstream out;
    out << "Scan Summary\n"
        << "============\n"
        << "Files: " << file_count << "\n"
        << "Directories: " << directory_count << "\n"
        << "Total Size: " << fo::core::Exporter::format_size(total_size) << "\n\n"
        << "First files:\n";

    for (const auto* file : sample_files) {
        out << "- " << file->uri << " (" << fo::core::Exporter::format_size(file->size) << ")\n";
    }

    return out.str();
}

std::string make_duplicates_summary_text(const std::vector<fo::core::DuplicateGroup>& groups)
{
    size_t duplicate_files = 0;
    for (const auto& group : groups) {
        duplicate_files += group.files.size();
    }

    std::ostringstream out;
    out << "Duplicate Summary\n"
        << "=================\n"
        << "Groups: " << groups.size() << "\n"
        << "Files in Groups: " << duplicate_files << "\n\n";

    const size_t limit = std::min<size_t>(groups.size(), 10);
    for (size_t i = 0; i < limit; ++i) {
        const auto& group = groups[i];
        out << "Group " << (i + 1) << ": "
            << group.files.size() << " files, size " << fo::core::Exporter::format_size(group.size)
            << ", fast64=" << group.fast64 << "\n";
        for (const auto& file : group.files) {
            out << "  - " << file.uri << "\n";
        }
        out << "\n";
    }

    return out.str();
}

std::string make_stats_summary_text(const std::vector<fo::core::FileInfo>& files)
{
    std::uintmax_t total_size = 0;
    int directory_count = 0;
    int file_count = 0;
    std::map<std::string, int> extension_counts;

    for (const auto& file : files) {
        if (file.is_dir) {
            ++directory_count;
            continue;
        }

        ++file_count;
        total_size += file.size;

        const auto dot = file.uri.find_last_of('.');
        const auto slash = file.uri.find_last_of("/\\");
        std::string extension = (dot != std::string::npos && (slash == std::string::npos || dot > slash))
            ? file.uri.substr(dot)
            : "(none)";
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        extension_counts[extension]++;
    }

    std::vector<std::pair<std::string, int>> sorted_extensions(extension_counts.begin(), extension_counts.end());
    std::sort(sorted_extensions.begin(), sorted_extensions.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.second > rhs.second;
    });

    std::ostringstream out;
    out << "Statistics Summary\n"
        << "==================\n"
        << "Files: " << file_count << "\n"
        << "Directories: " << directory_count << "\n"
        << "Total Size: " << fo::core::Exporter::format_size(total_size) << "\n\n"
        << "Top Extensions:\n";

    const size_t limit = std::min<size_t>(sorted_extensions.size(), 10);
    for (size_t i = 0; i < limit; ++i) {
        out << "- " << sorted_extensions[i].first << ": " << sorted_extensions[i].second << " files\n";
    }

    return out.str();
}

std::string make_hash_summary_text(const std::vector<fo::core::FileInfo>& files)
{
    auto hasher = fo::core::Registry<fo::core::IHasher>::instance().create("fast64");
    if (!hasher) {
        throw std::runtime_error("Hasher provider 'fast64' not found.");
    }

    std::ostringstream out;
    out << "Hash Summary\n"
        << "============\n";

    size_t shown = 0;
    for (const auto& file : files) {
        if (file.is_dir) {
            continue;
        }

        out << file.uri << "\n"
            << "  fast64: " << hasher->fast64(std::filesystem::path(file.uri)) << "\n";
        if (++shown >= 20) {
            break;
        }
    }

    return out.str();
}

std::string make_metadata_summary_text(const std::vector<fo::core::FileInfo>& files)
{
    auto provider = fo::core::Registry<fo::core::IMetadataProvider>::instance().create("tinyexif");
    if (!provider) {
        throw std::runtime_error("Metadata provider 'tinyexif' not found.");
    }

    std::ostringstream out;
    out << "Metadata Summary\n"
        << "================\n";

    size_t shown = 0;
    for (const auto& file : files) {
        if (file.is_dir) {
            continue;
        }

        fo::core::ImageMetadata metadata;
        if (!provider->read(std::filesystem::path(file.uri), metadata)) {
            continue;
        }

        out << file.uri << "\n"
            << "  taken: " << format_taken_timestamp(metadata) << "\n";
        if (metadata.has_gps) {
            out << "  gps: " << metadata.gps_lat << ", " << metadata.gps_lon << "\n";
        } else {
            out << "  gps: n/a\n";
        }

        out << "\n";
        if (++shown >= 20) {
            break;
        }
    }

    if (shown == 0) {
        out << "No metadata records were read.\n";
    }

    return out.str();
}

std::string lint_type_name(fo::core::LintType type)
{
    switch (type) {
        case fo::core::LintType::EmptyFile:
            return "EmptyFile";
        case fo::core::LintType::EmptyDirectory:
            return "EmptyDirectory";
        case fo::core::LintType::BrokenSymlink:
            return "BrokenSymlink";
        case fo::core::LintType::TemporaryFile:
            return "TemporaryFile";
    }

    return "Unknown";
}

std::vector<fo::core::LintResult> run_lint(const std::filesystem::path& root)
{
    auto linter = fo::core::Registry<fo::core::ILinter>::instance().create("std");
    if (!linter) {
        throw std::runtime_error("Linter provider 'std' not found.");
    }

    return linter->lint({ root });
}

std::string make_lint_json(const std::vector<fo::core::LintResult>& results)
{
    std::ostringstream out;
    out << "[\n";
    for (size_t i = 0; i < results.size(); ++i) {
        out << "  {\"path\": \"" << fo::core::Exporter::json_escape(results[i].path.string())
            << "\", \"type\": \"" << lint_type_name(results[i].type)
            << "\", \"details\": \"" << fo::core::Exporter::json_escape(results[i].details) << "\"}";
        if (i + 1 < results.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "]\n";
    return out.str();
}

std::string make_lint_summary_text(const std::vector<fo::core::LintResult>& results)
{
    std::map<std::string, int> counts;
    for (const auto& result : results) {
        counts[lint_type_name(result.type)]++;
    }

    std::ostringstream out;
    out << "Lint Summary\n"
        << "============\n"
        << "Issues Found: " << results.size() << "\n\n"
        << "Issue Counts:\n";

    for (const auto& [type_name, count] : counts) {
        out << "- " << type_name << ": " << count << "\n";
    }

    out << "\nFirst Issues:\n";
    const size_t limit = std::min<size_t>(results.size(), 20);
    for (size_t i = 0; i < limit; ++i) {
        out << "- [" << lint_type_name(results[i].type) << "] "
            << results[i].path.string() << " — " << results[i].details << "\n";
    }

    if (results.empty()) {
        out << "No lint issues were found.\n";
    }

    return out.str();
}

std::string operation_type_name(fo::core::OperationType type)
{
    switch (type) {
        case fo::core::OperationType::Move:
            return "move";
        case fo::core::OperationType::Copy:
            return "copy";
        case fo::core::OperationType::Rename:
            return "rename";
        case fo::core::OperationType::Delete:
            return "delete";
    }

    return "unknown";
}

std::string format_operation_timestamp(std::chrono::system_clock::time_point timestamp)
{
    const auto raw_time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm tm_buffer;
#ifdef _WIN32
    localtime_s(&tm_buffer, &raw_time);
#else
    localtime_r(&raw_time, &tm_buffer);
#endif
    std::ostringstream out;
    out << std::put_time(&tm_buffer, "%Y-%m-%dT%H:%M:%S");
    return out.str();
}

std::string make_history_json(const std::vector<fo::core::OperationRecord>& operations)
{
    std::ostringstream out;
    out << "[\n";
    for (size_t i = 0; i < operations.size(); ++i) {
        const auto& operation = operations[i];
        out << "  {\"id\": " << operation.id
            << ", \"type\": \"" << operation_type_name(operation.type)
            << "\", \"source\": \"" << fo::core::Exporter::json_escape(operation.source_path)
            << "\", \"dest\": \"" << fo::core::Exporter::json_escape(operation.dest_path)
            << "\", \"timestamp\": \"" << format_operation_timestamp(operation.timestamp)
            << "\", \"undone\": " << (operation.undone ? "true" : "false")
            << "}";
        if (i + 1 < operations.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "]\n";
    return out.str();
}

std::string make_history_summary_text(const std::vector<fo::core::OperationRecord>& operations)
{
    std::map<std::string, int> counts;
    for (const auto& operation : operations) {
        counts[operation_type_name(operation.type)]++;
    }

    std::ostringstream out;
    out << "History Summary\n"
        << "===============\n"
        << "Operations Loaded: " << operations.size() << "\n\n"
        << "Operation Counts:\n";

    for (const auto& [type_name, count] : counts) {
        out << "- " << type_name << ": " << count << "\n";
    }

    out << "\nMost Recent Operations:\n";
    if (operations.empty()) {
        out << "No operation history.\n";
        return out.str();
    }

    const size_t limit = std::min<size_t>(operations.size(), 20);
    for (size_t i = 0; i < limit; ++i) {
        const auto& operation = operations[i];
        out << "- " << format_operation_timestamp(operation.timestamp)
            << " [" << operation_type_name(operation.type) << "] "
            << operation.source_path;
        if (!operation.dest_path.empty()) {
            out << " -> " << operation.dest_path;
        }
        if (operation.undone) {
            out << " (undone)";
        }
        out << "\n";
    }

    return out.str();
}

std::string make_ignore_json(const std::vector<fo::core::IgnoreRule>& rules)
{
    std::ostringstream out;
    out << "[\n";
    for (size_t i = 0; i < rules.size(); ++i) {
        out << "  {\"id\": " << rules[i].id
            << ", \"pattern\": \"" << fo::core::Exporter::json_escape(rules[i].pattern)
            << "\", \"reason\": \"" << fo::core::Exporter::json_escape(rules[i].reason)
            << "\"}";
        if (i + 1 < rules.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "]\n";
    return out.str();
}

std::string make_ignore_summary_text(const std::vector<fo::core::IgnoreRule>& rules)
{
    std::ostringstream out;
    out << "Ignore Rules Summary\n"
        << "====================\n"
        << "Rules Loaded: " << rules.size() << "\n\n"
        << "Configured Rules:\n";

    if (rules.empty()) {
        out << "No ignore rules configured.\n";
        return out.str();
    }

    for (const auto& rule : rules) {
        out << "- " << rule.pattern;
        if (!rule.reason.empty()) {
            out << " (" << rule.reason << ")";
        }
        out << "\n";
    }

    return out.str();
}

std::string make_ignore_action_json(const char* action,
                                    const std::string& pattern,
                                    const std::string& reason)
{
    std::ostringstream out;
    out << "{\n"
        << "  \"action\": \"" << action << "\",\n"
        << "  \"pattern\": \"" << fo::core::Exporter::json_escape(pattern) << "\",\n"
        << "  \"reason\": \"" << fo::core::Exporter::json_escape(reason) << "\"\n"
        << "}\n";
    return out.str();
}

std::string make_ignore_add_summary_text(const std::string& pattern,
                                         const std::string& reason,
                                         const std::vector<fo::core::IgnoreRule>& rules)
{
    std::ostringstream out;
    out << "Ignore Rule Added\n"
        << "=================\n"
        << "Pattern: " << pattern << "\n";

    if (!reason.empty()) {
        out << "Reason: " << reason << "\n";
    }

    out << "Total Rules: " << rules.size() << "\n\n"
        << make_ignore_summary_text(rules);
    return out.str();
}

std::string make_ignore_remove_summary_text(const std::string& pattern,
                                            const std::vector<fo::core::IgnoreRule>& rules)
{
    std::ostringstream out;
    out << "Ignore Rule Removed\n"
        << "===================\n"
        << "Pattern: " << pattern << "\n"
        << "Remaining Rules: " << rules.size() << "\n\n"
        << make_ignore_summary_text(rules);
    return out.str();
}

void ensure_providers_registered()
{
    std::call_once(g_provider_registration_once, []() {
        fo::core::register_all_providers();
    });
}

template <typename BuildJson>
char* execute_json_request(const char* root_path, BuildJson&& build_json)
{
    clear_last_error();

    try {
        ensure_providers_registered();
        const auto root = parse_root_path(root_path);
        const auto files = collect_files(root);
        return duplicate_c_string(build_json(files));
    } catch (const std::exception& error) {
        set_last_error(error.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown bobfilez C API failure.");
        return nullptr;
    }
}

template <typename BuildLint>
char* execute_lint_request(const char* root_path, BuildLint&& build_lint)
{
    clear_last_error();

    try {
        ensure_providers_registered();
        const auto root = parse_root_path(root_path);
        const auto results = run_lint(root);
        return duplicate_c_string(build_lint(results));
    } catch (const std::exception& error) {
        set_last_error(error.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown bobfilez C API failure.");
        return nullptr;
    }
}

template <typename BuildGlobal>
char* execute_global_request(BuildGlobal&& build_global)
{
    clear_last_error();

    try {
        ensure_providers_registered();
        return duplicate_c_string(build_global());
    } catch (const std::exception& error) {
        set_last_error(error.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown bobfilez C API failure.");
        return nullptr;
    }
}

template <typename BuildIgnore>
char* execute_ignore_action_request(const char* pattern,
                                    const char* reason,
                                    BuildIgnore&& build_ignore)
{
    clear_last_error();

    try {
        ensure_providers_registered();
        const std::string parsed_pattern = parse_ignore_pattern(pattern);
        const std::string parsed_reason = parse_optional_text(reason);
        return duplicate_c_string(build_ignore(parsed_pattern, parsed_reason));
    } catch (const std::exception& error) {
        set_last_error(error.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown bobfilez C API failure.");
        return nullptr;
    }
}

template <typename BuildSearch>
char* execute_search_request(const char* root_path, const char* query, BuildSearch&& build_search)
{
    clear_last_error();

    try {
        ensure_providers_registered();
        const auto root = parse_root_path(root_path);
        if (query == nullptr || query[0] == '\0') {
            throw std::invalid_argument("A non-empty search query is required.");
        }
        return duplicate_c_string(build_search(root, query));
    } catch (const std::exception& error) {
        set_last_error(error.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown bobfilez C API failure.");
        return nullptr;
    }
}

} // namespace

extern "C" const char* fo_bobfilez_last_error(void)
{
    return g_last_error.c_str();
}

extern "C" char* fo_bobfilez_scan_json(const char* root_path)
{
    return execute_json_request(root_path, [](const auto& files) {
        return make_scan_json(files);
    });
}

extern "C" char* fo_bobfilez_duplicates_json(const char* root_path)
{
    clear_last_error();

    try {
        ensure_providers_registered();
        const auto root = parse_root_path(root_path);
        fo::core::EngineConfig config;
        config.db_path = ":memory:";
        config.scanner = "std";
        config.hasher = "fast64";
        fo::core::Engine engine(config);

        const auto files = collect_files(root);
        const auto groups = engine.find_duplicates(files);
        return duplicate_c_string(make_duplicates_json(groups));
    } catch (const std::exception& error) {
        set_last_error(error.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown bobfilez C API failure.");
        return nullptr;
    }
}

extern "C" char* fo_bobfilez_stats_json(const char* root_path)
{
    return execute_json_request(root_path, [](const auto& files) {
        return make_stats_json(files);
    });
}

extern "C" char* fo_bobfilez_hash_json(const char* root_path)
{
    return execute_json_request(root_path, [](const auto& files) {
        return make_hash_json(files);
    });
}

extern "C" char* fo_bobfilez_metadata_json(const char* root_path)
{
    return execute_json_request(root_path, [](const auto& files) {
        return make_metadata_json(files);
    });
}

extern "C" char* fo_bobfilez_lint_json(const char* root_path)
{
    return execute_lint_request(root_path, [](const auto& results) {
        return make_lint_json(results);
    });
}

extern "C" char* fo_bobfilez_search_json(const char* root_path, const char* query)
{
    return execute_search_request(root_path, query, [](const auto& root, const char* q) {
        std::ostringstream out;
        out << "[\n";
        bool first = true;
        std::error_code ec;
        std::string query_lower = q;
        std::transform(query_lower.begin(), query_lower.end(), query_lower.begin(), ::tolower);

        for (auto& entry : std::filesystem::recursive_directory_iterator(root, ec)) {
            if (!entry.is_regular_file()) continue;
            auto fname = entry.path().filename().string();
            std::string fname_lower = fname;
            std::transform(fname_lower.begin(), fname_lower.end(), fname_lower.begin(), ::tolower);

            if (fname_lower.find(query_lower) != std::string::npos) {
                if (!first) out << ",\n";
                first = false;
                out << "  {\"path\": \"" << fo::core::Exporter::json_escape(entry.path().string()) << "\"}";
            }
        }
        out << "\n]\n";
        return out.str();
    });
}

extern "C" char* fo_bobfilez_history_json(const char* reserved)
{
    (void) reserved;
    return execute_global_request([]() {
        fo::core::EngineConfig config;
        config.db_path = current_c_api_db_path();
        fo::core::Engine engine(config);
        fo::core::OperationRepository op_repo(engine.database());
        return make_history_json(op_repo.get_all(50));
    });
}

extern "C" char* fo_bobfilez_undo_json(const char* reserved)
{
    (void) reserved;
    return execute_global_request([]() {
        fo::core::EngineConfig config;
        config.db_path = current_c_api_db_path();
        fo::core::Engine engine(config);
        fo::core::OperationRepository op_repo(engine.database());
        auto undone = op_repo.undo_last();
        
        std::ostringstream out;
        if (undone) {
            out << "{\"success\": true"
                << ", \"type\": \"" << operation_type_name(undone->type) << "\""
                << ", \"source\": \"" << fo::core::Exporter::json_escape(undone->source_path) << "\""
                << ", \"dest\": \"" << fo::core::Exporter::json_escape(undone->dest_path) << "\""
                << "}";
        } else {
            out << "{\"success\": false, \"message\": \"No operations to undo\"}";
        }
        return out.str();
    });
}

extern "C" char* fo_bobfilez_ignore_json(const char* reserved)
{
    (void) reserved;
    return execute_global_request([]() {
        fo::core::EngineConfig config;
        config.db_path = current_c_api_db_path();
        fo::core::Engine engine(config);
        return make_ignore_json(engine.ignore_repository().get_all());
    });
}

extern "C" char* fo_bobfilez_ignore_add_json(const char* pattern, const char* reason)
{
    return execute_ignore_action_request(pattern, reason, [](const auto& parsed_pattern, const auto& parsed_reason) {
        fo::core::EngineConfig config;
        config.db_path = current_c_api_db_path();
        fo::core::Engine engine(config);
        engine.ignore_repository().add(parsed_pattern, parsed_reason);
        return make_ignore_action_json("add", parsed_pattern, parsed_reason);
    });
}

extern "C" char* fo_bobfilez_ignore_remove_json(const char* pattern)
{
    return execute_ignore_action_request(pattern, nullptr, [](const auto& parsed_pattern, const auto&) {
        fo::core::EngineConfig config;
        config.db_path = current_c_api_db_path();
        fo::core::Engine engine(config);
        engine.ignore_repository().remove(parsed_pattern);
        return make_ignore_action_json("remove", parsed_pattern, "");
    });
}

extern "C" char* fo_bobfilez_scan_summary_text(const char* root_path)
{
    return execute_json_request(root_path, [](const auto& files) {
        return make_scan_summary_text(files);
    });
}

extern "C" char* fo_bobfilez_duplicates_summary_text(const char* root_path)
{
    clear_last_error();

    try {
        ensure_providers_registered();
        const auto root = parse_root_path(root_path);
        fo::core::EngineConfig config;
        config.db_path = ":memory:";
        config.scanner = "std";
        config.hasher = "fast64";
        fo::core::Engine engine(config);

        const auto files = collect_files(root);
        const auto groups = engine.find_duplicates(files);
        return duplicate_c_string(make_duplicates_summary_text(groups));
    } catch (const std::exception& error) {
        set_last_error(error.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown bobfilez C API failure.");
        return nullptr;
    }
}

extern "C" char* fo_bobfilez_stats_summary_text(const char* root_path)
{
    return execute_json_request(root_path, [](const auto& files) {
        return make_stats_summary_text(files);
    });
}

extern "C" char* fo_bobfilez_hash_summary_text(const char* root_path)
{
    return execute_json_request(root_path, [](const auto& files) {
        return make_hash_summary_text(files);
    });
}

extern "C" char* fo_bobfilez_metadata_summary_text(const char* root_path)
{
    return execute_json_request(root_path, [](const auto& files) {
        return make_metadata_summary_text(files);
    });
}

extern "C" char* fo_bobfilez_lint_summary_text(const char* root_path)
{
    return execute_lint_request(root_path, [](const auto& results) {
        return make_lint_summary_text(results);
    });
}

extern "C" char* fo_bobfilez_search_summary_text(const char* root_path, const char* query)
{
    return execute_search_request(root_path, query, [](const auto& root, const char* q) {
        std::ostringstream out;
        out << "Search Results for '" << q << "' in " << root.string() << "\n"
            << "========================================================\n";
        int count = 0;
        std::error_code ec;
        std::string query_lower = q;
        std::transform(query_lower.begin(), query_lower.end(), query_lower.begin(), ::tolower);

        for (auto& entry : std::filesystem::recursive_directory_iterator(root, ec)) {
            if (!entry.is_regular_file()) continue;
            auto fname = entry.path().filename().string();
            std::string fname_lower = fname;
            std::transform(fname_lower.begin(), fname_lower.end(), fname_lower.begin(), ::tolower);

            if (fname_lower.find(query_lower) != std::string::npos) {
                out << "- " << entry.path().string() << "\n";
                count++;
                if (count >= 100) {
                    out << "... and more results truncated.\n";
                    break;
                }
            }
        }
        if (count == 0) out << "No files found matching the query.\n";
        else out << "\nTotal: " << count << " file(s) found.\n";
        return out.str();
    });
}

extern "C" char* fo_bobfilez_history_summary_text(const char* reserved)
{
    (void) reserved;
    return execute_global_request([]() {
        fo::core::EngineConfig config;
        config.db_path = current_c_api_db_path();
        fo::core::Engine engine(config);
        fo::core::OperationRepository op_repo(engine.database());
        return make_history_summary_text(op_repo.get_all(50));
    });
}

extern "C" char* fo_bobfilez_undo_summary_text(const char* reserved)
{
    (void) reserved;
    return execute_global_request([]() {
        fo::core::EngineConfig config;
        config.db_path = current_c_api_db_path();
        fo::core::Engine engine(config);
        fo::core::OperationRepository op_repo(engine.database());
        auto undone = op_repo.undo_last();

        std::ostringstream out;
        if (undone) {
            out << "Undo Successful\n"
                << "===============\n"
                << "Action: " << operation_type_name(undone->type) << "\n"
                << "Reverted: " << undone->dest_path << " -> " << undone->source_path << "\n";
        } else {
            out << "Nothing to undo.\n";
        }
        return out.str();
    });
}

extern "C" char* fo_bobfilez_ignore_summary_text(const char* reserved)
{
    (void) reserved;
    return execute_global_request([]() {
        fo::core::EngineConfig config;
        config.db_path = current_c_api_db_path();
        fo::core::Engine engine(config);
        return make_ignore_summary_text(engine.ignore_repository().get_all());
    });
}

extern "C" char* fo_bobfilez_ignore_add_summary_text(const char* pattern, const char* reason)
{
    return execute_ignore_action_request(pattern, reason, [](const auto& parsed_pattern, const auto& parsed_reason) {
        fo::core::EngineConfig config;
        config.db_path = current_c_api_db_path();
        fo::core::Engine engine(config);
        engine.ignore_repository().add(parsed_pattern, parsed_reason);
        return make_ignore_add_summary_text(parsed_pattern, parsed_reason, engine.ignore_repository().get_all());
    });
}

extern "C" char* fo_bobfilez_ignore_remove_summary_text(const char* pattern)
{
    return execute_ignore_action_request(pattern, nullptr, [](const auto& parsed_pattern, const auto&) {
        fo::core::EngineConfig config;
        config.db_path = current_c_api_db_path();
        fo::core::Engine engine(config);
        engine.ignore_repository().remove(parsed_pattern);
        return make_ignore_remove_summary_text(parsed_pattern, engine.ignore_repository().get_all());
    });
}

// ── Organize dry-run helpers ────────────────────────────────────────────

namespace {

std::string make_organize_dry_run_json(const std::vector<fo::core::FileInfo>& files,
                                       const std::string& destination_template)
{
    fo::core::RuleEngine rule_engine;
    rule_engine.add_rule({"c_api_rule", "", destination_template});

    std::ostringstream out;
    out << "{\n"
        << "  \"dry_run\": true,\n"
        << "  \"template\": \"" << fo::core::Exporter::json_escape(destination_template) << "\",\n"
        << "  \"total_files\": " << files.size() << ",\n"
        << "  \"moves\": [\n";

    bool first = true;
    size_t move_count = 0;
    for (const auto& file : files) {
        if (file.is_dir) continue;

        auto new_path_opt = rule_engine.apply_rules(file, {});
        if (new_path_opt && new_path_opt->string() != file.uri) {
            if (!first) out << ",\n";
            first = false;
            out << "    {\"source\": \"" << fo::core::Exporter::json_escape(file.uri)
                << "\", \"dest\": \"" << fo::core::Exporter::json_escape(new_path_opt->string())
                << "\"}";
            move_count++;
        }
    }

    out << "\n  ],\n"
        << "  \"move_count\": " << move_count << "\n"
        << "}\n";
    return out.str();
}

std::string make_organize_dry_run_summary_text(const std::vector<fo::core::FileInfo>& files,
                                               const std::string& destination_template)
{
    fo::core::RuleEngine rule_engine;
    rule_engine.add_rule({"c_api_rule", "", destination_template});

    std::ostringstream out;
    out << "Organize Preview (Dry Run)\n"
        << "==========================\n"
        << "Template: " << destination_template << "\n"
        << "Files scanned: " << files.size() << "\n\n";

    size_t move_count = 0;
    const size_t display_limit = 20;
    for (const auto& file : files) {
        if (file.is_dir) continue;

        auto new_path_opt = rule_engine.apply_rules(file, {});
        if (new_path_opt && new_path_opt->string() != file.uri) {
            if (move_count < display_limit) {
                out << "  " << file.uri << "\n"
                    << "    -> " << new_path_opt->string() << "\n";
            }
            move_count++;
        }
    }

    out << "\nTotal moves planned: " << move_count;
    if (move_count > display_limit) {
        out << " (showing first " << display_limit << ")";
    }
    out << "\n";

    return out.str();
}

std::string make_count_json(const std::vector<fo::core::FileInfo>& files,
                            const std::vector<fo::core::DuplicateGroup>& groups)
{
    int file_count = 0;
    int dir_count = 0;
    std::uintmax_t total_size = 0;

    for (const auto& f : files) {
        if (f.is_dir) {
            ++dir_count;
        } else {
            ++file_count;
            total_size += f.size;
        }
    }

    size_t duplicate_files = 0;
    std::uintmax_t wasted_size = 0;
    for (const auto& g : groups) {
        if (g.files.size() > 1) {
            duplicate_files += g.files.size() - 1;
            wasted_size += g.size * (g.files.size() - 1);
        }
    }

    std::ostringstream out;
    out << "{\n"
        << "  \"files\": " << file_count << ",\n"
        << "  \"directories\": " << dir_count << ",\n"
        << "  \"total_size\": " << total_size << ",\n"
        << "  \"total_size_human\": \"" << fo::core::Exporter::format_size(total_size) << "\",\n"
        << "  \"duplicate_groups\": " << groups.size() << ",\n"
        << "  \"duplicate_files\": " << duplicate_files << ",\n"
        << "  \"wasted_size\": " << wasted_size << ",\n"
        << "  \"wasted_size_human\": \"" << fo::core::Exporter::format_size(wasted_size) << "\"\n"
        << "}\n";
    return out.str();
}

std::string make_count_summary_text(const std::vector<fo::core::FileInfo>& files,
                                    const std::vector<fo::core::DuplicateGroup>& groups)
{
    int file_count = 0;
    int dir_count = 0;
    std::uintmax_t total_size = 0;

    for (const auto& f : files) {
        if (f.is_dir) {
            ++dir_count;
        } else {
            ++file_count;
            total_size += f.size;
        }
    }

    size_t duplicate_files = 0;
    std::uintmax_t wasted_size = 0;
    for (const auto& g : groups) {
        if (g.files.size() > 1) {
            duplicate_files += g.files.size() - 1;
            wasted_size += g.size * (g.files.size() - 1);
        }
    }

    std::ostringstream out;
    out << "Count Summary\n"
        << "=============\n"
        << "Files: " << file_count << "\n"
        << "Directories: " << dir_count << "\n"
        << "Total Size: " << fo::core::Exporter::format_size(total_size) << "\n"
        << "Duplicate Groups: " << groups.size() << "\n"
        << "Duplicate Files: " << duplicate_files << "\n"
        << "Wasted Space: " << fo::core::Exporter::format_size(wasted_size) << "\n";
    return out.str();
}

} // namespace

extern "C" char* fo_bobfilez_organize_dry_run_json(const char* root_path, const char* destination_template)
{
    clear_last_error();

    try {
        ensure_providers_registered();
        const auto root = parse_root_path(root_path);
        if (destination_template == nullptr || destination_template[0] == '\0') {
            throw std::invalid_argument("A non-empty destination template is required for organize.");
        }
        const auto files = collect_files(root);
        return duplicate_c_string(make_organize_dry_run_json(files, destination_template));
    } catch (const std::exception& error) {
        set_last_error(error.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown bobfilez C API failure.");
        return nullptr;
    }
}

extern "C" char* fo_bobfilez_organize_dry_run_summary_text(const char* root_path, const char* destination_template)
{
    clear_last_error();

    try {
        ensure_providers_registered();
        const auto root = parse_root_path(root_path);
        if (destination_template == nullptr || destination_template[0] == '\0') {
            throw std::invalid_argument("A non-empty destination template is required for organize.");
        }
        const auto files = collect_files(root);
        return duplicate_c_string(make_organize_dry_run_summary_text(files, destination_template));
    } catch (const std::exception& error) {
        set_last_error(error.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown bobfilez C API failure.");
        return nullptr;
    }
}

extern "C" char* fo_bobfilez_count_json(const char* root_path)
{
    clear_last_error();

    try {
        ensure_providers_registered();
        const auto root = parse_root_path(root_path);
        fo::core::EngineConfig config;
        config.db_path = ":memory:";
        config.scanner = "std";
        config.hasher = "fast64";
        fo::core::Engine engine(config);

        const auto files = collect_files(root);
        const auto groups = engine.find_duplicates(files);
        return duplicate_c_string(make_count_json(files, groups));
    } catch (const std::exception& error) {
        set_last_error(error.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown bobfilez C API failure.");
        return nullptr;
    }
}

extern "C" char* fo_bobfilez_count_summary_text(const char* root_path)
{
    clear_last_error();

    try {
        ensure_providers_registered();
        const auto root = parse_root_path(root_path);
        fo::core::EngineConfig config;
        config.db_path = ":memory:";
        config.scanner = "std";
        config.hasher = "fast64";
        fo::core::Engine engine(config);

        const auto files = collect_files(root);
        const auto groups = engine.find_duplicates(files);
        return duplicate_c_string(make_count_summary_text(files, groups));
    } catch (const std::exception& error) {
        set_last_error(error.what());
        return nullptr;
    } catch (...) {
        set_last_error("Unknown bobfilez C API failure.");
        return nullptr;
    }
}

extern "C" void fo_bobfilez_free_string(char* value)
{
    std::free(value);
}
