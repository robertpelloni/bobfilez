#pragma once
/// @file batch_rename_interface.hpp
/// @brief Comprehensive batch file renaming engine for bobfilez.
///
/// Implements every feature found in:
///   - Bulk Rename Utility (BRU)        — the de-facto standard
///   - Advanced Renamer                 — preset/macro-based
///   - ReNamer                          — rule chain model
///   - PowerRename (Windows 11 PowerToys) — regex + preview
///   - F2 (CLI renamer, Go)             — fast scripted renames
///
/// Rename operations are chained — each rule transforms the name in order.
/// A preview is always generated before any files are touched.
///
/// Supported operations (exhaustive):
///   Replace:     Simple or regex substring replace (case options, whole-word)
///   Insert:      Insert text at position (from start/end/before/after pattern)
///   Delete:      Remove N chars from position
///   Trim:        Strip leading/trailing/both whitespace or custom chars
///   Case:        UPPER/lower/Title Case/Sentence case/tOGGLE/CamelCase/snake_case
///   Number:      Sequential numbering with start, step, padding, position
///   Date/Time:   Insert file date metadata (created/modified) with strftime format
///   EXIF/Meta:   Insert EXIF tag values (artist, camera, date, GPS, etc.)
///   Extension:   Change, add, remove, keep original
///   RegexMatch:  Select files by regex (filter)
///   Groups:      Named capture group → name component  ($1 $2 etc.)
///   Padding:     Pad name to length with char
///   Truncate:    Limit name to max length
///   Sanitize:    Replace illegal filename chars
///   UUID:        Replace name with UUID v4
///   Hash:        Replace name with file content hash (md5/sha1/sha256/xxhash)
///   Counter:     Per-folder counter (restarts for each folder)
///   Move Part:   Move part of name (e.g. move "2024_" prefix to suffix)
///   Transliterate: Unicode → ASCII (e.g. café → cafe)

#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <functional>
#include <optional>
#include <map>
#include <chrono>

namespace fo::core {

/// A single rename preview entry.
struct RenamePreview {
    std::filesystem::path original_path;
    std::filesystem::path new_path;
    bool conflict = false;     // Would overwrite an existing file
    bool duplicate = false;    // Two files in batch would collide with each other
    std::string error;         // Non-empty if this rename can't be performed
};

/// Result of a committed rename
struct RenameResult {
    std::filesystem::path original_path;
    std::filesystem::path new_path;
    bool success = false;
    std::string error;
};

/// Base class for all rename rules.
/// Rules are applied in sequence to produce the new filename (stem only by default).
class IRenameRule {
public:
    virtual ~IRenameRule() = default;
    virtual std::string rule_type() const = 0;
    /// Transform name stem. Receives current stem, index in batch, full path, and metadata.
    virtual std::string apply(const std::string& current_stem,
                               int index,
                               const std::filesystem::path& original_path,
                               const std::map<std::string, std::string>& metadata) const = 0;
    virtual std::string description() const = 0;
    virtual bool enabled = true;
};

//─────────────────────────── Concrete Rule Types ───────────────────────────//

/// Replace: find→replace with optional regex and case options
struct ReplaceRule : IRenameRule {
    std::string find;
    std::string replace;
    bool use_regex = false;
    bool case_sensitive = true;
    bool whole_word = false;     // \b word boundary match
    bool replace_all = true;     // false = replace first occurrence only
    int replace_nth = 0;         // 0 = all, N = replace Nth occurrence only

    std::string rule_type() const override { return "replace"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

/// Insert: insert text at absolute position or relative to pattern
struct InsertRule : IRenameRule {
    std::string text;
    enum class Position { Start, End, AtIndex, BeforePattern, AfterPattern } position = Position::Start;
    int index = 0;               // For AtIndex
    std::string pattern;         // For BeforePattern / AfterPattern
    bool count_from_end = false; // Negative index support

    std::string rule_type() const override { return "insert"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

/// Delete: remove N characters from position
struct DeleteRule : IRenameRule {
    int start_index = 0;
    int count = 1;
    bool count_from_end = false;

    std::string rule_type() const override { return "delete"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

/// Trim: strip chars from start/end
struct TrimRule : IRenameRule {
    bool trim_start = true;
    bool trim_end = true;
    std::string chars = " \t_-"; // Chars to strip (empty = whitespace only)

    std::string rule_type() const override { return "trim"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

/// Case transform
struct CaseRule : IRenameRule {
    enum class Mode {
        Upper, Lower, TitleCase, SentenceCase, Toggle,
        CamelCase, SnakeCase, KebabCase, PascalCase
    } mode = Mode::TitleCase;

    std::string rule_type() const override { return "case"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

/// Sequential numbering
struct NumberRule : IRenameRule {
    enum class Position { Start, End, AtIndex, Replace } position = Position::End;
    int start = 1;
    int step = 1;
    int pad_width = 3;         // Zero-pad to this many digits
    char pad_char = '0';
    std::string prefix;        // e.g. "(" before number
    std::string suffix;        // e.g. ")" after number
    int index_pos = 0;         // For AtIndex
    bool per_folder = false;   // Reset counter per folder

    std::string rule_type() const override { return "number"; }
    std::string description() const override;
    std::string apply(const std::string& s, int idx, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

/// Date/time insertion using file timestamps or current time
struct DateTimeRule : IRenameRule {
    enum class Source { FileModified, FileCreated, CurrentTime, ExifDate } source = Source::FileModified;
    std::string format = "%Y-%m-%d"; // strftime format
    enum class Position { Start, End, Replace } position = Position::Start;
    std::string separator = "_";

    std::string rule_type() const override { return "datetime"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

/// EXIF/metadata field insertion
struct MetadataRule : IRenameRule {
    std::string field;           // e.g. "camera_make", "camera_model", "gps_city", "artist"
    enum class Position { Start, End, Replace } position = Position::End;
    std::string separator = "_";
    std::string fallback;        // Used if field not available

    std::string rule_type() const override { return "metadata"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path& p,
                      const std::map<std::string, std::string>& meta) const override;
};

/// Extension rule: operate on the extension part
struct ExtensionRule : IRenameRule {
    enum class Mode { Change, Remove, AddIfMissing, LowerCase, UpperCase } mode = Mode::Change;
    std::string new_ext; // For Change / AddIfMissing

    std::string rule_type() const override { return "extension"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

/// Truncate name to max length (respects word boundaries optionally)
struct TruncateRule : IRenameRule {
    int max_length = 64;
    bool at_word_boundary = true;
    std::string ellipsis; // appended after truncation (e.g. "…")

    std::string rule_type() const override { return "truncate"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

/// Sanitize: replace/remove chars illegal in filenames
struct SanitizeRule : IRenameRule {
    std::string replacement = "_"; // Replace illegal chars with this
    bool target_windows = true;    // Windows-illegal chars: \ / : * ? " < > |
    bool target_linux = false;     // Linux-illegal: / and null only
    bool collapse_replacement = true; // Collapse multiple consecutive replacements to one

    std::string rule_type() const override { return "sanitize"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

/// Hash: replace name with content hash
struct HashNameRule : IRenameRule {
    std::string algorithm = "xxhash"; // md5, sha1, sha256, xxhash
    bool preserve_extension = true;

    std::string rule_type() const override { return "hash"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path& p,
                      const std::map<std::string, std::string>&) const override;
};

/// Transliterate: Unicode → ASCII (e.g. ñ→n, ü→u, café→cafe)
struct TransliterateRule : IRenameRule {
    bool remove_unhandled = false; // Remove chars that can't be transliterated

    std::string rule_type() const override { return "transliterate"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

/// Regex groups: use capture groups from first regex match as name components
struct RegexGroupsRule : IRenameRule {
    std::string pattern;         // e.g. "(\d{4})-(\d{2})-(\d{2})"
    std::string output_template; // e.g. "$3$2$1" or "$1_$2"

    std::string rule_type() const override { return "regex_groups"; }
    std::string description() const override;
    std::string apply(const std::string& s, int, const std::filesystem::path&,
                      const std::map<std::string, std::string>&) const override;
};

//─────────────────────────── Batch Rename Engine ───────────────────────────//

/// A preset is a named, saveable chain of rules
struct RenamePreset {
    std::string name;
    std::string description;
    std::vector<std::shared_ptr<IRenameRule>> rules;
};

/// The main batch rename engine
class BatchRenameEngine {
public:
    /// Apply a rule chain to one filename. Returns the new stem.
    static std::string apply_rules(const std::string& original_stem,
                                    const std::vector<std::shared_ptr<IRenameRule>>& rules,
                                    int index,
                                    const std::filesystem::path& path,
                                    const std::map<std::string, std::string>& metadata = {});

    /// Preview renames for a list of files with a given rule chain.
    /// extension_rule: optional separate extension override.
    static std::vector<RenamePreview> preview(
        const std::vector<std::filesystem::path>& files,
        const std::vector<std::shared_ptr<IRenameRule>>& rules,
        bool include_extension = false,
        int start_index = 0);

    /// Execute renames. Returns undo-able list.
    /// Checks for conflicts before touching anything.
    static std::vector<RenameResult> execute(
        const std::vector<RenamePreview>& previews,
        bool skip_conflicts = true,
        bool dry_run = false);

    /// Save/load presets from JSON
    static void save_preset(const RenamePreset& preset, const std::filesystem::path& path);
    static RenamePreset load_preset(const std::filesystem::path& path);

    /// Extract file metadata map for use by MetadataRule / DateTimeRule
    static std::map<std::string, std::string> extract_metadata(const std::filesystem::path& file);

    /// Built-in preset library (TV show renamer, photo date organizer, etc.)
    static std::vector<RenamePreset> builtin_presets();
};

} // namespace fo::core
