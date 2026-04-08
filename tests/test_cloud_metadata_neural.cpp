/// @file test_cloud_metadata_neural.cpp
/// @brief Tests for CloudProviderManager, TinyEXIF registration, NeuralBridge.

#include <gtest/gtest.h>
#include "fo/core/cloud_provider_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/interfaces.hpp"
#include "fo/core/neural_bridge_interface.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>

using namespace fo::core;

// ── CloudProviderManager Tests ───────────────────────────────────────────

class CloudProviderTest : public ::testing::Test {
protected:
    CloudProviderManager mgr;
};

TEST_F(CloudProviderTest, AddAccount) {
    CloudAccount acc;
    acc.id = "s3-prod";
    acc.provider = "s3";
    acc.display_name = "Production S3";
    acc.region = "us-east-1";
    mgr.add_account(acc);

    auto accounts = mgr.get_accounts();
    ASSERT_EQ(accounts.size(), 1u);
    EXPECT_EQ(accounts[0].id, "s3-prod");
    EXPECT_EQ(accounts[0].provider, "s3");
}

TEST_F(CloudProviderTest, RemoveAccount) {
    CloudAccount acc;
    acc.id = "temp";
    mgr.add_account(acc);
    EXPECT_EQ(mgr.get_accounts().size(), 1u);

    mgr.remove_account("temp");
    EXPECT_TRUE(mgr.get_accounts().empty());
}

TEST_F(CloudProviderTest, RemoveNonexistentDoesNotCrash) {
    EXPECT_NO_THROW(mgr.remove_account("nonexistent"));
}

TEST_F(CloudProviderTest, MultipleAccounts) {
    for (int i = 0; i < 5; ++i) {
        CloudAccount acc;
        acc.id = "acc-" + std::to_string(i);
        acc.provider = "s3";
        mgr.add_account(acc);
    }
    EXPECT_EQ(mgr.get_accounts().size(), 5u);
}

TEST_F(CloudProviderTest, GetProviderReturnsNull) {
    // No real providers wired yet
    EXPECT_EQ(mgr.get_provider("any-id"), nullptr);
}

TEST_F(CloudProviderTest, CloudAccountCredentials) {
    CloudAccount acc;
    acc.id = "cred-test";
    acc.credentials["access_key"] = "AKIA123";
    acc.credentials["secret_key"] = "secret";
    mgr.add_account(acc);

    auto accounts = mgr.get_accounts();
    ASSERT_EQ(accounts.size(), 1u);
    EXPECT_EQ(accounts[0].credentials.at("access_key"), "AKIA123");
    EXPECT_EQ(accounts[0].credentials.at("secret_key"), "secret");
}

TEST_F(CloudProviderTest, OverwriteAccount) {
    CloudAccount acc;
    acc.id = "dup";
    acc.display_name = "First";
    mgr.add_account(acc);

    acc.display_name = "Second";
    mgr.add_account(acc);

    auto accounts = mgr.get_accounts();
    ASSERT_EQ(accounts.size(), 1u);
    EXPECT_EQ(accounts[0].display_name, "Second");
}

// ── TinyEXIF Registration Tests ─────────────────────────────────────────

class MetadataRegistrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
    }
};

TEST_F(MetadataRegistrationTest, TinyExifProviderRegistered) {
    auto provider = Registry<IMetadataProvider>::instance().create("tinyexif");
    ASSERT_NE(provider, nullptr);
    EXPECT_EQ(provider->name(), "tinyexif");
}

TEST_F(MetadataRegistrationTest, TinyExifReadNonexistentFile) {
    auto provider = Registry<IMetadataProvider>::instance().create("tinyexif");
    ASSERT_NE(provider, nullptr);

    ImageMetadata meta;
    bool result = provider->read("/nonexistent/path/image.jpg", meta);
    EXPECT_FALSE(result);
}

TEST_F(MetadataRegistrationTest, TinyExifReadNonImage) {
    auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    auto test_dir = std::filesystem::temp_directory_path() / ("fo_test_meta_" + unique_id);
    std::filesystem::create_directories(test_dir);

    // Write a plain text file
    auto txt_path = test_dir / "not_image.txt";
    { std::ofstream ofs(txt_path); ofs << "Hello World"; }

    auto provider = Registry<IMetadataProvider>::instance().create("tinyexif");
    ASSERT_NE(provider, nullptr);

    ImageMetadata meta;
    bool result = provider->read(txt_path, meta);
    EXPECT_FALSE(result); // No EXIF in a text file

    std::filesystem::remove_all(test_dir);
}

// ── NeuralBridge Tests ──────────────────────────────────────────────────

class NeuralBridgeTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
    }
};

TEST_F(NeuralBridgeTest, CreatesDefaultBridge) {
    auto bridge = Registry<INeuralBridge>::instance().create("default");
    ASSERT_NE(bridge, nullptr);
}

TEST_F(NeuralBridgeTest, LoadSuiteDoesNotCrash) {
    auto bridge = Registry<INeuralBridge>::instance().create("default");
    ASSERT_NE(bridge, nullptr);
    EXPECT_TRUE(bridge->load_suite("default"));
}

TEST_F(NeuralBridgeTest, GetStatusReturnsVector) {
    auto bridge = Registry<INeuralBridge>::instance().create("default");
    ASSERT_NE(bridge, nullptr);
    auto status = bridge->get_status();
    // Status may be empty if no ML models are available, but shouldn't crash
    EXPECT_NO_THROW(bridge->get_status());
}

TEST_F(NeuralBridgeTest, UnloadIdleModelsDoesNotCrash) {
    auto bridge = Registry<INeuralBridge>::instance().create("default");
    ASSERT_NE(bridge, nullptr);
    EXPECT_NO_THROW(bridge->unload_idle_models());
}

TEST_F(NeuralBridgeTest, ModelStatusStruct) {
    ModelStatus status;
    status.name = "TestModel";
    status.loaded = true;
    status.memory_usage_bytes = 1024 * 1024;
    status.device = "CPU";
    EXPECT_EQ(status.name, "TestModel");
    EXPECT_TRUE(status.loaded);
    EXPECT_EQ(status.memory_usage_bytes, 1048576u);
    EXPECT_EQ(status.device, "CPU");
}
