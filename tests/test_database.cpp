// DatabaseManager contention: two connections on one file must
// serialize briefly instead of hard-failing "database is locked"
// (concurrent CLI invocations on one --db did exactly that,
// including a migration race at startup).
#include <gtest/gtest.h>

#include "fo/core/database.hpp"

#include <filesystem>
#include <fstream>
#include <thread>

namespace {

struct DatabaseFixture : ::testing::Test {
    std::filesystem::path dir;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("bobfilez-db-test-" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()));
        { std::error_code ec0; std::filesystem::remove_all(dir, ec0); }
        std::filesystem::create_directories(dir);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }
};

TEST_F(DatabaseFixture, ConcurrentWriterWaitsInsteadOfFailing) {
    auto path = dir / "shared.db";
    fo::core::DatabaseManager a;
    a.open(path);
    a.execute("CREATE TABLE t (v INTEGER);");

    fo::core::DatabaseManager b;
    b.open(path);

    // Hold a write transaction on A; B's insert must wait for the
    // busy timeout and then succeed, not fail instantly.
    a.execute("BEGIN IMMEDIATE;");
    a.execute("INSERT INTO t VALUES (1);");

    std::string error;
    std::thread writer([&]() {
        try {
            b.execute("INSERT INTO t VALUES (2);");
        } catch (const std::exception& e) {
            error = e.what();
        }
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    a.execute("COMMIT;");
    writer.join();

    EXPECT_TRUE(error.empty()) << error;
    a.close();
    b.close();
}

} // namespace
