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
    std::filesystem::path original_cwd;

    void SetUp() override {
        if (!std::filesystem::exists(cli_path())) {
            GTEST_SKIP() << "fo_cli.exe not built beside fo_tests";
        }
        original_cwd = std::filesystem::current_path();
        dir = std::filesystem::temp_directory_path() /
              ("bobfilez-cli-test-" + std::to_string(reinterpret_cast<uintptr_t>(this)));
        std::filesystem::create_directories(dir);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::current_path(original_cwd, ec);
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

TEST_F(CliFixture, OrganizeMoveWritesAuditEntry) {
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "photo.txt") << "organize me"; }
    auto db = (dir / "t.db").string();

    // Organize destinations are CWD-relative; contain them in the fixture.
    std::filesystem::current_path(dir);
    auto org = run_cli("organize --rule={extension}/{name} --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(org.code, 0) << org.out;

    auto audit = run_cli("audit --format=json --db=" + db, dir);
    ASSERT_EQ(audit.code, 0) << audit.out;
    EXPECT_NE(audit.out.find("\"operation\": \"move\""), std::string::npos) << audit.out;
    EXPECT_NE(audit.out.find("\"verified\": true"), std::string::npos) << audit.out;
}

TEST_F(CliFixture, OrganizeThenUndoRestoresFile) {
    // Data-safety gate for destructive ops: an applied organize move
    // must be recoverable through the journal undo path.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "photo.txt") << "recover me"; }
    auto db = (dir / "t.db").string();

    std::filesystem::current_path(dir);
    auto org = run_cli("organize --rule={extension}/{name} --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(org.code, 0) << org.out;
    ASSERT_FALSE(std::filesystem::exists(inbox / "photo.txt")) << org.out;

    auto undo = run_cli("undo --db=" + db, dir);
    ASSERT_EQ(undo.code, 0) << undo.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "photo.txt")) << undo.out;
}

TEST_F(CliFixture, OrganizeReportsFailureWhenMovesFail) {
    // Every destination is under a regular file, so every move fails.
    // Organize must say so (nonzero exit) and leave files in place;
    // it used to exit 0 and list the planned moves as if they happened.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "aaa"; }
    // Destinations are CWD-relative: rule "blocker/{name}" resolves
    // under this regular file, so directory creation must fail.
    { std::ofstream(dir / "blocker") << "not a directory"; }
    auto db = (dir / "t.db").string();

    std::filesystem::current_path(dir);
    auto r = run_cli("organize --rule=blocker/{name} --db=" + db + " \"" + inbox.string() + "\"", dir);
    EXPECT_NE(r.code, 0) << r.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "a.txt")) << r.out;
}

TEST_F(CliFixture, OrganizeSkipsOwnDatabaseFiles) {
    // The database may live inside the organized folder; organize must
    // never try to move its own live database (or its -wal/-shm).
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "aaa"; }
    auto db = (inbox / "t.db").string();

    std::filesystem::current_path(dir);
    auto r = run_cli("organize --rule={extension}/{name} --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(r.code, 0) << r.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "t.db")) << r.out;
    EXPECT_FALSE(std::filesystem::exists(inbox / "a.txt")) << r.out;
}

TEST_F(CliFixture, DeleteDuplicatesWritesAuditEntry) {
    auto dups = dir / "dups";
    std::filesystem::create_directories(dups);
    { std::ofstream(dups / "a.txt") << "same-content"; }
    { std::ofstream(dups / "b.txt") << "same-content"; }
    auto db = (dir / "t.db").string();

    ASSERT_EQ(run_cli("duplicates --db=" + db + " \"" + dups.string() + "\"", dir).code, 0);
    auto del = run_cli("delete-duplicates --db=" + db + " \"" + dups.string() + "\"", dir);
    ASSERT_EQ(del.code, 0) << del.out;
    // Exactly one of the duplicates survives an applied delete.
    EXPECT_NE(std::filesystem::exists(dups / "a.txt"), std::filesystem::exists(dups / "b.txt"));

    auto audit = run_cli("audit --format=json --db=" + db, dir);
    ASSERT_EQ(audit.code, 0) << audit.out;
    EXPECT_NE(audit.out.find("\"operation\": \"delete\""), std::string::npos) << audit.out;
    EXPECT_NE(audit.out.find("\"verified\": true"), std::string::npos) << audit.out;
}

TEST_F(CliFixture, RenameWritesAuditEntry) {
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "old.txt") << "rename me"; }
    auto db = (dir / "t.db").string();

    auto ren = run_cli("rename --pattern=renamed-{name} --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(ren.code, 0) << ren.out;
    // RuleEngine {name} is the stem, so the extension is dropped by design.
    EXPECT_TRUE(std::filesystem::exists(inbox / "renamed-old"));

    auto audit = run_cli("audit --format=json --db=" + db, dir);
    ASSERT_EQ(audit.code, 0) << audit.out;
    EXPECT_NE(audit.out.find("\"operation\": \"rename\""), std::string::npos) << audit.out;
    EXPECT_NE(audit.out.find("\"verified\": true"), std::string::npos) << audit.out;
}

TEST_F(CliFixture, ScrubDetectsCorruptionAfterBaseline) {
    auto inbox = dir / "guarded";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "guarded.txt") << "original-bytes"; }
    auto db = (dir / "t.db").string();

    // First run establishes the persistent baseline.
    auto r1 = run_cli("scrub --format=json --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(r1.code, 0) << r1.out;
    EXPECT_NE(r1.out.find("\"corrupted\":0"), std::string::npos) << r1.out;

    // Tamper with the file; a second run (a new process, so only a
    // persisted baseline can catch this) must report the corruption.
    { std::ofstream(inbox / "guarded.txt") << "tampered-bytes!!"; }
    auto r2 = run_cli("scrub --format=json --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(r2.code, 0) << r2.out;
    EXPECT_NE(r2.out.find("\"corrupted\":1"), std::string::npos) << r2.out;
}

TEST_F(CliFixture, ConvertAcceptsToOption) {
    // The global option parser must accept the convert handler's
    // --to=/--quality= options (the dashboard bridge sends --to=);
    // it used to reject them with exit 2 before the handler ran.
    auto inbox = dir / "media";
    std::filesystem::create_directories(inbox);
    auto wav = inbox / "tone.wav";
    { std::ofstream(wav, std::ios::binary) << "RIFF\x24\x00\x00\x00WAVEfmt "; }
    auto db = (dir / "t.db").string();

    auto r = run_cli("convert --to=flac --db=" + db + " \"" + wav.string() + "\"", dir);
    EXPECT_NE(r.code, 2) << r.out;
    EXPECT_EQ(r.out.find("Unknown option"), std::string::npos) << r.out;
    // The handler must also parse the value correctly (substr off-by-one
    // used to yield "=flac" and "Unknown format").
    EXPECT_EQ(r.out.find("Unknown format"), std::string::npos) << r.out;
}

TEST_F(CliFixture, RenameBatchDryRunFindsMatches) {
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "IMG_001.txt") << "x"; }
    { std::ofstream(inbox / "IMG_002.txt") << "y"; }
    auto db = (dir / "t.db").string();

    auto r = run_cli("rename-batch --find=IMG_ --replace=Photo_ --dry-run --db=" + db +
                     " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(r.code, 0) << r.out;
    EXPECT_NE(r.out.find("IMG_001"), std::string::npos) << r.out;
    // Dry-run must not have renamed anything.
    EXPECT_TRUE(std::filesystem::exists(inbox / "IMG_001.txt"));
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
