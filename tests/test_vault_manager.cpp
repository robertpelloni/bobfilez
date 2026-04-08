#include <gtest/gtest.h>
#include "fo/core/vault_manager.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>

using namespace fo::core;

class VaultManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_vault_" + unique_id);
        vault_dir = test_dir / "vault";
        files_dir = test_dir / "files";
        std::filesystem::create_directories(files_dir);
    }

    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content = "secret data") {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path, std::ios::binary);
        ofs << content;
    }

    std::filesystem::path test_dir;
    std::filesystem::path vault_dir;
    std::filesystem::path files_dir;
};

TEST_F(VaultManagerTest, InitializeCreatesDirectory) {
    VaultManager vm;
    EXPECT_FALSE(std::filesystem::exists(vault_dir));
    EXPECT_TRUE(vm.initialize(vault_dir, "my_password"));
    EXPECT_TRUE(std::filesystem::is_directory(vault_dir));
}

TEST_F(VaultManagerTest, InitializeWithExistingDirectory) {
    std::filesystem::create_directories(vault_dir);
    VaultManager vm;
    EXPECT_TRUE(vm.initialize(vault_dir, "password123"));
}

TEST_F(VaultManagerTest, LockFileReturnsTrueForExistingFile) {
    create_file(files_dir / "secret.txt", "classified info");
    VaultManager vm;
    vm.initialize(vault_dir, "pass");

    EXPECT_TRUE(vm.lock_file(files_dir / "secret.txt"));
}

TEST_F(VaultManagerTest, LockFileReturnsFalseForMissingFile) {
    VaultManager vm;
    vm.initialize(vault_dir, "pass");

    EXPECT_FALSE(vm.lock_file(files_dir / "nonexistent.txt"));
}

TEST_F(VaultManagerTest, UnlockFileReturnsTrue) {
    VaultManager vm;
    vm.initialize(vault_dir, "pass");

    EXPECT_TRUE(vm.unlock_file("vault_id_001", files_dir / "restored.txt"));
}

TEST_F(VaultManagerTest, ListContentsReturnsEmpty) {
    VaultManager vm;
    vm.initialize(vault_dir, "pass");

    auto contents = vm.list_contents();
    EXPECT_TRUE(contents.empty());
}

TEST_F(VaultManagerTest, LockMultipleFiles) {
    create_file(files_dir / "a.txt", "data a");
    create_file(files_dir / "b.txt", "data b");
    create_file(files_dir / "c.txt", "data c");

    VaultManager vm;
    vm.initialize(vault_dir, "pass");

    EXPECT_TRUE(vm.lock_file(files_dir / "a.txt"));
    EXPECT_TRUE(vm.lock_file(files_dir / "b.txt"));
    EXPECT_TRUE(vm.lock_file(files_dir / "c.txt"));
}

TEST_F(VaultManagerTest, LockFileWithSubdirectory) {
    create_file(files_dir / "sub" / "deep" / "file.txt", "nested");
    VaultManager vm;
    vm.initialize(vault_dir, "pass");

    EXPECT_TRUE(vm.lock_file(files_dir / "sub" / "deep" / "file.txt"));
}
