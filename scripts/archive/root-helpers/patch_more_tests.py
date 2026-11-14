with open("tests/test_vault_manager.cpp", "r") as f:
    text = f.read()

new_tests = """
TEST_F(VaultManagerTest, LockEmptyFile) {
    create_file(files_dir / "empty.txt", "");
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");

    EXPECT_TRUE(vm->lock_file(files_dir / "empty.txt"));
    auto contents = vm->list_contents();
    ASSERT_EQ(contents.size(), 1);

    std::filesystem::path dest_file = files_dir / "restored_empty.txt";
    EXPECT_TRUE(vm->unlock_file(contents[0].id, dest_file));

    std::ifstream ifs(dest_file, std::ios::binary);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    EXPECT_EQ(buffer.str(), "");
}

TEST_F(VaultManagerTest, UnlockInvalidVaultIdFails) {
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");
    EXPECT_FALSE(vm->unlock_file("invalid_id_not_exist", files_dir / "nope.txt"));
}

TEST_F(VaultManagerTest, MissingMetadataFallbackToDestPath) {
    create_file(files_dir / "orphan.txt", "lost name");
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");

    EXPECT_TRUE(vm->lock_file(files_dir / "orphan.txt"));
    auto contents = vm->list_contents();
    ASSERT_EQ(contents.size(), 1);

    // Manually delete the .meta file
    std::filesystem::path meta_path = vault_dir / (contents[0].id + ".meta");
    std::filesystem::remove(meta_path);

    std::filesystem::path dest_file = files_dir / "recovered_orphan.txt";
    // Should still decrypt perfectly, just won't be able to auto-infer name if dest is a dir
    EXPECT_TRUE(vm->unlock_file(contents[0].id, dest_file));

    std::ifstream ifs(dest_file, std::ios::binary);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    EXPECT_EQ(buffer.str(), "lost name");
}

TEST_F(VaultManagerTest, CorruptedEncryptedFileFailsToDecryptAndCatchesTagMismatch) {
    create_file(files_dir / "secret.txt", "this is highly confidential data");
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");

    EXPECT_TRUE(vm->lock_file(files_dir / "secret.txt"));
    auto contents = vm->list_contents();
    ASSERT_EQ(contents.size(), 1);

    // Corrupt the encrypted file by changing one byte in the middle
    std::filesystem::path enc_file = vault_dir / contents[0].id;
    std::fstream fs(enc_file, std::ios::in | std::ios::out | std::ios::binary);
    fs.seekp(32); // arbitrary byte inside payload or salt/iv area
    char b;
    fs.read(&b, 1);
    fs.seekp(32);
    b = ~b;
    fs.write(&b, 1);
    fs.close();

    std::filesystem::path dest_file = files_dir / "hacked.txt";
    // GCM tag verification should fail
    EXPECT_FALSE(vm->unlock_file(contents[0].id, dest_file));
}

TEST_F(VaultManagerTest, LargeFileEncryptionChunking) {
    // 5MB file
    std::string large_data(5 * 1024 * 1024, 'A');
    create_file(files_dir / "large.bin", large_data);
    auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");
    vm->initialize(vault_dir, "pass");

    EXPECT_TRUE(vm->lock_file(files_dir / "large.bin"));
    auto contents = vm->list_contents();
    ASSERT_EQ(contents.size(), 1);

    std::filesystem::path dest_file = files_dir / "large_restored.bin";
    EXPECT_TRUE(vm->unlock_file(contents[0].id, dest_file));

    EXPECT_EQ(std::filesystem::file_size(dest_file), 5 * 1024 * 1024);
}
"""

text += "\n" + new_tests

with open("tests/test_vault_manager.cpp", "w") as f:
    f.write(text)
