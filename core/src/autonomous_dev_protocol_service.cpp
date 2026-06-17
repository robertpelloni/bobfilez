/// @file autonomous_dev_protocol_service.cpp
/// @brief Implementation of the Autonomous Development Protocol.

#include "fo/core/autonomous_dev_protocol_interface.hpp"
#include "fo/core/autonomous_sync_interface.hpp"
#include "fo/core/file_watcher_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/nexus_interface.hpp"
#include <iostream>
#include <mutex>
#include <thread>
#include <map>

namespace fo::core {

class AutonomousDevProtocolImpl : public IAutonomousDevProtocol {
    std::mutex mtx_;
    std::map<std::string, DevTask> tasks_;
    std::unique_ptr<IFileWatcher> watcher_;
    std::filesystem::path repo_root_;
    std::shared_ptr<IAutonomousSyncService> sync_service_;
    DevStatus status_;

public:
    AutonomousDevProtocolImpl() {
        watcher_ = Registry<IFileWatcher>::instance().create("native");
        status_.is_loop_active = false;
        std::cout << "[AutoDev] Protocol Initialized.\n";
    }

    void set_sync_service(std::shared_ptr<IAutonomousSyncService> sync) override {
        std::lock_guard<std::mutex> lock(mtx_);
        sync_service_ = sync;
    }

    void register_task(const DevTask& task) override {
        std::lock_guard<std::mutex> lock(mtx_);
        tasks_[task.id] = task;
        std::cout << "[AutoDev] Registered Task: " << task.name << " (" << task.id << ")\n";
    }

    void trigger_task(const std::string& task_id) override {
        std::lock_guard<std::mutex> lock(mtx_);
        if (tasks_.count(task_id)) {
            auto& t = tasks_[task_id];

            // 1. Resolve dependencies
            for (const auto& dep_id : t.dependencies) {
                if (status_.last_results[dep_id] != "Success") {
                    std::cout << "[AutoDev] Task " << task_id << " blocked by dependency: " << dep_id << "\n";
                    return;
                }
            }

            // 2. Checkpointing: Sync current state before task
            if (sync_service_ && !repo_root_.empty()) {
                std::cout << "[AutoDev] Creating state checkpoint before task: " << t.id << "\n";
                // In simulation, we just upload the root to represent a checkpoint
                sync_service_->upload_file(repo_root_, "checkpoints/" + t.id + "/pre_run");
                status_.last_checkpoint_id = "checkpoint-" + t.id;
            }

            std::cout << "[AutoDev] Triggering: " << t.name << " [" << t.command << "]\n";
            auto t0 = std::chrono::steady_clock::now();

            // 3. Actual execution
            if (t.command == "" || t.command == "simulate") {
                status_.last_results[task_id] = "Success";
            } else {
                int ret = std::system(t.command.c_str());
                status_.last_results[task_id] = (ret == 0) ? "Success" : "Failed";
            }

            auto t1 = std::chrono::steady_clock::now();
            double duration = std::chrono::duration<double>(t1 - t0).count();
            
            auto nexus = Registry<INexus>::instance().get_shared("default");
            if (nexus) {
                nexus->report_metric("AutoDev", "TaskTriggerCount", 1.0);
                nexus->report_metric("AutoDev", "LastTaskDuration", duration);
                if (t.type == DevTaskType::Build) nexus->report_metric("AutoDev", "BuildCount", 1.0);
                if (t.type == DevTaskType::Test) nexus->report_metric("AutoDev", "TestCount", 1.0);
            }
        }
    }

    void start_autodev_loop(const std::filesystem::path& repo_root) override {
        std::lock_guard<std::mutex> lock(mtx_);
        if (status_.is_loop_active || !watcher_) return;

        repo_root_ = repo_root;
        status_.is_loop_active = true;
        std::cout << "[AutoDev] Starting autonomous loop for: " << repo_root_ << "\n";

        WatcherConfig cfg;
        cfg.watch_paths = { repo_root_ };
        cfg.recursive = true;
        cfg.debounce_ms = 500; // Build after 5s of quiet

        watcher_->start(cfg, [this](const std::vector<FileChangeEvent>& events) {
            std::cout << "[AutoDev] Source change detected (" << events.size() << " files).\n";
            this->trigger_task("build-primary");
            this->trigger_task("test-primary");
        });
    }

    void stop_autodev_loop() override {
        std::lock_guard<std::mutex> lock(mtx_);
        if (watcher_) watcher_->stop();
        status_.is_loop_active = false;
        std::cout << "[AutoDev] Stopped loop.\n";
    }

    void provide_feedback(const std::string& task_id, bool approved, const std::string& comment = "") override {
        std::lock_guard<std::mutex> lock(mtx_);
        std::cout << "[AutoDev] Feedback received for " << task_id << ": " 
                  << (approved ? "APPROVED" : "REJECTED") << " (" << comment << ")\n";
        
        if (!approved) {
            status_.last_results[task_id] = "Rejected: " + comment;
        } else {
            status_.last_results[task_id] = "Success (Manual)";
        }

        auto nexus = Registry<INexus>::instance().get_shared("default");
        if (nexus) {
            nexus->report_metric("AutoDev", "ManualFeedbackCount", 1.0);
        }
    }

    DevStatus get_dev_status() override {
        std::lock_guard<std::mutex> lock(mtx_);
        return status_;
    }
};

static bool reg_autodev = []() {
    Registry<IAutonomousDevProtocol>::instance().add("default", []() {
        return std::make_unique<AutonomousDevProtocolImpl>();
    });
    return true;
}();

void register_autonomous_dev_protocol() { (void)reg_autodev; }

} // namespace fo::core
