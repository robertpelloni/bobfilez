#pragma once
/// @file file_watcher_interface.hpp
/// @brief Real-time file system watcher for bobfilez (the "Shadow Sorter").
///
/// Monitors directories for file changes using OS-native APIs:
///   - Windows: ReadDirectoryChangesW
///   - Linux: inotify
///   - macOS: FSEvents / kqueue
///
/// When a new file arrives in a watched directory (e.g. ~/Downloads), the watcher
/// emits an event that the RuleEngine can consume to auto-organize in real time.
///
/// Design goals:
///   - Zero-polling: pure event-driven via kernel notifications
///   - Recursive subdirectory monitoring
///   - Configurable debounce to batch rapid-fire events (e.g. unzipping)
///   - Thread-safe callback dispatch

#include <filesystem>
#include <functional>
#include <string>
#include <vector>
#include <chrono>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace fo::core {

/// Types of filesystem events the watcher can detect
enum class FileEvent {
    Created,     ///< New file or directory created
    Modified,    ///< Existing file contents changed
    Deleted,     ///< File or directory removed
    Renamed,     ///< File or directory renamed (old path in `old_path`)
    Moved        ///< File moved in/out of watched directory
};

/// A single filesystem change notification
struct FileChangeEvent {
    FileEvent type = FileEvent::Created;
    std::filesystem::path path;          ///< Current path of the affected file
    std::filesystem::path old_path;      ///< Previous path (for Renamed events)
    std::chrono::system_clock::time_point timestamp;
    bool is_directory = false;
};

/// Configuration for the directory watcher
struct WatcherConfig {
    std::vector<std::filesystem::path> watch_paths;  ///< Directories to monitor
    bool recursive = true;                           ///< Watch subdirectories
    int debounce_ms = 500;                           ///< Debounce delay in milliseconds
    bool ignore_hidden = true;                       ///< Skip dotfiles/hidden files
    std::vector<std::string> include_extensions;     ///< Only watch these (empty = all)
    std::vector<std::string> exclude_patterns;       ///< Glob patterns to exclude
};

/// Callback signature: receives a batch of events after debounce
using FileWatchCallback = std::function<void(const std::vector<FileChangeEvent>&)>;

/// Abstract interface for OS-native file watching
class IFileWatcher {
public:
    virtual ~IFileWatcher() = default;

    /// Start monitoring with the given configuration.
    /// The callback is invoked on a background thread whenever events are detected.
    virtual bool start(const WatcherConfig& config, FileWatchCallback callback) = 0;

    /// Stop monitoring and join the background thread
    virtual void stop() = 0;

    /// Add a new path to the watch set without restarting
    virtual bool add_watch(const std::filesystem::path& path) = 0;

    /// Remove a path from the watch set
    virtual bool remove_watch(const std::filesystem::path& path) = 0;

    /// True if the watcher is actively monitoring
    virtual bool is_running() const = 0;

    /// Returns the number of events processed since start
    virtual uint64_t events_processed() const = 0;
};

} // namespace fo::core
