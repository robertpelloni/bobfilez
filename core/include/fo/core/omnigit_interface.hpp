#pragma once
/// @file omnigit_interface.hpp
/// @brief Professional Version Control Engine (libgit2) for bobfilez.
///
/// Integrates Git directly into the file manager shell. Provides
/// repository management, commit graphs, staging, and diffing without
/// relying on external CLI tools.

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

namespace fo::core {

struct GitCommit {
    std::string hash;
    std::string author_name;
    std::string author_email;
    std::string message;
    int64_t timestamp;
    std::vector<std::string> parent_hashes;
};

struct GitFileStatus {
    std::filesystem::path path;
    bool is_staged;
    bool is_modified;
    bool is_untracked;
    bool is_deleted;
    bool is_conflicted;
};

struct GitRepoState {
    std::string current_branch;
    int ahead_of_remote;
    int behind_remote;
    std::vector<GitFileStatus> changes;
};

struct GitDiff {
    std::string file_path;
    int insertions;
    int deletions;
    std::string patch_text; // The unified diff text
};

class IOmniGitEngine {
public:
    virtual ~IOmniGitEngine() = default;

    /// Open an existing repository or initialize a new one
    virtual bool open_repository(const std::filesystem::path& repo_path) = 0;
    virtual bool init_repository(const std::filesystem::path& repo_path) = 0;
    virtual bool clone_repository(const std::string& url, const std::filesystem::path& local_path) = 0;

    /// Get current working directory status
    virtual GitRepoState get_status() = 0;

    /// Get commit history (topological sort)
    virtual std::vector<GitCommit> get_history(int limit = 100) = 0;

    /// Get diff for a specific file or the whole working directory
    virtual std::vector<GitDiff> get_diff() = 0;

    /// Core Git Operations
    virtual bool stage_file(const std::filesystem::path& file) = 0;
    virtual bool unstage_file(const std::filesystem::path& file) = 0;
    virtual bool commit(const std::string& message, const std::string& author, const std::string& email) = 0;
    virtual bool push(const std::string& remote = "origin", const std::string& branch = "main") = 0;
    virtual bool pull(const std::string& remote = "origin", const std::string& branch = "main") = 0;
};

} // namespace fo::core
