#include <gtest/gtest.h>
#include "fo/core/audit_logger_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/database.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>

using namespace fo::core;

class AuditLoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        db.open(":memory:");
    }

    void TearDown() override {
        // db closes in its destructor
    }

    DatabaseManager db;
};

TEST_F(AuditLoggerTest, DefaultLoggerIsRegistered) {
    auto logger = Registry<IAuditLogger>::instance().create("default");
    ASSERT_NE(logger, nullptr);
}

TEST_F(AuditLoggerTest, LogAndRetrieveSingleEntry) {
    auto logger = Registry<IAuditLogger>::instance().create("default");
    ASSERT_NE(logger, nullptr);

    logger->log("move", "/source/file.txt", "/dest/file.txt", 1024, "abc123");

    auto history = logger->get_history(10);
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0].operation, "move");
    EXPECT_EQ(history[0].src, "/source/file.txt");
    EXPECT_EQ(history[0].dst, "/dest/file.txt");
    EXPECT_EQ(history[0].size, 1024u);
    EXPECT_EQ(history[0].checksum, "abc123");
}

TEST_F(AuditLoggerTest, LogMultipleEntries) {
    auto logger = Registry<IAuditLogger>::instance().create("default");
    ASSERT_NE(logger, nullptr);

    logger->log("move", "/a.txt", "/b.txt", 100, "hash1");
    logger->log("delete", "/c.txt", "", 200, "hash2");
    logger->log("rename", "/d.txt", "/e.txt", 300, "hash3");

    auto history = logger->get_history(10);
    ASSERT_EQ(history.size(), 3u);

    // Most recent first
    EXPECT_EQ(history[0].operation, "rename");
    EXPECT_EQ(history[1].operation, "delete");
    EXPECT_EQ(history[2].operation, "move");
}

TEST_F(AuditLoggerTest, GetHistoryRespectsLimit) {
    auto logger = Registry<IAuditLogger>::instance().create("default");
    ASSERT_NE(logger, nullptr);

    for (int i = 0; i < 10; ++i) {
        logger->log("op", "/file" + std::to_string(i), "", i, "");
    }

    auto history = logger->get_history(3);
    EXPECT_EQ(history.size(), 3u);
}

TEST_F(AuditLoggerTest, LedgerIntegrityOnSingleEntry) {
    auto logger = Registry<IAuditLogger>::instance().create("default");
    ASSERT_NE(logger, nullptr);

    logger->log("move", "/src", "/dst", 42, "cksum");

    EXPECT_TRUE(logger->verify_ledger());
}

TEST_F(AuditLoggerTest, LedgerIntegrityOnMultipleEntries) {
    auto logger = Registry<IAuditLogger>::instance().create("default");
    ASSERT_NE(logger, nullptr);

    logger->log("move", "/a", "/b", 1, "h1");
    logger->log("copy", "/c", "/d", 2, "h2");
    logger->log("delete", "/e", "", 3, "h3");
    logger->log("rename", "/f", "/g", 4, "h4");

    EXPECT_TRUE(logger->verify_ledger());
}

TEST_F(AuditLoggerTest, EntryHasNonEmptyHash) {
    auto logger = Registry<IAuditLogger>::instance().create("default");
    ASSERT_NE(logger, nullptr);

    logger->log("move", "/src", "/dst", 100, "hash");

    auto history = logger->get_history(1);
    ASSERT_EQ(history.size(), 1u);
    EXPECT_FALSE(history[0].hash.empty());
    // SHA-256 produces 64 hex chars
    EXPECT_EQ(history[0].hash.size(), 64u);
}

TEST_F(AuditLoggerTest, EntryHasTimestamp) {
    auto logger = Registry<IAuditLogger>::instance().create("default");
    ASSERT_NE(logger, nullptr);

    auto before = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    logger->log("move", "/src", "/dst", 0, "");

    auto after = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    auto history = logger->get_history(1);
    ASSERT_EQ(history.size(), 1u);
    EXPECT_GE(history[0].timestamp, before);
    EXPECT_LE(history[0].timestamp, after);
}

TEST_F(AuditLoggerTest, EmptyLedgerVerifiesTrue) {
    auto logger = Registry<IAuditLogger>::instance().create("default");
    ASSERT_NE(logger, nullptr);

    EXPECT_TRUE(logger->verify_ledger());
}

TEST_F(AuditLoggerTest, EmptyHistoryOnFreshLogger) {
    auto logger = Registry<IAuditLogger>::instance().create("default");
    ASSERT_NE(logger, nullptr);

    auto history = logger->get_history(100);
    EXPECT_TRUE(history.empty());
}

// ── Direct construction with shared DatabaseManager ──────────────────

TEST_F(AuditLoggerTest, DirectConstructionWithDb) {
    // Re-open and run migrations to create audit_ledger
    db.open(":memory:");
    db.migrate();

    // Verify the table exists
    int count = db.query_int("SELECT COUNT(*) FROM audit_ledger");
    EXPECT_EQ(count, 0);
}
