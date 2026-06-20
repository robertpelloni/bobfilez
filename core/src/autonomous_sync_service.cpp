/// @file autonomous_sync_service.cpp
/// @brief Implementation of the Autonomous Sync Service.

#include "fo/core/autonomous_sync_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/nexus_interface.hpp"
#include "fo/core/database.hpp"
#include "fo/core/file_watcher_interface.hpp"
#include "fo/core/swarm_engine_interface.hpp"
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <fstream>
#include <sqlite3.h>
#include <algorithm>

namespace fo::core {

class AutonomousSyncServiceImpl : public IAutonomousSyncService {
    std::recursive_mutex mutex_;
    std::unordered_map<std::string, std::vector<FileVersion>> version_history_;
    std::unique_ptr<IHasher> hasher_;
    std::unique_ptr<DatabaseManager> db_;
    std::shared_ptr<ISwarmEngine> swarm_engine_;
    uint64_t swarm_sub_handle_ = 0;
    std::filesystem::path storage_path_;

    void load_from_db() {
        if (!db_) return;
        
        const char* sql = "SELECT remote_path, version_id, timestamp, checksum, author FROM sync_history ORDER BY timestamp ASC;";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_->get_db(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string remote_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            FileVersion v;
            v.version_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            v.timestamp = std::chrono::system_clock::from_time_t(sqlite3_column_int64(stmt, 2));
            v.checksum = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            v.author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

            version_history_[remote_path].push_back(v);
        }
        sqlite3_finalize(stmt);
    }

    void save_to_db(const std::string& remote_dest, const FileVersion& v) {
        if (!db_) return;
        
        const char* sql = "INSERT INTO sync_history (remote_path, version_id, timestamp, checksum, author) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_->get_db(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;

        sqlite3_bind_text(stmt, 1, remote_dest.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, v.version_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, std::chrono::system_clock::to_time_t(v.timestamp));
        sqlite3_bind_text(stmt, 4, v.checksum.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, v.author.c_str(), -1, SQLITE_TRANSIENT);

        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

public:
    AutonomousSyncServiceImpl() {
        hasher_ = Registry<IHasher>::instance().create("xxhash");
        if (!hasher_) {
            hasher_ = Registry<IHasher>::instance().create("fast64");
        }
        std::cout << "[SyncService] Initializing Autonomous Sync Protocol...\n";
    }

    void set_swarm_engine(std::shared_ptr<ISwarmEngine> swarm) override {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (swarm_engine_ && swarm_sub_handle_ != 0) {
            swarm_engine_->unsubscribe(swarm_sub_handle_);
        }
        swarm_engine_ = swarm;
        if (swarm_engine_) {
            swarm_sub_handle_ = swarm_engine_->subscribe_to_updates([this](const std::string& file_id) {
                this->download_file(file_id, "simulated_local_path");
            });
        }
    }

    void set_storage_path(const std::filesystem::path& path) override {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (db_) return; // Already initialized
        storage_path_ = path;
    }

    void ensure_db_ready() {
        if (db_) return;

        std::string db_path = storage_path_.empty() ? "sync.db" : storage_path_.string();
        if (const char* env_db = std::getenv("BOBFILEZ_SYNC_DB")) {
            db_path = env_db;
        }

        std::cout << "[SyncService] Opening sync database at: " << db_path << "\n";
        db_ = std::make_unique<DatabaseManager>();
        db_->open(db_path);
        db_->migrate();
        load_from_db();
        std::cout << "[SyncService] Database ready.\n";
    }

    bool upload_file(const std::filesystem::path& local_path, const std::string& remote_dest) override {
        bool broadcast_needed = false;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            std::cout << "[SyncService] upload_file start: " << local_path << " -> " << remote_dest << "\n";
            ensure_db_ready();
            
            if (!std::filesystem::exists(local_path)) {
                std::cout << "[SyncService] Local path does not exist: " << local_path << "\n";
                return false;
            }

            std::string current_checksum = "unknown";
            if (std::filesystem::is_directory(local_path)) {
                current_checksum = "directory";
            } else if (hasher_) {
                current_checksum = hasher_->fast64(local_path);
            }

            auto& history = version_history_[remote_dest];
            if (!history.empty() && history.back().checksum == current_checksum) {
                std::cout << "[SyncService] Already in sync: " << remote_dest << "\n";
                return true;
            }

            FileVersion v;
            v.version_id = "v" + std::to_string(history.size() + 1);
            v.timestamp = std::chrono::system_clock::now();
            v.checksum = current_checksum;
            v.author = "autonomous-agent";
            
            history.push_back(v);
            save_to_db(remote_dest, v);
            broadcast_needed = true;
            std::cout << "[SyncService] Version " << v.version_id << " tracked for " << remote_dest << "\n";
        }

        if (broadcast_needed && swarm_engine_) {
            swarm_engine_->broadcast_index_update(remote_dest);
        }
        
        auto nexus = Registry<INexus>::instance().get_shared("default");
        if (nexus) {
            nexus->report_metric("SyncService", "UploadCount", 1.0);
        }

        return true;
    }

    bool download_file(const std::string& remote_path, const std::filesystem::path& local_dest) override {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        std::cout << "[SyncService] download_file: " << remote_path << "\n";
        ensure_db_ready();

        auto& history = version_history_[remote_path];
        FileVersion v;
        v.version_id = "v" + std::to_string(history.size() + 1) + "-downloaded";
        v.timestamp = std::chrono::system_clock::now();
        v.checksum = "synced";
        v.author = "peer-node";
        
        history.push_back(v);
        save_to_db(remote_path, v);

        return true;
    }

    std::vector<FileVersion> get_version_history(const std::string& file_id) override {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        std::cout << "[SyncService] get_version_history: " << file_id << "\n";
        ensure_db_ready();
        return version_history_[file_id];
    }

    SyncStatus check_sync_status() override {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        SyncStatus status;
        status.is_in_sync = true;
        return status;
    }

    bool resolve_collision(const std::string& file_id, const std::string& preferred_version_id) override {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        ensure_db_ready();
        
        auto it = version_history_.find(file_id);
        if (it == version_history_.end()) {
            return false;
        }

        auto& history = it->second;
        auto v_it = std::find_if(history.begin(), history.end(), [&](const FileVersion& v) {
            return v.version_id == preferred_version_id;
        });

        if (v_it == history.end()) {
            return false;
        }

        FileVersion promoted = *v_it;
        promoted.version_id = "v" + std::to_string(history.size() + 1) + "-resolved";
        promoted.timestamp = std::chrono::system_clock::now();
        history.push_back(promoted);
        save_to_db(file_id, promoted);
        
        return true;
    }

    std::unique_ptr<IFileWatcher> watcher_;

    void start_sync_daemon(const std::filesystem::path& path) override {
        if (!std::filesystem::exists(path)) {
            return;
        }

        watcher_ = Registry<IFileWatcher>::instance().create("native");
        if (!watcher_) return;

        WatcherConfig cfg;
        cfg.watch_paths = { path };
        cfg.recursive = true;
        cfg.debounce_ms = 1000;

        watcher_->start(cfg, [this, path](const std::vector<FileChangeEvent>& events) {
            for (const auto& evt : events) {
                if (evt.type == FileEvent::Created || evt.type == FileEvent::Modified) {
                    if (evt.is_directory) continue;
                    
                    auto rel = std::filesystem::relative(evt.path, path);
                    std::string remote_id = "sync/" + rel.string();
                    std::replace(remote_id.begin(), remote_id.end(), '\\', '/');

                    this->upload_file(evt.path, remote_id);
                }
            }
        });
    }

    ~AutonomousSyncServiceImpl() {
        if (watcher_) {
            watcher_->stop();
        }
        if (swarm_engine_ && swarm_sub_handle_ != 0) {
            swarm_engine_->unsubscribe(swarm_sub_handle_);
        }
    }

    bool validate_live_environment(const std::filesystem::path& test_root) override {
        try {
            std::filesystem::create_directories(test_root);
            std::filesystem::path local_p = test_root / "live_test.txt";
            std::string remote_id = "remote/live_test.txt";

            {
                std::ofstream ofs(local_p);
                if (!ofs) return false;
                ofs << "Initial content for live validation.";
            }

            if (!upload_file(local_p, remote_id)) return false;

            {
                std::ofstream ofs(local_p, std::ios::app);
                if (!ofs) return false;
                ofs << "\nAppended content.";
            }

            if (!upload_file(local_p, remote_id)) return false;

            auto history = get_version_history(remote_id);
            if (history.size() < 2) {
                return false;
            }

            std::filesystem::remove(local_p);
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    }
};

static bool reg_sync_service = []() {
    Registry<IAutonomousSyncService>::instance().add("default", []() {
        return std::make_unique<AutonomousSyncServiceImpl>();
    });
    return true;
}();

void register_autonomous_sync_service() { (void)reg_sync_service; }

} // namespace fo::core
