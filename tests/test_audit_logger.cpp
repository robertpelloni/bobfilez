// Persistent forensic audit ledger: history, ordering, limits, and
// hash-chain tamper detection over a real (temp-file) database.
#include <gtest/gtest.h>

#include "fo/core/audit_logger_interface.hpp"
#include "fo/core/database.hpp"

#include <filesystem>
#include <memory>

namespace {

struct AuditFixture : ::testing::Test {
    std::filesystem::path dir;
    std::unique_ptr<fo::core::DatabaseManager> db;
    std::unique_ptr<fo::core::IAuditLogger> audit;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("bobfilez-audit-test-" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()) +
               "-" + std::to_string(reinterpret_cast<uintptr_t>(this)));
        std::filesystem::create_directories(dir);
        db = std::make_unique<fo::core::DatabaseManager>();
        db->open(dir / "test.db");
        db->migrate();
        audit = fo::core::make_audit_logger(*db);
    }

    void TearDown() override {
        audit.reset();
        db.reset();
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }
};

TEST_F(AuditFixture, EmptyLedgerVerifies) {
    EXPECT_TRUE(audit->get_history(50).empty());
    EXPECT_TRUE(audit->verify_ledger());
}

TEST_F(AuditFixture, LoggedEntriesReturnMostRecentFirstWithHashes) {
    audit->log("move", "a.txt", "b.txt", 10, "");
    audit->log("delete", "c.txt", "", 20, "");

    auto entries = audit->get_history(50);
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].operation, "delete");
    EXPECT_EQ(entries[1].operation, "move");
    EXPECT_FALSE(entries[0].hash.empty());
    EXPECT_FALSE(entries[1].hash.empty());
    EXPECT_NE(entries[0].hash, entries[1].hash);
    EXPECT_TRUE(audit->verify_ledger());
}

TEST_F(AuditFixture, HistoryRespectsLimit) {
    audit->log("move", "a.txt", "b.txt", 1, "");
    audit->log("move", "c.txt", "d.txt", 1, "");
    audit->log("move", "e.txt", "f.txt", 1, "");
    EXPECT_EQ(audit->get_history(2).size(), 2u);
    EXPECT_EQ(audit->get_history(1).front().operation, "move");
}

TEST_F(AuditFixture, TamperedEntryFailsVerification) {
    audit->log("move", "a.txt", "b.txt", 10, "");
    ASSERT_TRUE(audit->verify_ledger());
    db->execute("UPDATE audit_ledger SET operation='delete' WHERE id=1");
    EXPECT_FALSE(audit->verify_ledger());
}

} // namespace
