#include <gtest/gtest.h>
#include "fo/core/registry.hpp"
#include "fo/core/interfaces.hpp"
#include "fo/core/provider_registration.hpp"
#include <fstream>
#include <filesystem>

using namespace fo::core;

class ScannerTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        
        // Create a temporary test directory structure
        test_dir = std::filesystem::temp_directory_path() / "fo_test_scan";
        std::filesystem::create_directories(test_dir);
        std::filesystem::create_directories(test_dir / "subdir");
        
        // Create test files
        std::ofstream(test_dir / "file1.txt") << "content1";
        std::ofstream(test_dir / "file2.jpg") << "content2";
        std::ofstream(test_dir / "subdir" / "file3.txt") << "content3";
    }
    
    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }
    
    std::filesystem::path test_dir;
};

TEST_F(ScannerTest, StdScannerExists) {
    auto scanner = Registry<IFileScanner>::instance().create("std");
    ASSERT_NE(scanner, nullptr);
}

TEST_F(ScannerTest, StdScannerFindsAllFiles) {
    auto scanner = Registry<IFileScanner>::instance().create("std");
    ASSERT_NE(scanner, nullptr);
    
    auto files = scanner->scan({test_dir}, {}, false);
    
    // Should find 3 files
    EXPECT_EQ(files.size(), 3);
}

TEST_F(ScannerTest, StdScannerFiltersExtensions) {
    auto scanner = Registry<IFileScanner>::instance().create("std");
    ASSERT_NE(scanner, nullptr);
    
    auto files = scanner->scan({test_dir}, {".txt"}, false);
    
    // Should find 2 .txt files
    EXPECT_EQ(files.size(), 2);
    
    for (const auto& f : files) {
        EXPECT_EQ(std::filesystem::path(f.uri).extension(), ".txt");
    }
}

TEST_F(ScannerTest, StdScannerReturnsCorrectFileInfo) {
    auto scanner = Registry<IFileScanner>::instance().create("std");
    ASSERT_NE(scanner, nullptr);
    
    auto files = scanner->scan({test_dir}, {".jpg"}, false);
    
    ASSERT_EQ(files.size(), 1);
    
    const auto& f = files[0];
    EXPECT_EQ(std::filesystem::path(f.uri).filename(), "file2.jpg");
    EXPECT_GT(f.size, 0);
    EXPECT_FALSE(f.is_dir);
}

#ifdef _WIN32
TEST_F(ScannerTest, Win32ScannerExists) {
    auto scanner = Registry<IFileScanner>::instance().create("win32");
    ASSERT_NE(scanner, nullptr);
}

TEST_F(ScannerTest, Win32ScannerFindsAllFiles) {
    auto scanner = Registry<IFileScanner>::instance().create("win32");
    ASSERT_NE(scanner, nullptr);
    
    auto files = scanner->scan({test_dir}, {}, false);
    
    // Should find 3 files
    EXPECT_EQ(files.size(), 3);
}
#endif

TEST_F(ScannerTest, DirentScannerExists) {
    auto scanner = Registry<IFileScanner>::instance().create("dirent");
    ASSERT_NE(scanner, nullptr);
}

TEST_F(ScannerTest, ListAvailableScanners) {
    auto names = Registry<IFileScanner>::instance().names();
    
    EXPECT_FALSE(names.empty());
    
    // Should have at least std
    bool has_std = false;
    for (const auto& name : names) {
        if (name == "std") has_std = true;
    }
    EXPECT_TRUE(has_std);
}

