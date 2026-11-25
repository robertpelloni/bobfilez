// Scanner contract: a root that is a file scans as that file, and a
// directory root scans its contents. Dashboard Hash/Metadata cards
// accept "file or folder" and rely on this.
#include <gtest/gtest.h>

#include "fo/core/interfaces.hpp"
#include "fo/core/provider_registration.hpp"
#include "fo/core/registry.hpp"

#include <filesystem>
#include <fstream>
#include <memory>

namespace {

struct ScannerFixture : ::testing::Test {
    std::filesystem::path dir;
    std::filesystem::path file;

    static void SetUpTestSuite() { fo::core::register_all_providers(); }

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("bobfilez-scan-test-" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()));
        { std::error_code ec0; std::filesystem::remove_all(dir, ec0); }
        std::filesystem::create_directories(dir);
        file = dir / "one.txt";
        std::ofstream(file) << "content";
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }
};

TEST_F(ScannerFixture, StdScannerAcceptsFileRoot) {
    auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
    ASSERT_NE(scanner, nullptr);
    auto from_file = scanner->scan({file}, {}, false);
    ASSERT_EQ(from_file.size(), 1u);
    EXPECT_EQ(std::filesystem::path(from_file[0].uri), file);
    EXPECT_EQ(scanner->scan({dir}, {}, false).size(), 1u);
}

TEST_F(ScannerFixture, DirentScannerAcceptsFileRoot) {
    auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("dirent");
    ASSERT_NE(scanner, nullptr);
    EXPECT_EQ(scanner->scan({file}, {}, false).size(), 1u);
    EXPECT_EQ(scanner->scan({dir}, {}, false).size(), 1u);
}

#ifdef _WIN32
TEST_F(ScannerFixture, Win32ScannerAcceptsFileRoot) {
    auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("win32");
    ASSERT_NE(scanner, nullptr);
    EXPECT_EQ(scanner->scan({file}, {}, false).size(), 1u);
    EXPECT_EQ(scanner->scan({dir}, {}, false).size(), 1u);
}
#endif

} // namespace
