// C API surface: the vault entry points must be declared in the
// public header and work through the registry chain
// (fo_bobfilez_vault_initialize -> ensure_providers_registered ->
// Registry<IVaultManager>::create("aes256_gcm")). Before v6.5.16 the
// registry was never populated; the functions also were not declared
// in bobfilez_c_api.h at all, so C consumers could not call them.
#include <gtest/gtest.h>

#include "fo/c_api/bobfilez_c_api.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

struct CApiFixture : ::testing::Test {
    std::filesystem::path dir;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("bobfilez-capi-test-" + std::to_string(reinterpret_cast<uintptr_t>(this)));
        std::filesystem::create_directories(dir);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }
};

#ifdef FO_HAVE_OPENSSL
TEST_F(CApiFixture, VaultRoundTripThroughCApi) {
    auto src = dir / "secret.txt";
    auto dst = dir / "restored.txt";
    { std::ofstream(src) << "c-api vault payload"; }
    const auto vault = (dir / "vault").string();

    ASSERT_EQ(fo_bobfilez_vault_initialize(vault.c_str(), "c-api-pass"), 1);
    ASSERT_EQ(fo_bobfilez_vault_lock(src.string().c_str()), 1);
    EXPECT_FALSE(std::filesystem::exists(src));

    char* list = fo_bobfilez_vault_list();
    ASSERT_NE(list, nullptr);
    const std::string list_json = list;
    fo_bobfilez_free_string(list);
    const auto id_pos = list_json.find("vault_");
    ASSERT_NE(id_pos, std::string::npos) << list_json;
    const auto id_end = list_json.find('"', id_pos);
    const auto id = list_json.substr(id_pos, id_end - id_pos);

    ASSERT_EQ(fo_bobfilez_vault_unlock(id.c_str(), dst.string().c_str()), 1);
    std::ifstream in(dst);
    std::stringstream ss;
    ss << in.rdbuf();
    EXPECT_EQ(ss.str(), "c-api vault payload");
}
#endif

TEST_F(CApiFixture, ScanJsonThroughCApi) {
    { std::ofstream(dir / "a.txt") << "x"; }
    char* json = fo_bobfilez_scan_json(dir.string().c_str());
    ASSERT_NE(json, nullptr);
    const std::string s = json;
    fo_bobfilez_free_string(json);
    EXPECT_NE(s.find("a.txt"), std::string::npos) << s;
}

} // namespace
