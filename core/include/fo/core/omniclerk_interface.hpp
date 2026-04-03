#pragma once
/// @file omniclerk_interface.hpp
/// @brief Autonomous AI Secretary for bobfilez.
///
/// OmniClerk is an orchestration engine that ties together:
/// FileWatcher -> OCR -> Local LLM (NER) -> Batch Rename -> FileOps -> Audit Ledger.
/// It creates a fully autonomous document ingestion and filing pipeline.

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

namespace fo::core {

struct ExtractedEntity {
    std::string key;        // e.g., "Vendor", "TotalAmount", "Date", "InvoiceNumber"
    std::string value;      // e.g., "AWS", "$45.00", "2024-03-01", "INV-8821"
    double confidence;      // 0.0 - 1.0
};

struct IngestionReceipt {
    std::filesystem::path original_path;
    std::filesystem::path filed_path;
    std::string document_type; // e.g., "Invoice", "Contract", "Receipt"
    std::vector<ExtractedEntity> entities;
    std::string raw_ocr_text;
    double processing_time_sec;
};

class IOmniClerkEngine {
public:
    virtual ~IOmniClerkEngine() = default;

    /// Start the autonomous background listener on an inbox folder
    virtual void start_listener(const std::filesystem::path& inbox_dir, const std::filesystem::path& archive_dir) = 0;

    /// Stop the background listener
    virtual void stop_listener() = 0;

    /// Manually process a single document through the pipeline
    virtual IngestionReceipt process_document(const std::filesystem::path& file) = 0;

    /// Get a history of all successfully processed and filed documents
    virtual std::vector<IngestionReceipt> get_processing_history() = 0;
};

} // namespace fo::core
