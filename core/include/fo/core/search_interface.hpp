#pragma once
/// @file search_interface.hpp
/// @brief Comprehensive filename + content search engine for bobfilez.
///
/// Combines features from:
///   Everything (voidtools)    — instant filename search, regex, size/date filters
///   AgentRansack / FileLocator — GUI content search, advanced filters
///   grepWin                   — regex content search with replace
///   dnGrep                    — everything + content, replacement, archive search
///   ripgrep                   — extremely fast content search engine
///   the_silver_searcher (ag)  — content search respecting .gitignore
///
/// Two distinct search modes:
///   1. FILENAME SEARCH  — searches file names/paths (like Everything)
///   2. CONTENT SEARCH   — searches inside file text content (like grepWin)
///
/// Both modes support: regex, wildcards, case options, size/date filters,
/// attribute filters, recursive depth, result exclusions, and live preview.

#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <functional>
#include <optional>
#include <chrono>
#include <cstdint>

namespace fo::core {

//───────────────────────── Search Filter Options ────────────────────────────//

/// Date range filter (used for created/modified/accessed)
struct DateRangeFilter {
    bool enabled = false;
    std::chrono::system_clock::time_point from;
    std::chrono::system_clock::time_point to;
    enum class Field { Modified, Created, Accessed } field = Field::Modified;
};

/// File size range filter
struct SizeRangeFilter {
    bool enabled = false;
    std::uintmax_t min_bytes = 0;
    std::uintmax_t max_bytes = UINT64_MAX;
};

/// File attribute filter (Windows & cross-platform)
struct AttributeFilter {
    std::optional<bool> read_only;    // true = only RO, false = only non-RO
    std::optional<bool> hidden;
    std::optional<bool> system;
    std::optional<bool> archive;
    std::optional<bool> is_directory;
    std::optional<bool> is_symlink;
};

/// Comprehensive search options — covers all filter capabilities of
/// Everything, AgentRansack, grepWin, and dnGrep combined.
struct SearchOptions {
    // ── Scope ──────────────────────────────────────────────────────────
    std::vector<std::filesystem::path> search_roots;
    bool recursive = true;
    int max_depth = -1;             // -1 = unlimited
    bool follow_symlinks = false;
    bool search_archives = false;   // Search inside zip/tar/7z (like dnGrep)

    // ── Pattern matching ───────────────────────────────────────────────
    std::string query;              // The search term
    enum class MatchMode {
        Literal,                    // Plain substring (default)
        Wildcard,                   // * and ? globs
        Regex,                      // Full PCRE2 regex
        FuzzyPath                   // Fuzzy path matching (like fzf)
    } match_mode = MatchMode::Literal;

    bool case_sensitive = false;
    bool whole_word = false;        // Match whole words only
    bool match_path = false;        // Match against full path, not just name
    bool invert_match = false;      // Return non-matching files

    // ── Filename search filters ────────────────────────────────────────
    std::vector<std::string> include_extensions; // e.g. {"cpp","hpp"}
    std::vector<std::string> exclude_extensions;
    std::vector<std::string> include_dirs;       // Only search in these dirs
    std::vector<std::string> exclude_dirs;       // Skip these dirs (e.g. "node_modules")
    std::vector<std::string> exclude_patterns;   // Glob patterns to skip

    // ── Content search (grepWin / AgentRansack) ───────────────────────
    bool search_content = false;    // If true, also grep file contents
    std::string content_query;      // Separate pattern for content
    MatchMode content_match_mode = MatchMode::Literal;
    bool content_case_sensitive = false;
    bool content_whole_word = false;
    int context_lines_before = 0;   // Lines of context before match
    int context_lines_after = 0;
    bool content_multiline = false; // . matches newlines
    bool content_dotall = false;    // Dotall mode
    int64_t max_file_size_content = 100 * 1024 * 1024; // Skip files > 100MB

    // ── File attribute filters ─────────────────────────────────────────
    SizeRangeFilter size_filter;
    DateRangeFilter date_filter;
    AttributeFilter attr_filter;

    // ── Result limits ──────────────────────────────────────────────────
    int max_results = 0;            // 0 = unlimited
    bool first_match_only = false;  // Stop at first match per file (faster)

    // ── Advanced ───────────────────────────────────────────────────────
    bool respect_gitignore = true;  // Like ripgrep/ag
    std::vector<std::string> ignore_files = {".gitignore", ".ignore", ".agignore"};
    int num_threads = 4;
};

/// A single content match within a file
struct ContentMatch {
    int64_t line_number;
    int64_t column;
    std::string line_text;        // The full line containing the match
    std::string match_text;       // The exact matched substring
    int64_t byte_offset;
    std::vector<std::string> context_before;
    std::vector<std::string> context_after;
};

/// A single file result from search
struct SearchResult {
    std::filesystem::path path;
    std::uintmax_t size = 0;
    std::chrono::system_clock::time_point modified;
    std::chrono::system_clock::time_point created;
    bool is_directory = false;
    std::vector<ContentMatch> matches; // Non-empty if content search was run
    int match_count = 0;              // Total matches in this file
};

/// Live progress during search
using SearchProgressCb = std::function<void(
    int files_scanned,
    int results_found,
    const std::filesystem::path& current_file
)>;

//─────────────────────────── Replace Options ────────────────────────────────//

/// Find-and-replace in file contents (grepWin / dnGrep feature)
struct ReplaceOptions {
    std::string find_pattern;
    std::string replace_with;
    MatchMode match_mode = MatchMode::Literal;
    bool case_sensitive = false;
    bool whole_word = false;
    bool backup_files = true;          // Create .bak before modifying
    std::string backup_extension = ".bak";
    bool dry_run = false;              // Preview only, no disk writes
    bool replace_all_in_file = true;   // false = replace only first match per file
    int encoding = 0;                  // 0=auto-detect, else codepage
};

/// Result of a file-content replace operation
struct ReplaceResult {
    std::filesystem::path path;
    int replacements_made = 0;
    bool success = false;
    std::string error;
    std::filesystem::path backup_path; // Set if backup was created
};

//──────────────────────────── Search Engine ─────────────────────────────────//

/// The main search engine combining filename and content search.
class SearchEngine {
public:
    /// Run a search synchronously. Results are returned sorted by relevance.
    /// progress_cb is called periodically during the search.
    std::vector<SearchResult> search(
        const SearchOptions& opts,
        SearchProgressCb progress_cb = nullptr);

    /// Async search — calls result_cb for each batch of results found.
    /// Returns immediately; call cancel() to stop.
    void search_async(
        const SearchOptions& opts,
        std::function<void(std::vector<SearchResult>)> result_cb,
        std::function<void()> done_cb = nullptr,
        SearchProgressCb progress_cb = nullptr);

    /// Stop an ongoing async search
    void cancel();
    bool is_running() const;

    /// Find-and-replace in file contents for a list of result files.
    std::vector<ReplaceResult> replace_in_files(
        const std::vector<SearchResult>& files,
        const ReplaceOptions& opts,
        std::function<void(int, int)> progress_cb = nullptr);

    /// Undo a replace operation (restores backups)
    bool undo_replace(const std::vector<ReplaceResult>& results);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::atomic<bool> cancelled_{false};

    // Core workers
    bool matches_filename(const std::filesystem::path& path, const SearchOptions& opts) const;
    bool matches_attributes(const std::filesystem::path& path, const SearchOptions& opts) const;
    std::vector<ContentMatch> search_content(const std::filesystem::path& path,
                                              const SearchOptions& opts) const;
    bool should_ignore(const std::filesystem::path& path, const SearchOptions& opts) const;
};

} // namespace fo::core
