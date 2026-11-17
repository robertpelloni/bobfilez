// CLI-level integration: drives the real fo_cli.exe built beside the
// tests (build-*/cli/fo_cli.exe) for behaviors that only exist at the
// command surface: audit --limit validation and the vault round-trip.
#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#ifdef _WIN32
#include <windows.h>
#endif
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::filesystem::path cli_path() {
    // fo_tests lives in build-*/tests/, the CLI in build-*/cli/. Resolve
    // from this executable's own location so cwd does not matter.
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (n > 0) {
        return std::filesystem::path(std::wstring(buf, n)).parent_path() / ".." / "cli" / "fo_cli.exe";
    }
#endif
    return std::filesystem::path("../cli/fo_cli.exe");
}

struct CliResult { int code; std::string out; };

CliResult run_cli(const std::string& args, const std::filesystem::path& workdir) {
    auto out_file = workdir / "cli-out.txt";
    // std::system goes through cmd.exe, whose quoting rules need an outer
    // pair of quotes when the executable path itself is quoted.
    std::string cmd = "\"\"" + cli_path().string() + "\" " + args +
                      " > \"" + out_file.string() + "\" 2>&1\"";
    int code = std::system(cmd.c_str());
    std::ifstream in(out_file);
    std::stringstream ss;
    ss << in.rdbuf();
    return {code, ss.str()};
}

struct CliFixture : ::testing::Test {
    std::filesystem::path dir;

    void SetUp() override {
        if (!std::filesystem::exists(cli_path())) {
            GTEST_SKIP() << "fo_cli.exe not built beside fo_tests";
        }
        dir = std::filesystem::temp_directory_path() /
              ("bobfilez-cli-test-" + std::to_string(reinterpret_cast<uintptr_t>(this)));
        std::filesystem::create_directories(dir);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }
};

TEST_F(CliFixture, AuditRejectsInvalidLimits) {
    auto db = (dir / "t.db").string();
    EXPECT_EQ(run_cli("audit --limit=0 --db=" + db, dir).code, 2);
    EXPECT_EQ(run_cli("audit --limit=1001 --db=" + db, dir).code, 2);
    EXPECT_EQ(run_cli("audit --limit=abc --db=" + db, dir).code, 2);
}

TEST_F(CliFixture, AuditJsonOnFreshDatabaseVerifiesEmpty) {
    auto db = (dir / "t.db").string();
    auto r = run_cli("audit --format=json --db=" + db, dir);
    ASSERT_EQ(r.code, 0) << r.out;
    EXPECT_NE(r.out.find("\"verified\": true"), std::string::npos) << r.out;
    EXPECT_NE(r.out.find("\"entries\": []"), std::string::npos) << r.out;
}

#ifdef FO_HAVE_OPENSSL
TEST_F(CliFixture, VaultRoundTripRestoresBytes) {
    auto vault = (dir / "vault").string();
    auto src = dir / "secret.txt";
    auto dst = dir / "restored.txt";
    { std::ofstream(src) << "cli vault payload 42"; }

    ASSERT_EQ(run_cli("vault-init " + vault + " test-pass", dir).code, 0);
    ASSERT_EQ(run_cli("vault-lock " + vault + " test-pass \"" + src.string() + "\"", dir).code, 0);
    auto list = run_cli("vault-list " + vault + " test-pass", dir);
    ASSERT_EQ(list.code, 0) << list.out;
    auto pos = list.out.find("vault_");
    ASSERT_NE(pos, std::string::npos) << list.out;
    auto id = list.out.substr(pos, list.out.find(' ', pos) - pos);

    ASSERT_EQ(run_cli("vault-unlock " + vault + " test-pass " + id + " \"" + dst.string() + "\"", dir).code, 0);
    std::ifstream in(dst);
    std::stringstream ss;
    ss << in.rdbuf();
    EXPECT_EQ(ss.str(), "cli vault payload 42");
}
#endif

} // namespace
