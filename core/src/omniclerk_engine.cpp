/// @file omniclerk_engine.cpp
/// @brief Implementation of the Autonomous AI Secretary.

#include "fo/core/omniclerk_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/ocr_interface.hpp"
#include "fo/core/pii_sentinel.hpp"
#include "fo/core/file_watcher_interface.hpp"
#include "fo/core/nexus_interface.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>

namespace fo::core {

class OmniClerkImpl : public IOmniClerkEngine {
    // Note: order of members matters for destruction!
    // watcher_ must be stopped/destroyed before members it uses (pii_sentinel_, history_, etc.)
    std::unique_ptr<IFileWatcher> watcher_;
    std::filesystem::path archive_dir_;
    std::vector<IngestionReceipt> history_;
    std::mutex history_mutex_;
    PIISentinel pii_sentinel_;

public:
    OmniClerkImpl() {
        std::cout << "[OmniClerk] Initializing Autonomous AI Secretary Pipeline...\n";
        watcher_ = Registry<IFileWatcher>::instance().create("native");
    }

    ~OmniClerkImpl() override {
        stop_listener();
    }

    void start_listener(const std::filesystem::path& inbox_dir, const std::filesystem::path& archive_dir) override {
        if (!watcher_) {
            std::cerr << "[OmniClerk] ERROR: No native file watcher found.\n";
            return;
        }
        if (watcher_->is_running()) return;

        archive_dir_ = archive_dir;
        std::cout << "[OmniClerk] Starting autonomous listener on: " << inbox_dir << "\n";

        WatcherConfig config;
        config.watch_paths = { inbox_dir };
        config.recursive = false;
        config.debounce_ms = 1000; 

        watcher_->start(config, [this](const std::vector<FileChangeEvent>& events) {
            for (const auto& evt : events) {
                if (evt.type == FileEvent::Created || evt.type == FileEvent::Moved) {
                    if (!evt.is_directory) {
                        this->process_document(evt.path);
                    }
                }
            }
        });
    }

    void stop_listener() override {
        if (watcher_ && watcher_->is_running()) {
            watcher_->stop();
            std::cout << "[OmniClerk] Stopped listening.\n";
        }
    }

    IngestionReceipt process_document(const std::filesystem::path& file) override {
        std::cout << "[OmniClerk] Processing Document: " << file.filename() << "\n";
        auto t0 = std::chrono::steady_clock::now();

        IngestionReceipt rec;
        rec.original_path = file;

        // 1. OCR (Simulated/Real via Tesseract)
        auto ocr = Registry<IOCRProvider>::instance().create("tesseract");
        if (ocr && std::filesystem::exists(file)) {
            auto res = ocr->recognize(file, "eng");
            if (res) {
                rec.raw_ocr_text = res->text;
            }
        }

        // 2. PII Scanning
        std::vector<PiiMatch> pii_matches;
        if (std::filesystem::exists(file)) {
            pii_matches = pii_sentinel_.scan_file(file);
            for (const auto& match : pii_matches) {
                rec.entities.push_back({match.type, match.snippet, match.confidence});
            }
        }

        // 3. Logic to filing (Simulated movement to archive)
        std::string new_name = "filed_" + file.filename().string();
        if (!pii_matches.empty()) {
            new_name = "SENSITIVE_" + new_name;
            rec.document_type = "Sensitive Document";
        } else {
            rec.document_type = "General Document";
        }

        if (!archive_dir_.empty()) {
            std::filesystem::create_directories(archive_dir_);
            rec.filed_path = archive_dir_ / new_name;
            try {
                if (std::filesystem::exists(file)) {
                    // Use copy + remove to simulate move across devices or simple filing
                    std::filesystem::copy(file, rec.filed_path, std::filesystem::copy_options::overwrite_existing);
                    std::filesystem::remove(file);
                }
            } catch (const std::exception& e) {
                std::cerr << "[OmniClerk] Filing error: " << e.what() << "\n";
            }
        }

        auto t1 = std::chrono::steady_clock::now();
        rec.processing_time_sec = std::chrono::duration<double>(t1 - t0).count();
        
        {
            std::lock_guard<std::mutex> lock(history_mutex_);
            history_.push_back(rec);
        }
        
        auto nexus = Registry<INexus>::instance().get_shared("default");
        if (nexus) {
            nexus->report_metric("OmniClerk", "DocumentProcessed", 1.0);
        }

        std::cout << "[OmniClerk] Filed document successfully: " << rec.filed_path.filename() << "\n";
        return rec;
    }

    std::vector<IngestionReceipt> get_processing_history() override {
        std::lock_guard<std::mutex> lock(history_mutex_);
        return history_;
    }
};

static bool register_omniclerk_engine_guard = []() {
    Registry<IOmniClerkEngine>::instance().add("default", []() {
        return std::make_unique<OmniClerkImpl>();
    });
    return true;
}();

void register_omniclerk_engine() { (void)register_omniclerk_engine_guard; }

} // namespace fo::core
