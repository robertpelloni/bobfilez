// Deep-tree (>260 char) walk discipline for core engines that
// enumerate directories outside the scanner: their throwing
// recursive_directory_iterator walks died, aborted, or
// silently truncated on trees containing over-long paths.
// Contract for every walker: no crash, no escaping exception,
// shallow files still processed, unenterable depth skipped.
#include <gtest/gtest.h>

#include "fo/core/registry.hpp"
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
        { std::error_code ec0; std::filesystem::remove_all(dir, ec0); }
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

TEST_F(DeepTreeFixture, CopyEstimateCountsShallowFileDespiteDeepTree) {
    fo::core::EnhancedCopyEngine engine;
    fo::core::EnhancedCopyOptions opts;
    opts.recursive = true;
    int64_t total = -1;
    EXPECT_NO_THROW(total = engine.estimate({dir}, opts));
    EXPECT_GE(total, 13);
}
