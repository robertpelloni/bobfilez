// Provider registry: the explicit-registration contract that keeps
// providers from being linker-dropped out of fo_core.lib.
#include <gtest/gtest.h>

#include "fo/core/audio_fingerprint_interface.hpp"
#include "fo/core/classification_interface.hpp"
#include "fo/core/interfaces.hpp"
#include "fo/core/lint_interface.hpp"
#include "fo/core/ocr_interface.hpp"
#include "fo/core/omni_oracle_interface.hpp"
#include "fo/core/omniflow_engine_interface.hpp"
#include "fo/core/autonomous_sync_interface.hpp"

#include <cstdlib>

#include <filesystem>
#include <fstream>
#include "fo/core/perceptual_hash_interface.hpp"
#include "fo/core/provider_registration.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/video_hash_interface.hpp"

#include <algorithm>

namespace {

bool contains(const std::vector<std::string>& names, const std::string& want) {
    return std::find(names.begin(), names.end(), want) != names.end();
}

struct RegistryFixture : ::testing::Test {
    static void SetUpTestSuite() { fo::core::register_all_providers(); }
};

TEST_F(RegistryFixture, CoreProvidersRegistered) {
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IFileScanner>::instance().names(), "std"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IHasher>::instance().names(), "fast64"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IHasher>::instance().names(), "sha256"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::ILinter>::instance().names(), "std"));
    // "dhash" is OpenCV-backed; asserted under FO_HAVE_OPENCV below.
}

#ifdef FO_HAVE_OPENCV
TEST_F(RegistryFixture, PerceptualHashersRegisteredWhenOpenCVPresent) {
    // Pins the explicit register_perceptual_opencv() path: a TU-local static
    // registrar was linker-dropped in the JUCE demo tree (dhash missing).
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IPerceptualHasher>::instance().names(), "dhash"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IPerceptualHasher>::instance().names(), "phash"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IPerceptualHasher>::instance().names(), "ahash"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IPerceptualHasher>::instance().names(), "opencv"));
}
#endif

#ifdef FO_HAVE_FFMPEG
TEST_F(RegistryFixture, VideoHasherRegisteredWhenFFmpegPresent) {
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IVideoHasher>::instance().names(), "ffmpeg"));
}
#endif

#if defined(FO_HAVE_CHROMAPRINT) && defined(FO_HAVE_FFMPEG)
TEST_F(RegistryFixture, AudioFingerprinterRegisteredWhenChromaprintPresent) {
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IAudioFingerprinter>::instance().names(), "chromaprint"));
}
#endif

TEST_F(RegistryFixture, VaultManagerRegistered) {
    // The C API creates the vault through this registry
    // (fo_bobfilez_vault_initialize); registration must be explicit via
    // register_all_providers(), not a TU-local static initializer.
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IVaultManager>::instance().names(), "aes256_gcm"));
    EXPECT_NE(fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm"), nullptr);
}

TEST_F(RegistryFixture, OracleDoesNotFabricateAnswers) {
    // OmniOracle has no local model/index wired up yet; it must say so
    // instead of inventing file citations and counts.
    auto oracle = fo::core::Registry<fo::core::IOmniOracle>::instance().create("default");
    ASSERT_NE(oracle, nullptr);
    const auto res = oracle->ask("how many files do I have?");
    EXPECT_TRUE(res.citations.empty());
    EXPECT_NE(res.answer_text.find("not connected"), std::string::npos);
}

TEST_F(RegistryFixture, FlowFailurePropagates) {
    // A workflow whose action fails (here: target_dir is under a regular
    // file, so it can never be created) must report failure, not success.
    auto flow = fo::core::Registry<fo::core::IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(flow, nullptr);
    const auto dir = std::filesystem::temp_directory_path() /
                     ("bobfilez-flow-test-" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()));
    { std::error_code ec0; std::filesystem::remove_all(dir, ec0); }
    std::filesystem::create_directories(dir);
    const auto payload = dir / "payload.txt";
    { std::ofstream(payload) << "x"; }
    const auto blocker = dir / "blocker";
    { std::ofstream(blocker) << "not a directory"; }

    fo::core::Workflow wf;
    wf.id = "test-flow-fail";
    wf.name = "Failing move";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", fo::core::FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"a1", "Action.MoveTo", fo::core::FlowNodeType::Action,
                        {{"target_dir", (blocker / "sub").string()}}});
    wf.connections.push_back({"t1", "out", "a1", "in"});
    flow->register_workflow(wf);

    EXPECT_FALSE(flow->execute_workflow("test-flow-fail", payload));
    // The payload must not have been moved by the failed action.
    EXPECT_TRUE(std::filesystem::exists(payload));
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

TEST_F(RegistryFixture, FlowMoveToRefusesExistingDestination) {
    // Action.MoveTo must not silently overwrite an existing file at
    // the destination (MSVC rename replaces). The run fails and both
    // files keep their bytes.
    auto flow = fo::core::Registry<fo::core::IOmniFlowEngine>::instance().create("default");
    ASSERT_NE(flow, nullptr);
    const auto dir = std::filesystem::temp_directory_path() /
                     ("bobfilez-flow-guard-" + std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()));
    { std::error_code ec0; std::filesystem::remove_all(dir, ec0); }
    std::filesystem::create_directories(dir / "dest");
    const auto payload = dir / "report.txt";
    { std::ofstream(payload) << "new-bytes"; }
    { std::ofstream(dir / "dest" / "report.txt") << "old-bytes"; }

    fo::core::Workflow wf;
    wf.id = "test-flow-guard";
    wf.name = "Guarded move";
    wf.is_active = true;
    wf.nodes.push_back({"t1", "Trigger.Manual", fo::core::FlowNodeType::Trigger, {}});
    wf.nodes.push_back({"a1", "Action.MoveTo", fo::core::FlowNodeType::Action,
                        {{"target_dir", (dir / "dest").string()}}});
    wf.connections.push_back({"t1", "out", "a1", "in"});
    flow->register_workflow(wf);

    EXPECT_FALSE(flow->execute_workflow("test-flow-guard", payload));
    EXPECT_TRUE(std::filesystem::exists(payload));
    std::ifstream in(dir / "dest" / "report.txt");
    std::stringstream ss;
    ss << in.rdbuf();
    EXPECT_EQ(ss.str(), "old-bytes");
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

TEST_F(RegistryFixture, SyncStatusUnknownWithoutActivity) {
    // A fresh sync service used to unconditionally report
    // "In Sync: Yes". With nothing tracked it must say unknown.
#ifdef _WIN32
    _putenv_s("BOBFILEZ_SYNC_DB", "bobfilez-sync-test-nonexistent.db");
#else
    setenv("BOBFILEZ_SYNC_DB", "bobfilez-sync-test-nonexistent.db", 1);
#endif
    auto sync = fo::core::Registry<fo::core::IAutonomousSyncService>::instance().create("default");
    ASSERT_NE(sync, nullptr);
    const auto status = sync->check_sync_status();
    EXPECT_FALSE(status.is_in_sync);
    EXPECT_FALSE(status.last_error.empty());
#ifdef _WIN32
    _putenv_s("BOBFILEZ_SYNC_DB", "");
#else
    unsetenv("BOBFILEZ_SYNC_DB");
#endif
}

TEST_F(RegistryFixture, UnknownProviderCreatesNull) {
    EXPECT_EQ(fo::core::Registry<fo::core::IFileScanner>::instance().create("does-not-exist"), nullptr);
}

} // namespace
