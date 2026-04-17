#pragma once
/// @file fileops_interface.hpp
/// @brief Comprehensive batch file operations engine for bobfilez.
///
/// Covers the full spectrum of file management utilities found in:
///   FreeFileSync  — sync (mirror/two-way/update), versioning, real-time watch
///   TeraCopy      — copy/move with verification, queue, error recovery
///   Beyond Compare — file/folder diff with merge
///   WinMerge      — 3-way diff, folder compare, hex compare
///   Robocopy      — enterprise-grade copy with retry/throttle/mirror
///   rsync/librsync — delta-based sync (skip unchanged chunks)
///   7-Zip         — archive creation/extraction with all formats
///   Areca Backup  — incremental/differential/full backup with versioning
///   Duplicati     — encrypted cloud-aware backup
///   rclone        — cloud sync (S3/GDrive/Azure/Dropbox/etc.)
///
/// All operations are:
///   - Previewable (dry-run / plan generation before execution)
///   - Resumable   (operations write a journal to survive crashes)
///   - Undoable    (for move/delete; copy/archive are non-destructive)
///   - Parallelisable (configurable thread pool)
///   - Progress-reported (per-file and aggregate)

#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <optional>
#include <chrono>
#include <cstdint>
#include <atomic>
#include <map>

namespace fo::core {

//───────────────────────────── Shared types ──────────────────────────────────

/// How to handle filename collisions during copy/move
enum class CollisionPolicy {
    Ask,            // Pause and ask (GUI mode)
    Skip,           // Leave destination unchanged
    Overwrite,      // Replace destination unconditionally
    OverwriteOlder, // Replace only if source is newer
    OverwriteSmaller,
    Rename,         // Auto-rename: file (2).txt, file (3).txt ...
    RenameNew,      // Rename the incoming file, keep old
    Merge,          // For directories: merge contents
    Abort           // Stop the entire operation
};

/// File operation types tracked for undo / journaling
enum class FileOpType { Copy, Move, Delete, CreateDir, Rename, HardLink, Symlink };

/// Per-file operation plan entry (used in previews)
struct FileOpPlanEntry {
    FileOpType type;
    std::filesystem::path source;
    std::filesystem::path dest;   // Empty for Delete
    std::uintmax_t size = 0;
    bool would_overwrite = false;
    bool would_skip = false;
    std::string collision_note;
};

/// Live progress for a running operation
struct FileOpProgress {
    int files_done = 0;
    int files_total = 0;
    int64_t bytes_done = 0;
    int64_t bytes_total = 0;
    double speed_bytes_sec = 0.0;
    double eta_sec = 0.0;
    std::filesystem::path current_file;
    std::string phase; // "Scanning", "Copying", "Verifying", "Done"
};

/// Result of a single file operation
struct FileOpResult {
    std::filesystem::path source;
    std::filesystem::path dest;
    FileOpType type;
    bool success = false;
    std::string error;
    double duration_sec = 0.0;
    int64_t bytes_transferred = 0;
    std::string checksum_before; // For verify mode
    std::string checksum_after;
    bool checksum_match = true;
};

/// Summary of a completed batch operation
struct FileOpSummary {
    int total = 0, succeeded = 0, skipped = 0, failed = 0;
    int64_t bytes_transferred = 0;
    double duration_sec = 0.0;
    std::vector<FileOpResult> results;
    std::vector<std::string> errors;
};

using FileOpProgressCb = std::function<void(const FileOpProgress&)>;
using FileOpResultCb   = std::function<void(const FileOpResult&)>;

//────────────────────────── Copy / Move Options ──────────────────────────────

struct CopyMoveOptions {
    CollisionPolicy collision = CollisionPolicy::Ask;
    bool move_mode = false;          // true = move (delete source after verify)
    bool verify_checksums = false;   // Compute + compare hash after copy
    std::string verify_hash = "xxhash"; // "xxhash","md5","sha256"
    bool preserve_timestamps = true;
    bool preserve_attributes = true;
    bool preserve_permissions = true;
    bool follow_symlinks = false;
    bool create_symlinks = false;    // Create symlinks instead of copying
    bool create_hardlinks = false;   // Create hardlinks where possible
    bool recursive = true;
    int  retry_count = 3;            // Retry on transient errors
    int  retry_delay_ms = 1000;
    int  num_threads = 4;
    int64_t throttle_bytes_sec = 0;  // 0 = no throttle (like Robocopy /IPG)
    bool dry_run = false;
    std::vector<std::string> include_extensions;
    std::vector<std::string> exclude_extensions;
    std::vector<std::string> exclude_patterns; // Glob patterns to skip
    std::uintmax_t min_size = 0;
    std::uintmax_t max_size = 0;     // 0 = no limit
    bool skip_hidden = false;
    bool skip_system = false;
    // Buffering
    size_t buffer_size = 4 * 1024 * 1024; // 4 MiB copy buffer
    // Flat copy: don't recreate directory structure
    bool flat_copy = false;
    // Delete extra files in dest not in source (mirror mode helper)
    bool delete_orphans = false;
};

//────────────────────────── Sync Options ─────────────────────────────────────
/// FreeFileSync-style synchronization options

enum class SyncMode {
    Mirror,      // Dest becomes exact copy of source (delete extras in dest)
    Update,      // Copy newer/missing from source → dest only
    TwoWay,      // Bidirectional: newest wins, both sides updated
    Custom       // Per-category actions configured manually
};

struct SyncCategoryAction {
    // What to do with files that are only in left / only in right / newer left / newer right / equal
    enum class Action { Copy, Delete, Skip, Ask } left_only, right_only, left_newer, right_newer, equal;
};

struct SyncOptions {
    SyncMode mode = SyncMode::Mirror;
    SyncCategoryAction custom_actions; // Used when mode == Custom
    CopyMoveOptions copy_opts;         // Inherits all copy settings
    bool use_versioning = false;       // Keep old versions of overwritten files
    std::filesystem::path versioning_dir;
    enum class VersioningStyle { Replace, TimeStamp, Recycle } versioning_style = VersioningStyle::TimeStamp;
    int versioning_max_versions = 10;
    bool detect_moved_files = true;    // Detect rename/move instead of del+add
    bool use_database = true;          // FFS-style sync db for tracking changes
    std::filesystem::path sync_db_path;
    bool realtime_watch = false;       // Watch for FS changes and auto-sync
    // Compare method
    enum class CompareMethod { FileSize, FileSizeAndDate, FileContent } compare_by = CompareMethod::FileSizeAndDate;
    int time_tolerance_sec = 2;        // FAT32 has 2-sec timestamp resolution
};

//────────────────────────── Diff Options ─────────────────────────────────────

enum class DiffAlgorithm {
    Myers,       // Classic O(ND) Myers diff (git default)
    Patience,    // Patience diff (better for moved blocks, used by git -P)
    Histogram,   // Histogram diff (git default since 2.7, best for code)
    Minimal      // Minimize diff size (slowest)
};

struct DiffOptions {
    DiffAlgorithm algorithm = DiffAlgorithm::Histogram;
    bool ignore_whitespace = false;
    bool ignore_blank_lines = false;
    bool ignore_case = false;
    int context_lines = 3;
    bool binary_compare = false;     // Hex diff for binary files
    bool side_by_side = false;       // Unified vs side-by-side output
    bool recursive = true;           // For folder diff
    bool show_identical = false;     // Include identical files in folder diff
    std::vector<std::string> exclude_patterns;
    enum class OutputFormat { Unified, SideBySide, HTML, JSON } output_format = OutputFormat::Unified;
};

struct DiffHunk {
    int left_start, left_count;
    int right_start, right_count;
    enum class Type { Equal, Insert, Delete, Replace } type;
    std::vector<std::string> left_lines;
    std::vector<std::string> right_lines;
};

struct FileDiffResult {
    std::filesystem::path left_path;
    std::filesystem::path right_path;
    bool binary = false;
    bool identical = false;
    std::vector<DiffHunk> hunks;
    int added_lines = 0;
    int removed_lines = 0;
    std::string unified_diff;         // Full unified diff text
    std::string html_diff;            // Side-by-side HTML representation
};

struct FolderDiffResult {
    std::filesystem::path left_root;
    std::filesystem::path right_root;
    struct Entry {
        std::filesystem::path rel_path;
        enum class Status { LeftOnly, RightOnly, Different, Identical } status;
        std::optional<FileDiffResult> file_diff; // Non-null for Different
    };
    std::vector<Entry> entries;
};

//────────────────────────── Backup Options ───────────────────────────────────

enum class BackupType {
    Full,          // Copy everything
    Incremental,   // Copy files changed since last backup
    Differential,  // Copy files changed since last FULL backup
    Mirror         // FreeFileSync-style mirror with versioning
};

struct BackupOptions {
    BackupType type = BackupType::Full;
    std::filesystem::path source;
    std::filesystem::path dest;
    bool compress = true;
    enum class CompressAlgo { Zip, Zstd, Lz4, Brotli, SevenZip, Tar_Gz, Tar_Xz } compress_algo = CompressAlgo::Zstd;
    int compress_level = 6;           // Algo-specific (zstd: 1-22, zip: 0-9)
    bool encrypt = false;
    std::string encryption_password;
    enum class EncryptAlgo { AES256, ChaCha20 } encrypt_algo = EncryptAlgo::AES256;
    bool split_volumes = false;
    int64_t volume_size_bytes = 0;    // 0 = no split
    int max_versions = 10;            // Keep this many backup sets
    bool use_delta = false;           // rsync-style delta compression (only changed chunks)
    std::filesystem::path backup_db;  // Tracks backup history
    bool verify_after = true;         // Checksum verify after backup
    CopyMoveOptions copy_opts;
    std::vector<std::string> exclude_patterns;
};

struct BackupResult {
    BackupType type;
    std::chrono::system_clock::time_point timestamp;
    std::filesystem::path backup_path; // Where the backup was written
    int64_t bytes_backed_up = 0;
    int64_t bytes_compressed = 0;
    double compression_ratio = 0.0;
    int files_backed_up = 0;
    bool verified = false;
    bool success = false;
    std::vector<std::string> errors;
};

//────────────────────────── Archive Options ──────────────────────────────────

enum class ArchiveFormat { Zip, SevenZip, Tar, Tar_Gz, Tar_Bz2, Tar_Xz, Tar_Zstd, Tar_Lz4, Rar /* read-only */ };

struct ArchiveCreateOptions {
    ArchiveFormat format = ArchiveFormat::SevenZip;
    int compression_level = 5;        // 0=store, 9=ultra
    std::string compression_method;   // "LZMA2","Deflate","Brotli" etc.
    bool encrypt_contents = false;
    bool encrypt_filenames = false;   // 7z only
    std::string password;
    bool solid_mode = true;           // 7z solid (better ratio, slower update)
    bool include_root_dir = true;     // Include parent folder in archive
    bool self_extracting = false;     // Create SFX executable
    bool split_volumes = false;
    int64_t volume_size_bytes = 0;
    int num_threads = 0;              // 0 = auto
    std::vector<std::string> exclude_patterns;
};

struct ArchiveExtractOptions {
    std::filesystem::path dest_dir;
    bool overwrite = false;
    std::string password;
    std::vector<std::string> include_patterns; // Extract only matching entries
    bool keep_structure = true;       // Preserve directory structure
    bool verify_after = false;
};

//────────────────────────── Engines ──────────────────────────────────────────

/// Batch copy/move engine (TeraCopy-style)
class FileCopyMoveEngine {
public:
    /// Build a plan without executing (dry-run preview)
    std::vector<FileOpPlanEntry> plan(
        const std::vector<std::filesystem::path>& sources,
        const std::filesystem::path& dest_root,
        const CopyMoveOptions& opts);

    /// Execute copy or move
    FileOpSummary execute(
        const std::vector<FileOpPlanEntry>& plan,
        const CopyMoveOptions& opts,
        FileOpProgressCb progress_cb = nullptr,
        FileOpResultCb   result_cb   = nullptr);

    /// Convenience: plan + execute
    FileOpSummary run(
        const std::vector<std::filesystem::path>& sources,
        const std::filesystem::path& dest_root,
        const CopyMoveOptions& opts,
        FileOpProgressCb progress_cb = nullptr);

    void cancel() { cancelled_ = true; }
    bool is_cancelled() const { return cancelled_; }

private:
    std::atomic<bool> cancelled_{false};
    FileOpResult copy_single(const std::filesystem::path& src,
                              const std::filesystem::path& dst,
                              const CopyMoveOptions& opts);
};

/// Folder synchronization engine (FreeFileSync-style)
class FolderSyncEngine {
public:
    struct SyncPlan {
        struct Entry {
            std::filesystem::path rel_path;
            enum class Action { CopyLR, CopyRL, DeleteL, DeleteR, Skip, Conflict } action;
            std::filesystem::path left_abs, right_abs;
            int64_t left_size = 0, right_size = 0;
            std::chrono::system_clock::time_point left_mtime, right_mtime;
        };
        std::vector<Entry> entries;
        int to_copy_lr = 0, to_copy_rl = 0, to_delete_l = 0, to_delete_r = 0;
        int64_t bytes_to_transfer = 0;
    };

    SyncPlan build_plan(
        const std::filesystem::path& left,
        const std::filesystem::path& right,
        const SyncOptions& opts);

    FileOpSummary execute(
        const SyncPlan& plan,
        const SyncOptions& opts,
        FileOpProgressCb progress_cb = nullptr);

    FileOpSummary sync(
        const std::filesystem::path& left,
        const std::filesystem::path& right,
        const SyncOptions& opts,
        FileOpProgressCb progress_cb = nullptr);
};

/// File/folder diff engine (WinMerge/Beyond Compare-style)
class FileDiffEngine {
public:
    FileDiffResult diff_files(
        const std::filesystem::path& left,
        const std::filesystem::path& right,
        const DiffOptions& opts);

    FolderDiffResult diff_folders(
        const std::filesystem::path& left,
        const std::filesystem::path& right,
        const DiffOptions& opts);

    /// Generate side-by-side HTML diff for display in a WebView
    static std::string render_html_diff(const FileDiffResult& result);
};

/// Backup engine (Areca/Duplicati-style)
class BackupEngine {
public:
    BackupResult run_backup(const BackupOptions& opts,
                             FileOpProgressCb progress_cb = nullptr);
    bool restore(const std::filesystem::path& backup_path,
                 const ArchiveExtractOptions& opts,
                 FileOpProgressCb progress_cb = nullptr);
    std::vector<BackupResult> list_backups(const std::filesystem::path& backup_dir);
    bool delete_backup(const BackupResult& bak);
};

/// Archive engine (7-Zip-style)
class ArchiveEngine {
public:
    /// Create an archive from a list of paths
    bool create(const std::vector<std::filesystem::path>& sources,
                const std::filesystem::path& output_archive,
                const ArchiveCreateOptions& opts,
                FileOpProgressCb progress_cb = nullptr);

    /// Extract an archive
    bool extract(const std::filesystem::path& archive,
                 const ArchiveExtractOptions& opts,
                 FileOpProgressCb progress_cb = nullptr);

    /// List archive contents without extracting
    struct ArchiveEntry {
        std::string path;
        int64_t size = 0;
        int64_t compressed_size = 0;
        bool is_dir = false;
        std::chrono::system_clock::time_point modified;
    };
    std::vector<ArchiveEntry> list(const std::filesystem::path& archive);

    /// Test archive integrity
    bool verify(const std::filesystem::path& archive, const std::string& password = "");

    /// All formats this engine can read
    static std::vector<std::string> supported_read_extensions();
    /// All formats this engine can write
    static std::vector<std::string> supported_write_extensions();
};

} // namespace fo::core
