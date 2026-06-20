/// @file shadow_sorter_service.cpp
/// @brief Implementation of the real-time auto-organize service.

#include "fo/core/shadow_sorter_service.hpp"
#include <iostream>

namespace fo::core {

ShadowSorterService::ShadowSorterService(std::shared_ptr<IFileWatcher> watcher,
                                         std::shared_ptr<RuleEngine> rules,
                                         std::shared_ptr<EnhancedCopyEngine> copy_engine)
    : watcher_(watcher), rules_(rules), copy_engine_(copy_engine) {}

void ShadowSorterService::start(const std::vector<std::filesystem::path>& watch_paths) {
    WatcherConfig config;
    config.watch_paths = watch_paths;
    config.recursive = false; // Only watch top-level input dirs for automation
    config.debounce_ms = 2000; // Wait for file to finish writing (2s)

    watcher_->start(config, [this](auto& events) { this->handle_file_events(events); });
    std::cout << "[ShadowSorter] Active and watching for incoming files...\n";
}

void ShadowSorterService::stop() {
    watcher_->stop();
}

void ShadowSorterService::handle_file_events(const std::vector<FileChangeEvent>& events) {
    for (const auto& evt : events) {
        if (evt.type == FileEvent::Created || evt.type == FileEvent::Moved) {
            if (evt.is_directory) continue;

            std::error_code ec;
            if (!std::filesystem::exists(evt.path, ec)) continue;

            // Apply rules to find target destination
            FileInfo info;
            info.uri = evt.path.string();
            info.size = std::filesystem::file_size(evt.path, ec);
            info.mtime = std::filesystem::last_write_time(evt.path, ec);
            
            auto dest = rules_->apply_rules(info, {});
            if (dest) {
                std::cout << "[ShadowSorter] Auto-moving: " << evt.path << " -> " << *dest << "\n";
                
                // Enqueue an enhanced move operation with robust retries
                EnhancedCopyOptions opts;
                opts.move_mode = true;
                opts.auto_retry_count = 5;       // More retries for automation
                opts.auto_retry_delay_ms = 5000; // 5s between retries (wait for locks)
                opts.verify_checksums = true;    // Always verify automated moves
                opts.write_log = true;

                copy_engine_->enqueue({evt.path}, dest->parent_path(), opts, 
                    nullptr, // progress
                    [evt](const FileError& err) {
                        std::cerr << "[ShadowSorter] ERROR handling " << evt.path << ": " << err.message << "\n";
                        return FileErrorAction::Skip; // For now, skip if all engine retries fail
                    },
                    [evt](const FileOpResult& res) {
                        if (res.success) {
                            std::cout << "[ShadowSorter] Successfully handled: " << evt.path << "\n";
                        }
                    }
                );
            }
        }
    }
}

} // namespace fo::core
