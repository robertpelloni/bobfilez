/// @file file_watcher.cpp
/// @brief Native file watcher implementation using ReadDirectoryChangesW (Windows) / inotify (Linux).
///
/// The "Shadow Sorter" — monitors directories in real time and fires callbacks
/// whenever files are created, modified, deleted, or renamed. Runs on a dedicated
/// background thread with configurable debounce to coalesce rapid events.
///
/// On Windows, we use the overlapped I/O version of ReadDirectoryChangesW for
/// non-blocking waits with cancellation support. On Linux, we use inotify with
/// epoll for efficient event multiplexing.

#include "fo/core/file_watcher_interface.hpp"
#include "fo/core/registry.hpp"
#include <iostream>
#include <algorithm>
#include <queue>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <sys/inotify.h>
#include <unistd.h>
#include <poll.h>
#include <limits.h>
#endif

namespace fo::core {

class NativeFileWatcher : public IFileWatcher {
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> event_count_{0};
    std::thread worker_thread_;
    FileWatchCallback callback_;
    WatcherConfig config_;
    std::mutex mutex_;

    /// Check if a path should be ignored based on config filters
    bool should_ignore(const std::filesystem::path& p) const {
        // Hidden files
        if (config_.ignore_hidden) {
            auto fname = p.filename().string();
            if (!fname.empty() && fname[0] == '.') return true;
        }
        // Extension filter
        if (!config_.include_extensions.empty()) {
            auto ext = p.extension().string();
            if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            bool found = false;
            for (auto& ie : config_.include_extensions) {
                std::string low = ie;
                std::transform(low.begin(), low.end(), low.begin(), ::tolower);
                if (low == ext) { found = true; break; }
            }
            if (!found) return true;
        }
        // Exclude patterns (simple glob against filename)
        auto fname = p.filename().string();
        for (auto& pat : config_.exclude_patterns) {
            // Basic wildcard matching
            if (fname.find(pat) != std::string::npos) return true;
        }
        return false;
    }

#ifdef _WIN32
    // ─── Windows Implementation: ReadDirectoryChangesW ───────────────────────
    HANDLE stop_event_ = nullptr;

    void watch_loop() {
        // Create a stop event for clean shutdown
        stop_event_ = CreateEventA(nullptr, TRUE, FALSE, nullptr);

        for (auto& watch_path : config_.watch_paths) {
            if (!std::filesystem::is_directory(watch_path)) {
                std::cerr << "[FileWatcher] Not a directory: " << watch_path << "\n";
                continue;
            }

            HANDLE dir_handle = CreateFileW(
                watch_path.wstring().c_str(),
                FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                nullptr,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
                nullptr
            );

            if (dir_handle == INVALID_HANDLE_VALUE) {
                std::cerr << "[FileWatcher] Failed to open directory: " << watch_path
                          << " (error " << GetLastError() << ")\n";
                continue;
            }

            // 64KB buffer for change notifications
            constexpr DWORD BUF_SIZE = 64 * 1024;
            auto buffer = std::make_unique<BYTE[]>(BUF_SIZE);
            OVERLAPPED overlapped = {};
            overlapped.hEvent = CreateEventA(nullptr, TRUE, FALSE, nullptr);

            DWORD filter = FILE_NOTIFY_CHANGE_FILE_NAME
                         | FILE_NOTIFY_CHANGE_DIR_NAME
                         | FILE_NOTIFY_CHANGE_SIZE
                         | FILE_NOTIFY_CHANGE_LAST_WRITE
                         | FILE_NOTIFY_CHANGE_CREATION;

            // Main event loop
            while (running_.load()) {
                DWORD bytes_returned = 0;
                ResetEvent(overlapped.hEvent);

                BOOL ok = ReadDirectoryChangesW(
                    dir_handle,
                    buffer.get(),
                    BUF_SIZE,
                    config_.recursive ? TRUE : FALSE,
                    filter,
                    &bytes_returned,
                    &overlapped,
                    nullptr
                );

                if (!ok) {
                    std::cerr << "[FileWatcher] ReadDirectoryChangesW failed: " << GetLastError() << "\n";
                    break;
                }

                // Wait for either: directory change or stop signal
                HANDLE handles[] = { overlapped.hEvent, stop_event_ };
                DWORD wait_result = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

                if (wait_result == WAIT_OBJECT_0 + 1) {
                    // Stop requested
                    CancelIo(dir_handle);
                    break;
                }

                if (wait_result != WAIT_OBJECT_0) continue;

                // Get the actual result
                if (!GetOverlappedResult(dir_handle, &overlapped, &bytes_returned, FALSE))
                    continue;
                if (bytes_returned == 0) continue;

                // Parse the FILE_NOTIFY_INFORMATION chain
                std::vector<FileChangeEvent> pending_events;
                auto now = std::chrono::system_clock::now();

                BYTE* ptr = buffer.get();
                while (true) {
                    auto* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(ptr);
                    std::wstring wname(info->FileName, info->FileNameLength / sizeof(WCHAR));
                    std::filesystem::path full_path = watch_path / wname;

                    if (!should_ignore(full_path)) {
                        FileChangeEvent evt;
                        evt.path = full_path;
                        evt.timestamp = now;
                        evt.is_directory = std::filesystem::is_directory(full_path);

                        switch (info->Action) {
                            case FILE_ACTION_ADDED:            evt.type = FileEvent::Created;  break;
                            case FILE_ACTION_REMOVED:          evt.type = FileEvent::Deleted;  break;
                            case FILE_ACTION_MODIFIED:         evt.type = FileEvent::Modified; break;
                            case FILE_ACTION_RENAMED_OLD_NAME: evt.type = FileEvent::Renamed; evt.old_path = full_path; break;
                            case FILE_ACTION_RENAMED_NEW_NAME: evt.type = FileEvent::Renamed;  break;
                            default: break;
                        }
                        pending_events.push_back(std::move(evt));
                        event_count_.fetch_add(1);
                    }

                    if (info->NextEntryOffset == 0) break;
                    ptr += info->NextEntryOffset;
                }

                // Debounce: sleep briefly then fire callback with batched events
                if (!pending_events.empty()) {
                    if (config_.debounce_ms > 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(config_.debounce_ms));
                    }
                    if (callback_) callback_(pending_events);
                }
            }

            CloseHandle(overlapped.hEvent);
            CloseHandle(dir_handle);
        }

        if (stop_event_) {
            CloseHandle(stop_event_);
            stop_event_ = nullptr;
        }
    }

    void signal_stop() {
        if (stop_event_) SetEvent(stop_event_);
    }

#else
    // ─── Linux Implementation: inotify + poll ────────────────────────────────
    int inotify_fd_ = -1;
    int stop_pipe_[2] = {-1, -1};

    void watch_loop() {
        inotify_fd_ = inotify_init1(IN_NONBLOCK);
        if (inotify_fd_ < 0) {
            std::cerr << "[FileWatcher] inotify_init1 failed\n";
            return;
        }

        // Self-pipe trick for clean shutdown
        if (pipe(stop_pipe_) < 0) {
            std::cerr << "[FileWatcher] pipe() failed\n";
            close(inotify_fd_);
            return;
        }

        uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_CLOSE_WRITE;

        // Add watches for all configured paths
        std::unordered_map<int, std::filesystem::path> wd_to_path;
        for (auto& wp : config_.watch_paths) {
            int wd = inotify_add_watch(inotify_fd_, wp.string().c_str(), mask);
            if (wd >= 0) {
                wd_to_path[wd] = wp;
            } else {
                std::cerr << "[FileWatcher] Failed to watch: " << wp << "\n";
            }

            // If recursive, add all subdirectories
            if (config_.recursive) {
                std::error_code ec;
                for (auto& entry : std::filesystem::recursive_directory_iterator(wp, ec)) {
                    if (entry.is_directory()) {
                        int swd = inotify_add_watch(inotify_fd_, entry.path().string().c_str(), mask);
                        if (swd >= 0) wd_to_path[swd] = entry.path();
                    }
                }
            }
        }

        // Event buffer
        constexpr size_t BUF_LEN = 4096 * (sizeof(struct inotify_event) + NAME_MAX + 1);
        auto buffer = std::make_unique<char[]>(BUF_LEN);

        struct pollfd fds[2];
        fds[0].fd = inotify_fd_;
        fds[0].events = POLLIN;
        fds[1].fd = stop_pipe_[0];
        fds[1].events = POLLIN;

        while (running_.load()) {
            int ret = poll(fds, 2, -1);
            if (ret < 0) break;

            // Check for stop signal
            if (fds[1].revents & POLLIN) break;

            if (!(fds[0].revents & POLLIN)) continue;

            ssize_t len = read(inotify_fd_, buffer.get(), BUF_LEN);
            if (len <= 0) continue;

            std::vector<FileChangeEvent> pending_events;
            auto now = std::chrono::system_clock::now();

            char* ptr = buffer.get();
            while (ptr < buffer.get() + len) {
                auto* event = reinterpret_cast<struct inotify_event*>(ptr);

                if (event->len > 0) {
                    std::string name(event->name);
                    auto it = wd_to_path.find(event->wd);
                    std::filesystem::path parent = (it != wd_to_path.end()) ? it->second : ".";
                    std::filesystem::path full_path = parent / name;

                    if (!should_ignore(full_path)) {
                        FileChangeEvent evt;
                        evt.path = full_path;
                        evt.timestamp = now;
                        evt.is_directory = (event->mask & IN_ISDIR) != 0;

                        if (event->mask & IN_CREATE)      evt.type = FileEvent::Created;
                        else if (event->mask & IN_DELETE)  evt.type = FileEvent::Deleted;
                        else if (event->mask & IN_MODIFY)  evt.type = FileEvent::Modified;
                        else if (event->mask & IN_CLOSE_WRITE) evt.type = FileEvent::Modified;
                        else if (event->mask & IN_MOVED_FROM) { evt.type = FileEvent::Moved; evt.old_path = full_path; }
                        else if (event->mask & IN_MOVED_TO) evt.type = FileEvent::Moved;

                        pending_events.push_back(std::move(evt));
                        event_count_.fetch_add(1);

                        // Auto-add new subdirectories
                        if (evt.is_directory && (event->mask & IN_CREATE) && config_.recursive) {
                            int swd = inotify_add_watch(inotify_fd_, full_path.string().c_str(), mask);
                            if (swd >= 0) wd_to_path[swd] = full_path;
                        }
                    }
                }

                ptr += sizeof(struct inotify_event) + event->len;
            }

            // Debounce + fire
            if (!pending_events.empty()) {
                if (config_.debounce_ms > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(config_.debounce_ms));
                if (callback_) callback_(pending_events);
            }
        }

        // Cleanup
        for (auto& [wd, _] : wd_to_path) {
            inotify_rm_watch(inotify_fd_, wd);
        }
        close(inotify_fd_);
        close(stop_pipe_[0]);
        close(stop_pipe_[1]);
    }

    void signal_stop() {
        char c = 1;
        write(stop_pipe_[1], &c, 1);
    }
#endif

public:
    ~NativeFileWatcher() override { stop(); }

    bool start(const WatcherConfig& config, FileWatchCallback callback) override {
        if (running_.load()) return false;

        config_ = config;
        callback_ = std::move(callback);
        event_count_ = 0;
        running_ = true;

        worker_thread_ = std::thread([this]() {
            watch_loop();
            running_ = false;
        });

        std::cout << "[FileWatcher] Started monitoring " << config_.watch_paths.size()
                  << " path(s), recursive=" << (config_.recursive ? "yes" : "no")
                  << ", debounce=" << config_.debounce_ms << "ms\n";
        return true;
    }

    void stop() override {
        if (!running_.load()) return;
        running_ = false;
        signal_stop();
        if (worker_thread_.joinable()) worker_thread_.join();
        std::cout << "[FileWatcher] Stopped. Total events: " << event_count_.load() << "\n";
    }

    bool add_watch(const std::filesystem::path& path) override {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.watch_paths.push_back(path);
        // Requires restart to take effect with current implementation
        // A more advanced version would hotpatch the watch set
        return true;
    }

    bool remove_watch(const std::filesystem::path& path) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& paths = config_.watch_paths;
        paths.erase(std::remove(paths.begin(), paths.end(), path), paths.end());
        return true;
    }

    bool is_running() const override { return running_.load(); }
    uint64_t events_processed() const override { return event_count_.load(); }
};

// Register in the provider registry
static auto reg = []() {
    Registry<IFileWatcher>::instance().add("native", []() {
        return std::make_unique<NativeFileWatcher>();
    });
    return true;
}();

} // namespace fo::core
