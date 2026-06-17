#include <gtest/gtest.h>
#include "fo/core/omniclerk_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/file_watcher_interface.hpp"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace fo::core;

class OmniClerkWatcherTest : public ::testing::Test {
protected:
    std::filesystem::path inbox = "test_inbox";
    std::filesystem::path archive = "test_archive";

    void SetUp() override {
        std::filesystem::create_directories(inbox);
        std::filesystem::create_directories(archive);
    }

    void TearDown() override {
        std::filesystem::remove_all(inbox);
        std::filesystem::remove_all(archive);
    }
};

TEST_F(OmniClerkWatcherTest, RealTimeTrigger) {
    auto clerk = Registry<IOmniClerkEngine>::instance().create("default");
    ASSERT_NE(clerk, nullptr);

    clerk->start_listener(inbox, archive);

    // Give watcher time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Create a new file in the inbox
    std::filesystem::path test_file = inbox / "new_document.pdf";
    {
        std::ofstream ofs(test_file);
        ofs << "Test document content for OCR simulation.";
        ofs.flush();
    }

    // Wait for debounce and processing
    // Debounce is 1s, processing is ~1.2s
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    auto history = clerk->get_processing_history();
    EXPECT_GE(history.size(), 1);
    
    bool found = false;
    for (auto& rec : history) {
        if (rec.original_path.filename() == "new_document.pdf") {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);

    clerk->stop_listener();
}
