// FileRepository perceptual-hash lookup: stored 'dhash' values are the
// hex strings written by the dhash hasher (fo::providers::DHash::fast64),
// so find_similar_images must parse hex and compare Hamming distance
// against the same bit convention the perceptual hasher produces.
#include <gtest/gtest.h>

#include "fo/core/database.hpp"
#include "fo/core/file_repository.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>

namespace {

struct FileRepoFixture : ::testing::Test {
    std::filesystem::path dir;
    std::unique_ptr<fo::core::DatabaseManager> db;
    std::unique_ptr<fo::core::FileRepository> repo;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("bobfilez-filerepo-test-" + std::to_string(reinterpret_cast<uintptr_t>(this)));
        std::filesystem::create_directories(dir);
        db = std::make_unique<fo::core::DatabaseManager>();
        db->open(dir / "test.db");
        db->migrate();
        repo = std::make_unique<fo::core::FileRepository>(*db);
    }

    void TearDown() override {
        repo.reset();
        db.reset();
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }

    int64_t add_file(const std::string& name) {
        fo::core::FileInfo fi;
        fi.uri = (dir / name).string();
        fi.size = 1;
        fi.mtime = std::filesystem::file_time_type{};
        repo->upsert(fi);
        return fi.id;
    }

    static bool contains(const std::vector<int64_t>& ids, int64_t want) {
        return std::find(ids.begin(), ids.end(), want) != ids.end();
    }
};

TEST_F(FileRepoFixture, StoredHexHashMatchesTarget) {
    const auto id = add_file("a.png");
    repo->add_hash(id, "dhash", "ffffffffffffffff");
    EXPECT_TRUE(contains(repo->find_similar_images(~0ULL, 10), id));
}

TEST_F(FileRepoFixture, NearHexHashWithinThresholdAndFarExcluded) {
    const auto near_id = add_file("near.png");
    const auto far_id = add_file("far.png");
    repo->add_hash(near_id, "dhash", "fffffffffffffffe"); // distance 1
    repo->add_hash(far_id, "dhash", "0000000000000000");  // distance 64
    const auto matches = repo->find_similar_images(~0ULL, 10);
    EXPECT_TRUE(contains(matches, near_id));
    EXPECT_FALSE(contains(matches, far_id));
}

TEST_F(FileRepoFixture, UnparsableStoredHashIsSkippedNotZero) {
    const auto id = add_file("bad.png");
    repo->add_hash(id, "dhash", "not-a-hash");
    // A parse failure must not collapse to hash 0 and match everything.
    EXPECT_FALSE(contains(repo->find_similar_images(0, 10), id));
}

} // namespace
