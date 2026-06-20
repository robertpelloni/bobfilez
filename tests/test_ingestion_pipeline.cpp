#include <gtest/gtest.h>
#include "fo/core/engine.hpp"
#include "fo/core/omniclerk_interface.hpp"
#include "fo/core/pii_sentinel.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace fo::core;

class IngestionPipelineTest : public ::testing::Test {
protected:
    std::filesystem::path test_root = "test_ingestion";
    std::filesystem::path inbox = test_root / "inbox";
    std::filesystem::path archive = test_root / "archive";
    std::filesystem::path db_path = test_root / "test.db";

    void SetUp() override {
        register_all_providers();
        if (std::filesystem::exists(test_root)) std::filesystem::remove_all(test_root);
        std::filesystem::create_directories(inbox);
        std::filesystem::create_directories(archive);
    }

    void TearDown() override {
        Registry<IOmniClerkEngine>::instance().clear_shared_instances();
        std::filesystem::remove_all(test_root);
    }
};

TEST_F(IngestionPipelineTest, EndToEndIngestionWithPII) {
    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto& clerk = engine.omniclerk_engine();
    PIISentinel sentinel;
    
    // Start autonomous listener
    clerk.start_listener(inbox, archive);
    
    // Give some time for watcher to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 1. Create a document in the inbox containing PII
    std::filesystem::path doc_path = inbox / "sensitive_invoice.pdf";
    {
        std::ofstream ofs(doc_path);
        ofs << "Simulated Invoice Content\n"
            << "Vendor: Amazon\n"
            << "Total: $99.99\n"
            << "Customer SSN: 123-45-6789\n"
            << "Date: 2026-06-02";
        ofs.flush();
    }

    // 2. Wait for OmniClerk to detect and process
    // (Debounce is 1s, processing is real now)
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    // 3. Verify history
    auto history = clerk.get_processing_history();
    ASSERT_GE(history.size(), 1);
    
    bool found = false;
    for (const auto& rec : history) {
        if (rec.original_path.filename() == "sensitive_invoice.pdf") {
            found = true;
            // The document is MOVED by clerk, so check filed_path
            auto pii_matches = sentinel.scan_file(rec.filed_path);
            EXPECT_GE(pii_matches.size(), 1u);
            
            EXPECT_EQ(rec.document_type, "Sensitive Document");
            EXPECT_FALSE(rec.filed_path.empty());
            EXPECT_TRUE(std::filesystem::exists(rec.filed_path));
            break;
        }
    }
    EXPECT_TRUE(found);

    clerk.stop_listener();
}

TEST_F(IngestionPipelineTest, ManualProcess) {
    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto& clerk = engine.omniclerk_engine();

    std::filesystem::path doc_path = inbox / "manual_doc.txt";
    {
        std::ofstream ofs(doc_path);
        ofs << "Manual document text with Email: test@example.com";
    }

    auto receipt = clerk.process_document(doc_path);
    
    EXPECT_EQ(receipt.original_path, doc_path);
    EXPECT_EQ(receipt.document_type, "Sensitive Document");
    EXPECT_FALSE(receipt.entities.empty());
    
    bool found_email = false;
    for (auto& ent : receipt.entities) {
        if (ent.key == "Email") found_email = true;
    }
    EXPECT_TRUE(found_email);
}
