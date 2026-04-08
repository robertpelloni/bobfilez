#include <gtest/gtest.h>
#include "fo/core/file_watcher_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

using namespace fo::core;

class FileWatcherTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_watcher_" + unique_id);
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content = "data") {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path);
        ofs << content;
        ofs.close();
    }

    std::filesystem::path test_dir;
};

TEST_F(FileWatcherTest, NativeWatcherIsRegistered) {
    auto watcher = Registry<IFileWatcher>::instance().create("native");
    ASSERT_NE(watcher, nullptr);
}

TEST_F(FileWatcherTest, StartAndStop) {
    auto watcher = Registry<IFileWatcher>::instance().create("native");
    ASSERT_NE(watcher, nullptr);

    WatcherConfig config;
    config.watch_paths = {test_dir};
    config.recursive = true;
    config.debounce_ms = 100;

    bool callback_called = false;
    bool started = watcher->start(config, [&](const std::vector<FileChangeEvent>&) {
        callback_called = true;
    });
    EXPECT_TRUE(started);
    EXPECT_TRUE(watcher->is_running());

    // Give the watcher thread time to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    watcher->stop();
    EXPECT_FALSE(watcher->is_running());
}

TEST_F(FileWatcherTest, DetectsNewFile) {
    auto watcher = Registry<IFileWatcher>::instance().create("native");
    ASSERT_NE(watcher, nullptr);

    WatcherConfig config;
    config.watch_paths = {test_dir};
    config.recursive = true;
    config.debounce_ms = 100;

    std::vector<FileChangeEvent> captured_events;
    std::mutex events_mutex;

    bool started = watcher->start(config, [&](const std::vector<FileChangeEvent>& events) {
        std::lock_guard<std::mutex> lock(events_mutex);
        for (const auto& e : events) {
            captured_events.push_back(e);
        }
    });
    ASSERT_TRUE(started);

    // Give the watcher time to start monitoring
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Create a file
    create_file(test_dir / "new_file.txt", "hello watcher");

    // Wait for the debounce + event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    watcher->stop();

    std::lock_guard<std::mutex> lock(events_mutex);
    bool found_create = false;
    for (const auto& e : captured_events) {
        if (e.path.filename() == "new_file.txt" && e.type == FileEvent::Created) {
            found_create = true;
        }
    }
    EXPECT_TRUE(found_create) << "Expected Created event for new_file.txt, got " << captured_events.size() << " events";
}

TEST_F(FileWatcherTest, DetectsFileModification) {
    auto watcher = Registry<IFileWatcher>::instance().create("native");
    ASSERT_NE(watcher, nullptr);

    // Pre-create a file
    auto file_path = test_dir / "modify_me.txt";
    create_file(file_path, "original");

    WatcherConfig config;
    config.watch_paths = {test_dir};
    config.recursive = true;
    config.debounce_ms = 100;

    std::vector<FileChangeEvent> captured_events;
    std::mutex events_mutex;

    bool started = watcher->start(config, [&](const std::vector<FileChangeEvent>& events) {
        std::lock_guard<std::mutex> lock(events_mutex);
        for (const auto& e : events) {
            captured_events.push_back(e);
        }
    });
    ASSERT_TRUE(started);

    // Give the watcher time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Modify the file
    create_file(file_path, "modified content!");

    // Wait for event processing
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    watcher->stop();

    std::lock_guard<std::mutex> lock(events_mutex);
    bool found_modify = false;
    for (const auto& e : captured_events) {
        if (e.path.filename() == "modify_me.txt" && e.type == FileEvent::Modified) {
            found_modify = true;
        }
    }
    EXPECT_TRUE(found_modify) << "Expected Modified event, got " << captured_events.size() << " events";
}

TEST_F(FileWatcherTest, EventsProcessedCounter) {
    auto watcher = Registry<IFileWatcher>::instance().create("native");
    ASSERT_NE(watcher, nullptr);

    WatcherConfig config;
    config.watch_paths = {test_dir};
    config.recursive = true;
    config.debounce_ms = 50;

    bool started = watcher->start(config, [&](const std::vector<FileChangeEvent>&) {});
    ASSERT_TRUE(started);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Create multiple files
    for (int i = 0; i < 5; ++i) {
        create_file(test_dir / ("batch_" + std::to_string(i) + ".txt"), "data");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    watcher->stop();

    // Should have processed at least some events
    EXPECT_GT(watcher->events_processed(), 0u);
}

TEST_F(FileWatcherTest, IgnoresHiddenFiles) {
    auto watcher = Registry<IFileWatcher>::instance().create("native");
    ASSERT_NE(watcher, nullptr);

    WatcherConfig config;
    config.watch_paths = {test_dir};
    config.recursive = true;
    config.debounce_ms = 100;
    config.ignore_hidden = true;

    std::vector<FileChangeEvent> captured_events;
    std::mutex events_mutex;

    bool started = watcher->start(config, [&](const std::vector<FileChangeEvent>& events) {
        std::lock_guard<std::mutex> lock(events_mutex);
        for (const auto& e : events) {
            captured_events.push_back(e);
        }
    });
    ASSERT_TRUE(started);

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Create a hidden file and a normal file
    create_file(test_dir / ".hidden_file", "hidden");
    create_file(test_dir / "visible.txt", "visible");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    watcher->stop();

    std::lock_guard<std::mutex> lock(events_mutex);
    bool found_visible = false;
    bool found_hidden = false;
    for (const auto& e : captured_events) {
        if (e.path.filename() == "visible.txt") found_visible = true;
        if (e.path.filename() == ".hidden_file") found_hidden = true;
    }
    EXPECT_TRUE(found_visible);
    EXPECT_FALSE(found_hidden);
}

TEST_F(FileWatcherTest, ExtensionFilter) {
    auto watcher = Registry<IFileWatcher>::instance().create("native");
    ASSERT_NE(watcher, nullptr);

    WatcherConfig config;
    config.watch_paths = {test_dir};
    config.recursive = true;
    config.debounce_ms = 100;
    config.include_extensions = {"txt"};

    std::vector<FileChangeEvent> captured_events;
    std::mutex events_mutex;

    bool started = watcher->start(config, [&](const std::vector<FileChangeEvent>& events) {
        std::lock_guard<std::mutex> lock(events_mutex);
        for (const auto& e : events) {
            captured_events.push_back(e);
        }
    });
    ASSERT_TRUE(started);

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Create a .txt and a .log file
    create_file(test_dir / "included.txt", "text");
    create_file(test_dir / "excluded.log", "log data");

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    watcher->stop();

    std::lock_guard<std::mutex> lock(events_mutex);
    bool found_txt = false;
    bool found_log = false;
    for (const auto& e : captured_events) {
        if (e.path.filename() == "included.txt") found_txt = true;
        if (e.path.filename() == "excluded.log") found_log = true;
    }
    EXPECT_TRUE(found_txt);
    EXPECT_FALSE(found_log);
}
