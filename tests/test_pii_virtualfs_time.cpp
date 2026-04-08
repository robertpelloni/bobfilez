/// @file test_pii_virtualfs_time.cpp
/// @brief Tests for PIISentinel, VirtualFS, TimeMachine, SemanticTagging, SwarmEngine.

#include <gtest/gtest.h>
#include "fo/core/pii_sentinel.hpp"
#include "fo/core/virtual_fs_interface.hpp"
#include "fo/core/time_machine_interface.hpp"
#include "fo/core/semantic_tagging_interface.hpp"
#include "fo/core/swarm_engine_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>

using namespace fo::core;

// ── PIISentinel Tests ───────────────────────────────────────────────────

class PIISentinelTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_pii_" + unique_id);
        std::filesystem::create_directories(test_dir);
    }
    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path);
        ofs << content;
    }

    std::filesystem::path test_dir;
};

TEST_F(PIISentinelTest, DetectSSN) {
    PIISentinel sentinel;
    auto matches = sentinel.scan_string("My SSN is 123-45-6789 please help");
    ASSERT_FALSE(matches.empty());
    EXPECT_EQ(matches[0].type, "SSN");
    EXPECT_DOUBLE_EQ(matches[0].confidence, 1.0);
}

TEST_F(PIISentinelTest, DetectAWSKey) {
    PIISentinel sentinel;
    auto matches = sentinel.scan_string("aws_access_key = AKIAIOSFODNN7EXAMPLE");
    ASSERT_FALSE(matches.empty());
    EXPECT_EQ(matches[0].type, "AWS_Key");
}

TEST_F(PIISentinelTest, DetectPrivateKey) {
    PIISentinel sentinel;
    auto matches = sentinel.scan_string("-----BEGIN RSA PRIVATE KEY-----\nMIIE...");
    ASSERT_FALSE(matches.empty());
    EXPECT_EQ(matches[0].type, "Private_Key");
}

TEST_F(PIISentinelTest, DetectEmail) {
    PIISentinel sentinel;
    auto matches = sentinel.scan_string("Contact: user@example.com for info");
    ASSERT_FALSE(matches.empty());
    EXPECT_EQ(matches[0].type, "Email");
    EXPECT_DOUBLE_EQ(matches[0].confidence, 0.5);
}

TEST_F(PIISentinelTest, NoFalsePositiveCleanText) {
    PIISentinel sentinel;
    auto matches = sentinel.scan_string("The quick brown fox jumps over the lazy dog");
    EXPECT_TRUE(matches.empty());
}

TEST_F(PIISentinelTest, ScanFile) {
    create_file(test_dir / "data.txt",
        "Name: John\n"
        "SSN: 987-65-4321\n"
        "Email: john@test.com\n"
        "No PII here");

    PIISentinel sentinel;
    auto matches = sentinel.scan_file(test_dir / "data.txt");
    EXPECT_GE(matches.size(), 2u); // At least SSN and Email
}

TEST_F(PIISentinelTest, ScanFileNonexistentReturnsEmpty) {
    PIISentinel sentinel;
    auto matches = sentinel.scan_file(test_dir / "nonexistent.txt");
    EXPECT_TRUE(matches.empty());
}

TEST_F(PIISentinelTest, ScanEmptyStringReturnsEmpty) {
    PIISentinel sentinel;
    auto matches = sentinel.scan_string("");
    EXPECT_TRUE(matches.empty());
}

TEST_F(PIISentinelTest, MaskSSN) {
    auto masked = PIISentinel::mask("123-45-6789", "SSN");
    EXPECT_NE(masked.find("6789"), std::string::npos);
    EXPECT_NE(masked.find("XXX-XX-"), std::string::npos);
}

TEST_F(PIISentinelTest, MaskCreditCard) {
    auto masked = PIISentinel::mask("4111111111111111", "CreditCard");
    EXPECT_NE(masked.find("1111"), std::string::npos);
    EXPECT_NE(masked.find("****-****-****"), std::string::npos);
}

TEST_F(PIISentinelTest, MaskShort) {
    auto masked = PIISentinel::mask("ab", "Generic");
    EXPECT_EQ(masked, "****");
}

TEST_F(PIISentinelTest, MultipleSSNsInString) {
    PIISentinel sentinel;
    auto matches = sentinel.scan_string("SSN1: 111-22-3333 SSN2: 444-55-6666");
    EXPECT_GE(matches.size(), 2u);
}

// ── VirtualFS Tests ─────────────────────────────────────────────────────

class VirtualFSTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
    }
};

TEST_F(VirtualFSTest, VirtualFolderDefaults) {
    VirtualFolder f;
    EXPECT_TRUE(f.name.empty());
    EXPECT_TRUE(f.query.empty());
    EXPECT_TRUE(f.static_files.empty());
    EXPECT_TRUE(f.is_dynamic);
}

TEST_F(VirtualFSTest, RegistryCreatesDefaultEngine) {
    auto vfs = Registry<IVirtualFileSystem>::instance().create("default");
    ASSERT_NE(vfs, nullptr);
}

TEST_F(VirtualFSTest, CreateFolder) {
    auto vfs = Registry<IVirtualFileSystem>::instance().create("default");
    ASSERT_NE(vfs, nullptr);
    EXPECT_NO_THROW(vfs->create_folder("TestFolder", "ext:cpp size:>1MB"));
}

// ── TimeMachine Tests ───────────────────────────────────────────────────

class TimeMachineTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
    }
};

TEST_F(TimeMachineTest, FileRevisionConstruction) {
    FileRevision r{};
    r.revision_id = "v1";
    r.timestamp = 12345;
    r.file_size = 1024;
    r.delta_size = 128;
    r.checksum = "abc";
    EXPECT_EQ(r.revision_id, "v1");
    EXPECT_EQ(r.timestamp, 12345);
    EXPECT_EQ(r.file_size, 1024u);
}

TEST_F(TimeMachineTest, RegistryCreatesDefaultEngine) {
    auto tm = Registry<ITimeMachine>::instance().create("default");
    ASSERT_NE(tm, nullptr);
}

// ── SemanticTagging Tests ───────────────────────────────────────────────

class SemanticTaggingTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
    }
};

TEST_F(SemanticTaggingTest, SemanticTagDefaults) {
    SemanticTag tag;
    EXPECT_EQ(tag.confidence, 0.0);
    EXPECT_TRUE(tag.name.empty());
}

TEST_F(SemanticTaggingTest, RegistryCreatesDefaultEngine) {
    auto engine = Registry<ISemanticTaggingEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

// ── SwarmEngine Tests ───────────────────────────────────────────────────

class SwarmEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
    }
};

TEST_F(SwarmEngineTest, SwarmNodeConstruction) {
    SwarmNode node{};
    node.id = "node-1";
    node.hostname = "desktop";
    node.port = 8080;
    node.is_online = true;
    EXPECT_EQ(node.id, "node-1");
    EXPECT_EQ(node.hostname, "desktop");
    EXPECT_EQ(node.port, 8080);
    EXPECT_TRUE(node.is_online);
}

TEST_F(SwarmEngineTest, RegistryCreatesDefaultEngine) {
    auto engine = Registry<ISwarmEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}
