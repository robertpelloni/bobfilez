// Deep-tree (>260 char) walk discipline for core engines that
// enumerate directories outside the scanner: their throwing
// recursive_directory_iterator walks died, aborted, or
// silently truncated on trees containing over-long paths.
// Contract for every walker: no crash, no escaping exception,
// shallow files still processed, unenterable depth skipped.
#include <gtest/gtest.h>

#include "fo/core/registry.hpp"
#include "fo/core/fs_walk.hpp"
#include "fo/core/data_pruner_interface.hpp"
#include "fo/core/treemap_engine_interface.hpp"
#include "fo/core/enhanced_fileops_interface.hpp"

#include <filesystem>
#include <fstream>

namespace {

struct DeepTreeFixture : ::testing::Test {
    std::filesystem::path dir;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("bobfilez-deepwalk-" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()));
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
        { std::error_code ec0; std::filesystem::remove_all(std::filesystem::path("\\\\?\\" + dir.string()), ec0); }
        std::filesystem::create_directories(dir);
        { std::ofstream(dir / "setup.exe") << "shallow-bytes"; }
        auto cur = dir;
        const std::string seg(40, 'd');
        for (int i = 0; i < 6; ++i) {
            cur /= seg;
            auto verbatim = std::filesystem::path("\\\\?\\" + cur.string());
            std::filesystem::create_directories(verbatim, ec);
            ASSERT_FALSE(ec) << "fixture cannot create long path: " << ec.message();
        }
        { std::ofstream(std::filesystem::path("\\\\?\\" + (cur / "deep.txt").string())) << "deep"; }
    }

    void TearDown() override {
        std::error_code ec;
        auto verbatim = std::filesystem::path("\\\\?\\" + dir.string());
        std::filesystem::remove_all(verbatim, ec);
    }
};

} // namespace

TEST_F(DeepTreeFixture, PrunerFindsShallowFileDespiteDeepTree) {
    auto pruner = fo::core::Registry<fo::core::IDataPruner>::instance().create("default");
    ASSERT_NE(pruner, nullptr);
    std::vector<fo::core::PruningSuggestion> out;
    EXPECT_NO_THROW(out = pruner->analyze(dir));
    bool found = false;
    for (const auto& s : out) {
        if (s.path.filename() == "setup.exe") found = true;
    }
    EXPECT_TRUE(found) << "shallow installer not suggested; walk likely truncated";
}

TEST_F(DeepTreeFixture, TreemapSizesShallowFileDespiteDeepTree) {
    auto engine = fo::core::Registry<fo::core::ITreemapEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
    fo::core::TreemapNode node;
    EXPECT_NO_THROW(node = engine->build_tree(dir, 8));
    EXPECT_GE(node.size, static_cast<std::uintmax_t>(13)); // "shallow-bytes"
}

TEST_F(DeepTreeFixture, DirentScannerScansShallowFileDespiteDeepTree) {
    // Round 12: the dirent scanner's _WIN32 branch walked with
    // recursive_directory_iterator + increment(ec) — the same
    // iterator family that faulted the std scanner. It needs
    // the same stack-DFS discipline and a test that proves it.
    auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("dirent");
    ASSERT_NE(scanner, nullptr);
    std::vector<fo::core::FileInfo> files;
    EXPECT_NO_THROW(files = scanner->scan({dir}, {}, false));
    bool found = false;
    for (const auto& f : files) {
        if (f.uri.find("setup.exe") != std::string::npos) found = true;
    }
    EXPECT_TRUE(found) << "dirent scan missed the shallow file on a deep tree";
}

TEST(RenameNoReplace, RefusesExistingDestination) {
    // Stale-authorization (ChatGPT's standing class): every
    // "check exists, then rename" pair has a window where the
    // destination appears in between, and MSVC/POSIX rename
    // then REPLACES it. rename_no_replace must fail instead,
    // leaving both files byte-intact.
    auto base = std::filesystem::temp_directory_path() /
                "bobfilez-noreplace-test";
    std::error_code ec;
    std::filesystem::remove_all(base, ec);
    std::filesystem::create_directories(base);
    { std::ofstream(base / "src.txt") << "new-bytes"; }
    { std::ofstream(base / "dst.txt") << "old-bytes"; }
    EXPECT_FALSE(fo::core::rename_no_replace(base / "src.txt", base / "dst.txt", ec));
    std::string sa, sb;
    { std::ifstream a(base / "src.txt"); sa.assign(std::istreambuf_iterator<char>(a), std::istreambuf_iterator<char>()); }
    { std::ifstream b(base / "dst.txt"); sb.assign(std::istreambuf_iterator<char>(b), std::istreambuf_iterator<char>()); }
    EXPECT_EQ(sa, "new-bytes");
    EXPECT_EQ(sb, "old-bytes");
    // And the free-destination case still renames.
    EXPECT_TRUE(fo::core::rename_no_replace(base / "src.txt", base / "free.txt", ec));
    EXPECT_TRUE(std::filesystem::exists(base / "free.txt"));
    std::filesystem::remove_all(base, ec);
}

TEST_F(DeepTreeFixture, CopyEstimateCountsShallowFileDespiteDeepTree) {
    fo::core::EnhancedCopyEngine engine;
    fo::core::EnhancedCopyOptions opts;
    opts.recursive = true;
    int64_t total = -1;
    EXPECT_NO_THROW(total = engine.estimate({dir}, opts));
    EXPECT_GE(total, 13);
}
