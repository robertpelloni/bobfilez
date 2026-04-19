#include <gtest/gtest.h>
#include "fo/core/vault_manager.hpp"
#include "fo/core/registry.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>

using namespace fo::core;

namespace fo::core::providers { extern void force_register_vault_manager(); }
class VaultManagerTest : public ::testing::Test {
public:
    VaultManagerTest() { fo::core::providers::force_register_vault_manager(); }
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
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    EXPECT_FALSE(std::filesystem::exists(vault_dir));
    EXPECT_TRUE(vm->initialize(vault_dir, "my_password"));
    EXPECT_TRUE(std::filesystem::is_directory(vault_dir));
}

TEST_F(VaultManagerTest, InitializeWithExistingDirectory) {
    std::filesystem::create_directories(vault_dir);
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    EXPECT_TRUE(vm->initialize(vault_dir, "password123"));
}

TEST_F(VaultManagerTest, LockFileReturnsTrueForExistingFileAndEncryptsIt) {
    create_file(files_dir / "secret.txt", "classified info");
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");

    EXPECT_TRUE(vm->lock_file(files_dir / "secret.txt"));

    // File should be moved/deleted from original path
    EXPECT_FALSE(std::filesystem::exists(files_dir / "secret.txt"));

    // We should have at least one file in vault dir
    bool found_encrypted = false;
    for (const auto& entry : std::filesystem::directory_iterator(vault_dir)) {
        if (entry.is_regular_file()) {
            found_encrypted = true;
            break;
        }
    }
    EXPECT_TRUE(found_encrypted);
}

TEST_F(VaultManagerTest, LockFileReturnsFalseForMissingFile) {
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");

    EXPECT_FALSE(vm->lock_file(files_dir / "nonexistent.txt"));
}

TEST_F(VaultManagerTest, UnlockFileReturnsTrueAndDecrypts) {
    create_file(files_dir / "secret.txt", "classified info to decrypt");
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");

    EXPECT_TRUE(vm->lock_file(files_dir / "secret.txt"));

    auto contents = vm->list_contents();
    ASSERT_EQ(contents.size(), 1);

    std::string vault_id = contents[0].id;
    std::filesystem::path dest_file = files_dir / "restored.txt";

    EXPECT_TRUE(vm->unlock_file(vault_id, dest_file));
    EXPECT_TRUE(std::filesystem::exists(dest_file));

    // Check if decrypted content matches original
    std::ifstream ifs(dest_file, std::ios::binary);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    EXPECT_EQ(buffer.str(), "classified info to decrypt");
}

TEST_F(VaultManagerTest, UnlockFileWithWrongPasswordFails) {
    create_file(files_dir / "secret.txt", "classified info to decrypt");
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");
    EXPECT_TRUE(vm->lock_file(files_dir / "secret.txt"));

    auto contents = vm->list_contents();
    ASSERT_EQ(contents.size(), 1);
    std::string vault_id = contents[0].id;

    auto vm2 = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm2->initialize(vault_dir, "wrong_pass");
    std::filesystem::path dest_file = files_dir / "restored.txt";

    EXPECT_FALSE(vm2->unlock_file(vault_id, dest_file));
}

TEST_F(VaultManagerTest, ListContentsReturnsCorrectItems) {
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");

    auto contents = vm->list_contents();
    EXPECT_TRUE(contents.empty());

    create_file(files_dir / "secret1.txt", "data 1");
    vm->lock_file(files_dir / "secret1.txt");

    contents = vm->list_contents();
    EXPECT_EQ(contents.size(), 1);
    EXPECT_EQ(contents[0].original_name, "secret1.txt");
}

TEST_F(VaultManagerTest, LockMultipleFiles) {
    create_file(files_dir / "a.txt", "data a");
    create_file(files_dir / "b.txt", "data b");
    create_file(files_dir / "c.txt", "data c");

    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");

    EXPECT_TRUE(vm->lock_file(files_dir / "a.txt"));
    EXPECT_TRUE(vm->lock_file(files_dir / "b.txt"));
    EXPECT_TRUE(vm->lock_file(files_dir / "c.txt"));

    EXPECT_EQ(vm->list_contents().size(), 3);
}

TEST_F(VaultManagerTest, LockFileWithSubdirectory) {
    create_file(files_dir / "sub" / "deep" / "file.txt", "nested");
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");

    EXPECT_TRUE(vm->lock_file(files_dir / "sub" / "deep" / "file.txt"));

    auto contents = vm->list_contents();
    ASSERT_EQ(contents.size(), 1);
    EXPECT_EQ(contents[0].original_name, "file.txt");
}
