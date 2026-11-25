// CLI-level integration: drives the real fo_cli.exe built beside the
// tests (build-*/cli/fo_cli.exe) for behaviors that only exist at the
// command surface: audit --limit validation and the vault round-trip.
#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <set>
#include <sqlite3.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

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

#ifdef _WIN32
// Runs the CLI through CreateProcessW so the argument reaches the
// process as UTF-16, exactly as a user (or the Node bridge, which
// spawns via CreateProcess) delivers it. run_cli's cmd.exe route
// would ANSI-mangle non-ASCII arguments before the CLI sees them.
CliResult run_cli_wide(const std::wstring& args, const std::filesystem::path& workdir) {
    auto out_file = workdir / "cli-out-wide.txt";
    SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
    HANDLE h = CreateFileW(out_file.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &sa,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return {-1, "CreateFileW failed"};
    std::wstring cmd = L"\"" + cli_path().wstring() + L"\" " + args;
    std::vector<wchar_t> buf(cmd.begin(), cmd.end());
    buf.push_back(L'\0');
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.hStdOutput = h;
    si.hStdError = h;
    si.dwFlags = STARTF_USESTDHANDLES;
    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessW(nullptr, buf.data(), nullptr, nullptr, TRUE, 0,
                             nullptr, workdir.c_str(), &si, &pi);
    CloseHandle(h);
    if (!ok) return {-1, "CreateProcessW failed"};
    WaitForSingleObject(pi.hProcess, 60000);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    std::ifstream in(out_file, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return {static_cast<int>(code), ss.str()};
}
#endif

// Runs the CLI with FO_FAULT_AT=<point> in its environment: the
// binary _Exit(70)s at that named checkpoint (fault_injection.hpp),
// simulating a hard kill with journals flushed but no cleanup.
// The env var lives in this process only for the one call.
CliResult run_cli_fault(const std::string& args, const std::filesystem::path& workdir,
                        const std::string& point) {
#ifdef _WIN32
    _putenv_s("FO_FAULT_AT", point.c_str());
    auto r = run_cli(args, workdir);
    _putenv_s("FO_FAULT_AT", "");
#else
    setenv("FO_FAULT_AT", point.c_str(), 1);
    auto r = run_cli(args, workdir);
    unsetenv("FO_FAULT_AT");
#endif
    return r;
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

TEST_F(CliFixture, OrganizeExtensionAliasExpands) {
    // "{extension}" is the long form the dashboard taught; it must
    // expand like {ext}, not become a literal "{extension}" folder.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "photo.txt") << "organize me"; }
    auto db = (dir / "t.db").string();

    std::filesystem::current_path(dir);
    auto org = run_cli("organize --rule={extension}/{name} --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(org.code, 0) << org.out;
    EXPECT_TRUE(std::filesystem::exists(dir / "txt" / "photo")) << org.out;
    EXPECT_FALSE(std::filesystem::exists(dir / "{extension}")) << org.out;
}

TEST_F(CliFixture, DeleteDoesNotBlockUndoOfMove) {
    // Contract (revised v6.5.72, Muse review): a delete batch
    // must not *permanently* block undoing an earlier Move, but
    // undo must not silently reverse that Move when the user
    // asked to undo the delete either. Undo refuses the delete
    // batch by name; `undo --dismiss` acknowledges it, and only
    // then does undo reach the Move. (Pre-v6.5.72 this test pinned
    // the silent fall-through that Claude flagged as the bug.)
    auto inbox = dir / "inbox";
    auto dups = dir / "dups";
    std::filesystem::create_directories(inbox);
    std::filesystem::create_directories(dups);
    { std::ofstream(inbox / "photo.txt") << "organize me"; }
    { std::ofstream(dups / "a.txt") << "same-content"; }
    { std::ofstream(dups / "b.txt") << "same-content"; }
    auto db = (dir / "t.db").string();

    std::filesystem::current_path(dir);
    ASSERT_EQ(run_cli("organize --rule={extension}/{name} --db=" + db + " \"" + inbox.string() + "\"", dir).code, 0);
    ASSERT_FALSE(std::filesystem::exists(inbox / "photo.txt"));
    ASSERT_EQ(run_cli("duplicates --db=" + db + " \"" + dups.string() + "\"", dir).code, 0);
    ASSERT_EQ(run_cli("delete-duplicates --db=" + db + " \"" + dups.string() + "\"", dir).code, 0);

    auto undo = run_cli("undo --db=" + db, dir);
    EXPECT_NE(undo.code, 0) << undo.out;
    EXPECT_NE(undo.out.find("cannot be undone"), std::string::npos) << undo.out;
    EXPECT_FALSE(std::filesystem::exists(inbox / "photo.txt")) << undo.out;

    ASSERT_EQ(run_cli("undo --dismiss --db=" + db, dir).code, 0);
    auto undo2 = run_cli("undo --db=" + db, dir);
    ASSERT_EQ(undo2.code, 0) << undo2.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "photo.txt")) << undo2.out;
}

TEST_F(CliFixture, UndoSkipsStaleRecords) {
    // If the newest move's destination was deleted externally, undo
    // must skip that stale record and undo the older move instead of
    // reporting "No operations to undo" forever. Two runs separated
    // by >1s so journal timestamps order unambiguously.
    auto inbox_a = dir / "inbox-a";
    auto inbox_b = dir / "inbox-b";
    std::filesystem::create_directories(inbox_a);
    std::filesystem::create_directories(inbox_b);
    { std::ofstream(inbox_a / "first.txt") << "first"; }
    { std::ofstream(inbox_b / "second.txt") << "second"; }
    auto db = (dir / "t.db").string();

    std::filesystem::current_path(dir);
    ASSERT_EQ(run_cli("organize --rule={extension}/{name} --db=" + db + " \"" + inbox_a.string() + "\"", dir).code, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    ASSERT_EQ(run_cli("organize --rule={extension}/{name} --db=" + db + " \"" + inbox_b.string() + "\"", dir).code, 0);
    ASSERT_TRUE(std::filesystem::exists(dir / "txt" / "first"));
    ASSERT_TRUE(std::filesystem::exists(dir / "txt" / "second"));

    std::filesystem::remove(dir / "txt" / "second");
    auto undo = run_cli("undo --db=" + db, dir);
    ASSERT_EQ(undo.code, 0) << undo.out;
    EXPECT_TRUE(std::filesystem::exists(inbox_a / "first.txt")) << undo.out;
}

TEST_F(CliFixture, OrganizeLoadsPersonaExampleRules) {
    // The shipped persona examples (pattern/target schema) must load
    // and plan moves; three of four example files used to fail with
    // "Failed to load rules" because the loader only understood the
    // rules:/destination schema.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "photo.jpg") << "fakejpg"; }
    auto db = (dir / "t.db").string();
    auto examples = std::filesystem::path(__FILE__).parent_path().parent_path() / "examples";

    for (const char* file : {"rules.yaml", "rules_developer.yaml",
                             "rules_photographer.yaml", "rules_sysadmin.yaml"}) {
        auto r = run_cli("organize --rules=\"" + (examples / file).string() +
                         "\" --dry-run --db=" + db + " \"" + inbox.string() + "\"", dir);
        ASSERT_EQ(r.code, 0) << file << ": " << r.out;
    }

    auto photo = run_cli("organize --rules=\"" + (examples / "rules_photographer.yaml").string() +
                         "\" --dry-run --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(photo.code, 0) << photo.out;
    EXPECT_NE(photo.out.find("Final_Exports"), std::string::npos) << photo.out;
}

// Writes a 64x64 24-bit BMP whose gray value depends on x:
// mode 0/1 increasing gradients (perceptually near-identical),
// mode 2 the reversed gradient (perceptually opposite).
void write_gradient_bmp(const std::filesystem::path& path, int mode) {
    const int w = 64, h = 64;
    std::ofstream out(path, std::ios::binary);
    auto u32 = [&](uint32_t v) { out.write(reinterpret_cast<const char*>(&v), 4); };
    auto u16 = [&](uint16_t v) { out.write(reinterpret_cast<const char*>(&v), 2); };
    const uint32_t data_size = static_cast<uint32_t>(w * h * 3);
    out << 'B' << 'M';
    u32(54 + data_size); u16(0); u16(0); u32(54);
    u32(40); u32(w); u32(h); u16(1); u16(24); u32(0); u32(data_size);
    u32(2835); u32(2835); u32(0); u32(0);
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            int v = (mode == 2) ? (255 - x * 4) : (x * 4 + (mode == 1 ? 2 : 0));
            if (v > 255) v = 255;
            char px[3] = {static_cast<char>(v), static_cast<char>(v), static_cast<char>(v)};
            out.write(px, 3);
        }
    }
}

// Writes a minimal 1s 440Hz PCM WAV (canonical 44-byte header).
void write_tone_wav(const std::filesystem::path& path) {
    std::ofstream out(path, std::ios::binary);
    const int rate = 44100, seconds = 1;
    const uint32_t data_size = static_cast<uint32_t>(rate * seconds * 2);
    auto u32 = [&](uint32_t v) { out.write(reinterpret_cast<const char*>(&v), 4); };
    auto u16 = [&](uint16_t v) { out.write(reinterpret_cast<const char*>(&v), 2); };
    out << "RIFF"; u32(36 + data_size); out << "WAVEfmt ";
    u32(16); u16(1); u16(1); u32(rate); u32(rate * 2); u16(2); u16(16);
    out << "data"; u32(data_size);
    for (int i = 0; i < rate * seconds; ++i) {
        int16_t s = static_cast<int16_t>(8000.0 * std::sin(6.2831853 * 440.0 * i / rate));
        out.write(reinterpret_cast<const char*>(&s), 2);
    }
}

// The missing-path conformance matrix. HelpCoversEveryPathCommand
// fails when the CLI's path-consuming set grows without a new row
// here, so this list cannot silently go stale (Muse roast).
static const std::string kMissingRootCmds[] = {
    "scan", "duplicates", "hash", "metadata", "stats", "lint",
    "export", "organize --rule={ext}/{filename}",
    "classify", "ocr", "similar", "vhash", "afingerprint",
    "delete-duplicates", "rename --pattern={name}_x",
    "rename-batch --find=a --replace=b", "scrub",
    "search needle", "convert --to=mp4",
};

// Minimal JPEG carrying an EXIF APP1 segment with only a Make
// tag (no GPS, no date). TinyEXIF reports absent GPS via
// sentinel doubles; the provider must not surface them.
void write_minimal_exif_jpeg(const std::filesystem::path& path) {
    std::vector<unsigned char> b;
    auto u16 = [&](uint16_t v) { b.push_back(v & 0xff); b.push_back((v >> 8) & 0xff); };
    auto u32 = [&](uint32_t v) { for (int i = 0; i < 4; ++i) { b.push_back(v & 0xff); v >>= 8; } };
    b.insert(b.end(), {0xff, 0xd8}); // SOI
    size_t app1_start = b.size();
    b.insert(b.end(), {0xff, 0xe1, 0, 0}); // APP1 + length placeholder
    const char* exif_hdr = "Exif\0\0";
    b.insert(b.end(), exif_hdr, exif_hdr + 6);
    size_t tiff = b.size();
    b.insert(b.end(), {'I', 'I', 42, 0});
    u32(8); // IFD0 offset
    u16(1); // one entry
    u16(0x010f); u16(2); u32(8); // Make, ASCII, 8 bytes
    u32(26); // value offset (tiff+26)
    u32(0); // next IFD
    const char* make = "TestCam\0";
    b.insert(b.end(), make, make + 8);
    size_t seg_len = b.size() - app1_start - 2;
    b[app1_start + 2] = static_cast<unsigned char>((seg_len >> 8) & 0xff);
    b[app1_start + 3] = static_cast<unsigned char>(seg_len & 0xff);
    b.insert(b.end(), {0xff, 0xd9}); // EOI
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(b.data()), static_cast<std::streamsize>(b.size()));
    (void)tiff;
}

TEST_F(CliFixture, VaultOpsAppearInAuditLedger) {
    // Vault lock/unlock are file-destructive (lock consumes the
    // source) but never reached the forensic audit ledger, so
    // the audit trail had a vault-shaped hole. They must appear
    // like every other destructive op.
    auto vault = dir / "vault";
    { std::ofstream(dir / "secret.txt") << "topsecret"; }
    auto db = (dir / "t.db").string();
    std::filesystem::current_path(dir);
    ASSERT_EQ(run_cli("vault-init \"" + vault.string() + "\" pw123 --db=" + db, dir).code, 0);
    ASSERT_EQ(run_cli("vault-lock \"" + vault.string() + "\" pw123 \"" + (dir / "secret.txt").string() + "\" --db=" + db, dir).code, 0);

    auto audit = run_cli("audit --db=" + db, dir);
    ASSERT_EQ(audit.code, 0) << audit.out;
    EXPECT_NE(audit.out.find("vault-lock"), std::string::npos) << audit.out;
    EXPECT_NE(audit.out.find("secret.txt"), std::string::npos) << audit.out;
    std::filesystem::current_path(original_cwd);
}

TEST_F(CliFixture, VaultFailedAttemptsAreAudited) {
    // A forensic trail that only records successes is a
    // brochure. Failed lock/unlock attempts (bad id, traversal,
    // wrong password, missing file) must reach the ledger too —
    // never with password material in the entry.
    auto vault = dir / "vault";
    { std::ofstream(dir / "s.txt") << "x"; }
    auto db = (dir / "t.db").string();
    std::filesystem::current_path(dir);
    ASSERT_EQ(run_cli("vault-init \"" + vault.string() + "\" pw123 --db=" + db, dir).code, 0);
    EXPECT_NE(run_cli("vault-lock \"" + vault.string() + "\" pw123 \"" +
                          (dir / "missing.txt").string() + "\" --db=" + db, dir).code, 0);
    EXPECT_NE(run_cli("vault-unlock \"" + vault.string() + "\" pw123 ../escape \"" +
                          (dir / "out.txt").string() + "\" --db=" + db, dir).code, 0);
    EXPECT_NE(run_cli("vault-list \"" + vault.string() + "\" wrongpw --db=" + db, dir).code, 0);
    auto audit = run_cli("audit --db=" + db, dir);
    ASSERT_EQ(audit.code, 0) << audit.out;
    EXPECT_NE(audit.out.find("vault-lock-failed"), std::string::npos) << audit.out;
    EXPECT_NE(audit.out.find("vault-unlock-failed"), std::string::npos) << audit.out;
    EXPECT_NE(audit.out.find("vault-initialize-failed"), std::string::npos) << audit.out;
    EXPECT_EQ(audit.out.find("pw123"), std::string::npos) << audit.out;
    EXPECT_EQ(audit.out.find("wrongpw"), std::string::npos) << audit.out;
    std::filesystem::current_path(original_cwd);
}

TEST_F(CliFixture, VaultLockUnlockCrashWindowsLoseNothing) {
    // vault-lock/unlock are destructive two-step operations.
    // Hard-kill between encrypt and source removal (and between
    // decrypt and entry removal): the result must be "both
    // present" or "fully done", never neither.
    auto vault = dir / "vault";
    { std::ofstream(dir / "s.txt") << "precious"; }
    auto db = (dir / "t.db").string();
    std::filesystem::current_path(dir);
    ASSERT_EQ(run_cli("vault-init \"" + vault.string() + "\" pw123 --db=" + db, dir).code, 0);

    auto crash = run_cli_fault("vault-lock \"" + vault.string() + "\" pw123 \"" +
                                   (dir / "s.txt").string() + "\" --db=" + db,
                               dir, "vault.after-encrypt");
    EXPECT_NE(crash.code, 0) << "fault point did not fire: " << crash.out;
    EXPECT_TRUE(std::filesystem::exists(dir / "s.txt")) << crash.out;
    auto list1 = run_cli("vault-list \"" + vault.string() + "\" pw123 --db=" + db, dir);
    EXPECT_NE(list1.out.find("(1 entries)"), std::string::npos) << list1.out;

    // Complete the lock normally, then crash the unlock.
    ASSERT_EQ(run_cli("vault-lock \"" + vault.string() + "\" pw123 \"" +
                          (dir / "s.txt").string() + "\" --db=" + db, dir).code, 0);
    EXPECT_FALSE(std::filesystem::exists(dir / "s.txt"));
    auto lst = run_cli("vault-list \"" + vault.string() + "\" pw123 --db=" + db, dir);
    auto id_pos = lst.out.find("vault_");
    ASSERT_NE(id_pos, std::string::npos) << lst.out;
    auto id_end = lst.out.find(' ', id_pos);
    std::string id = lst.out.substr(id_pos, id_end - id_pos);
    auto crash2 = run_cli_fault("vault-unlock \"" + vault.string() + "\" pw123 " + id +
                                    " \"" + (dir / "back.txt").string() + "\" --db=" + db,
                                dir, "vault.after-decrypt");
    EXPECT_NE(crash2.code, 0) << "fault point did not fire: " << crash2.out;
    ASSERT_TRUE(std::filesystem::exists(dir / "back.txt")) << crash2.out;
    std::ifstream in(dir / "back.txt");
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "precious");
    auto list2 = run_cli("vault-list \"" + vault.string() + "\" pw123 --db=" + db, dir);
    EXPECT_NE(list2.out.find(id), std::string::npos) << list2.out; // entry survives for retry
    std::filesystem::current_path(original_cwd);
}

TEST_F(CliFixture, OrganizeCaseCollisionAndReservedNameFailSafely) {
    // Windows path edges: two sources mapping to destinations
    // that differ only by case collide on NTFS (second must
    // fail, not overwrite), and a reserved device name (CON)
    // as a rename target must fail per-file, never crash or
    // silently "succeed".
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox / "x");
    std::filesystem::create_directories(inbox / "y");
    { std::ofstream(inbox / "x" / "Report.txt") << "one"; }
    { std::ofstream(inbox / "y" / "report.txt") << "two"; }
    auto db = (dir / "t.db").string();
    std::filesystem::current_path(dir);
    auto org = run_cli("organize --rule={filename} --format=json --db=" + db +
                           " \"" + inbox.string() + "\"", dir);
    EXPECT_NE(org.out.find("\"failed\": 1"), std::string::npos) << org.out;
    EXPECT_TRUE(std::filesystem::exists(dir / "Report.txt") ||
                std::filesystem::exists(dir / "report.txt")) << org.out;
    int remaining = (std::filesystem::exists(inbox / "x" / "Report.txt") ? 1 : 0) +
                    (std::filesystem::exists(inbox / "y" / "report.txt") ? 1 : 0);
    EXPECT_EQ(remaining, 1) << org.out; // loser stays home, intact

    { std::ofstream(dir / "ok.txt") << "ok"; }
    auto ren = run_cli("rename --pattern=CON --format=json --db=" + db +
                           " \"" + (dir / "ok.txt").string() + "\"", dir);
    EXPECT_NE(ren.out.find("\"failed\": 1"), std::string::npos) << ren.out;
    EXPECT_TRUE(std::filesystem::exists(dir / "ok.txt")) << ren.out;
    std::filesystem::current_path(original_cwd);
}

TEST_F(CliFixture, ConvertRefusesExistingOutputAndQuoteInjection) {
    // convert used to overwrite existing outputs silently
    // (ffmpeg -y) while every other command refuses, and it
    // built shell commands with an unchecked --output path: a
    // quote in it breaks out of the quoted argument under
    // cmd.exe. Both must be refused before any tool runs.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    write_tone_wav(inbox / "tone.wav");
    { std::ofstream(inbox / "tone.flac") << "previous conversion"; }
    auto db = (dir / "t.db").string();

    auto over = run_cli("convert --to=flac --db=" + db + " \"" + (inbox / "tone.wav").string() + "\"", dir);
    EXPECT_NE(over.code, 0) << over.out;
    EXPECT_NE(over.out.find("already exists"), std::string::npos) << over.out;
    std::ifstream in(inbox / "tone.flac");
    std::stringstream ss; ss << in.rdbuf();
    EXPECT_EQ(ss.str(), "previous conversion");

    auto inj = run_cli("convert --to=flac --output=\"evil\\\" & echo pwned > pwned.txt & \\\"\" --db=" + db +
                       " \"" + (inbox / "tone.wav").string() + "\"", dir);
    EXPECT_NE(inj.code, 0) << inj.out;
    EXPECT_FALSE(std::filesystem::exists(dir / "pwned.txt")) << inj.out;
}

TEST_F(CliFixture, IgnoreAddRemoveReportTruthfully) {
    // ignore add/remove claimed success unconditionally: adding
    // a duplicate and removing a nonexistent rule both printed
    // success and exited 0. Both must report what happened.
    auto db = (dir / "t.db").string();
    auto add1 = run_cli("ignore add \"*.log\" reason --db=" + db, dir);
    ASSERT_EQ(add1.code, 0) << add1.out;

    auto add2 = run_cli("ignore add \"*.log\" reason --db=" + db, dir);
    EXPECT_NE(add2.code, 0) << add2.out;
    EXPECT_NE(add2.out.find("already exists"), std::string::npos) << add2.out;

    auto rem1 = run_cli("ignore remove \"*.log\" --db=" + db, dir);
    ASSERT_EQ(rem1.code, 0) << rem1.out;

    auto rem2 = run_cli("ignore remove \"*.log\" --db=" + db, dir);
    EXPECT_NE(rem2.code, 0) << rem2.out;
    EXPECT_NE(rem2.out.find("No ignore rule"), std::string::npos) << rem2.out;
}

TEST_F(CliFixture, VaultUnlockRejectsTraversalId) {
    // unlock_file joined the vault id unchecked: "../x" escaped
    // the vault, and a successful decrypt DELETED the outside
    // file. Entry ids are generated names; anything containing
    // separators must be refused before touching the filesystem.
    auto vault = dir / "vault";
    { std::ofstream(dir / "secret.txt") << "topsecret"; }
    std::filesystem::current_path(dir);
    ASSERT_EQ(run_cli("vault-init \"" + vault.string() + "\" pw123", dir).code, 0);
    ASSERT_EQ(run_cli("vault-lock \"" + vault.string() + "\" pw123 \"" + (dir / "secret.txt").string() + "\"", dir).code, 0);

    // Copy the entry outside the vault, then attack via ../
    std::string entry;
    for (auto& e : std::filesystem::directory_iterator(vault)) {
        auto n = e.path().filename().string();
        if (n.rfind("vault_", 0) == 0 && n.find(".meta") == std::string::npos) entry = n;
    }
    ASSERT_FALSE(entry.empty());
    auto outside = dir / "outside-copy";
    std::filesystem::copy_file(vault / entry, outside);

    auto attack = run_cli("vault-unlock \"" + vault.string() + "\" pw123 \"../outside-copy\" \"" + (dir / "loot.txt").string() + "\"", dir);
    EXPECT_NE(attack.code, 0) << attack.out;
    EXPECT_TRUE(std::filesystem::exists(outside)) << attack.out;
    EXPECT_FALSE(std::filesystem::exists(dir / "loot.txt"));

    // Normal unlock still works.
    auto ok = run_cli("vault-unlock \"" + vault.string() + "\" pw123 \"" + entry + "\" \"" + (dir / "back.txt").string() + "\"", dir);
    ASSERT_EQ(ok.code, 0) << ok.out;
    EXPECT_TRUE(std::filesystem::exists(dir / "back.txt"));
    std::filesystem::current_path(original_cwd);
}

TEST_F(CliFixture, VaultListRejectsWrongPassword) {
    // vault-list took a password but never checked it: any
    // password listed entry names and sizes. A vault verifier
    // (written at init) must gate initialize() for list/lock/
    // unlock on vaults that have one.
    auto vault = dir / "vault";
    { std::ofstream(dir / "secret.txt") << "topsecret"; }
    std::filesystem::current_path(dir);
    auto init = run_cli("vault-init \"" + vault.string() + "\" pw123", dir);
    ASSERT_EQ(init.code, 0) << init.out;
    auto lock = run_cli("vault-lock \"" + vault.string() + "\" pw123 \"" + (dir / "secret.txt").string() + "\"", dir);
    ASSERT_EQ(lock.code, 0) << lock.out;

    auto wrong = run_cli("vault-list \"" + vault.string() + "\" wrongpw", dir);
    EXPECT_NE(wrong.code, 0) << wrong.out;
    EXPECT_EQ(wrong.out.find("secret.txt"), std::string::npos) << wrong.out;

    auto right = run_cli("vault-list \"" + vault.string() + "\" pw123", dir);
    ASSERT_EQ(right.code, 0) << right.out;
    EXPECT_NE(right.out.find("secret.txt"), std::string::npos) << right.out;
    std::filesystem::current_path(original_cwd);
}

TEST_F(CliFixture, MetadataOmitsBogusGps) {
    // A JPEG with EXIF Make but no GPS used to print
    // "GPS: 1.79769e+308, 1.79769e+308" — TinyEXIF's absent-GPS
    // sentinels surfaced as real coordinates.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    write_minimal_exif_jpeg(inbox / "photo.jpg");
    auto db = (dir / "t.db").string();
    auto r = run_cli("metadata --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(r.code, 0) << r.out;
    EXPECT_EQ(r.out.find("1.79769"), std::string::npos) << r.out;
    EXPECT_EQ(r.out.find("GPS:"), std::string::npos) << r.out;
}

TEST_F(CliFixture, MalformedTemplatesAndRegexAreUsageErrors) {
    // Garbage patterns used to "succeed": rename --pattern="{name"
    // renamed a file to a literal "{name", an unknown {bogus}
    // placeholder became a literal folder, and an invalid
    // --regex silently renamed 0 files with exit 0.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    auto db = (dir / "t.db").string();
    auto in = "\"" + inbox.string() + "\"";

    auto unclosed = run_cli("rename --pattern=\"{name\" --db=" + db + " " + in, dir);
    EXPECT_NE(unclosed.code, 0) << unclosed.out;
    EXPECT_NE(unclosed.out.find("Invalid rename pattern"), std::string::npos) << unclosed.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "a.txt"));

    auto unknown = run_cli("rename --pattern=\"{bogus}\" --db=" + db + " " + in, dir);
    EXPECT_NE(unknown.code, 0) << unknown.out;
    EXPECT_NE(unknown.out.find("Unknown placeholder"), std::string::npos) << unknown.out;

    auto org = run_cli("organize --dry-run --rule=\"{bogus}/{filename}\" --db=" + db + " " + in, dir);
    EXPECT_NE(org.code, 0) << org.out;
    EXPECT_NE(org.out.find("Unknown placeholder"), std::string::npos) << org.out;

    auto rx = run_cli("rename-batch --find=\"([\" --replace=x --regex --db=" + db + " " + in, dir);
    EXPECT_NE(rx.code, 0) << rx.out;
    EXPECT_NE(rx.out.find("Invalid regex"), std::string::npos) << rx.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "a.txt"));
}

TEST_F(CliFixture, BadNumericOptionsAreUsageErrors) {
    // --threshold=abc / --threads=abc used to abort the process
    // via uncaught std::invalid_argument (0xC0000409, no message).
    // Every numeric option must be a named usage error (exit 2).
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    auto db = (dir / "t.db").string();
    auto in = "\"" + inbox.string() + "\"";
    const std::string cmds[] = {
        "similar --threshold=abc", "scan --threads=abc",
        "search x --max-results=abc", "convert --quality=abc --to=mp4",
        "scan --min-size=abc", "scan --hash-limit-mb=abc",
    };
    for (const auto& c : cmds) {
        auto r = run_cli(c + " --db=" + db + " " + in, dir);
        EXPECT_EQ(r.code, 2) << c << ": " << r.out;
        EXPECT_NE(r.out.find("Invalid"), std::string::npos) << c << ": " << r.out;
    }
}

TEST_F(CliFixture, OrganizeRollsBackWhenJournalWriteFails) {
    // Fault injection (round-6/7; re-pinned round-8 v6.5.94):
    // replace operation_log with a view so the journal insert
    // throws. Originally the insert ran AFTER the rename and the
    // move was rolled back; with intent-before-rename ordering
    // the insert now fails BEFORE any rename, so the move is
    // never attempted. The user-visible contract is unchanged
    // and stronger: file home, nothing at dest, non-zero exit —
    // never moved but unjournaled.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    auto db = (dir / "t.db").string();
    ASSERT_EQ(run_cli("scan --db=" + db + " \"" + inbox.string() + "\"", dir).code, 0);

    sqlite3* h = nullptr;
    ASSERT_EQ(sqlite3_open(db.c_str(), &h), SQLITE_OK);
    char* err = nullptr;
    const char* mangle =
        "DROP TABLE operation_log;"
        "CREATE VIEW operation_log AS SELECT 0 AS id, 0 AS timestamp,"
        " '' AS operation_type, '' AS source_path, '' AS dest_path,"
        " 0 AS file_size, '' AS file_hash, '' AS status, 0 AS undone,"
        " 0 AS batch_id, 0 AS dest_mtime;";
    ASSERT_EQ(sqlite3_exec(h, mangle, nullptr, nullptr, &err), SQLITE_OK) << (err ? err : "");
    sqlite3_close(h);

    std::filesystem::current_path(dir);
    auto org = run_cli("organize --rule={ext}/{filename} --db=" + db + " \"" + inbox.string() + "\"", dir);
    std::filesystem::current_path(original_cwd);
    EXPECT_NE(org.code, 0) << org.out;
    EXPECT_NE(org.out.find("intent"), std::string::npos) << org.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "a.txt")) << org.out;
    EXPECT_FALSE(std::filesystem::exists(dir / "txt" / "a.txt")) << org.out;
}

TEST_F(CliFixture, OrganizeCrashRecoveryAtEachCheckpoint) {
    // Per-step crash injection (Muse roast rounds 6/8): the
    // exception-path rollback test cannot prove crash recovery —
    // a hard kill runs no rollback. With an intent row journaled
    // BEFORE the rename, every kill window must reconcile on the
    // next invocation: file home, or moved-and-journaled (then
    // undoable). Anything else is a lost file.
    const char* points[] = {"organize.after-mkdir", "organize.after-intent",
                            "organize.after-rename"};
    int n = 0;
    for (const char* point : points) {
        auto cdir = dir / ("case" + std::to_string(n++));
        auto inbox = cdir / "inbox";
        std::filesystem::create_directories(inbox);
        { std::ofstream(inbox / "a.txt") << "crash-data"; }
        auto db = (cdir / "t.db").string();
        std::filesystem::current_path(cdir); // organize dests are cwd-relative
        auto crash = run_cli_fault(
            "organize --rule={ext}/{filename} --format=json --db=" + db +
                " \"" + inbox.string() + "\"",
            cdir, point);
        EXPECT_NE(crash.code, 0) << point << ": fault point did not fire: " << crash.out;
        // Any later invocation reconciles pending intent rows.
        auto hist = run_cli("history --format=json --db=" + db, cdir);
        ASSERT_EQ(hist.code, 0) << point << ": " << hist.out;
        EXPECT_EQ(hist.out.find("pending"), std::string::npos) << point << ": " << hist.out;
        bool at_src = std::filesystem::exists(inbox / "a.txt");
        bool at_dst = std::filesystem::exists(cdir / "txt" / "a.txt");
        EXPECT_NE(at_src, at_dst) << point << ": file must be in exactly one place";
        auto where = at_dst ? (cdir / "txt" / "a.txt") : (inbox / "a.txt");
        std::string content;
        { // close the handle before undo: Windows refuses to
          // rename a file another handle holds open.
            std::ifstream in(where);
            content.assign((std::istreambuf_iterator<char>(in)),
                           std::istreambuf_iterator<char>());
        }
        EXPECT_EQ(content, "crash-data") << point;
        if (at_dst) { // moved + journaled => undo must restore it
            auto und = run_cli("undo --format=json --db=" + db, cdir);
            EXPECT_EQ(und.code, 0) << point << ": " << und.out;
            EXPECT_TRUE(std::filesystem::exists(inbox / "a.txt"))
                << point << ": hist=" << hist.out << " undo=" << und.out;
        }
        std::filesystem::current_path(original_cwd);
    }
}

TEST_F(CliFixture, UndoCrashRecoveryAtRenameCheckpoint) {
    // Same discipline inside undo: kill between undo's rename
    // and its journal mark. The next invocation reconciles the
    // 'undoing' record; the file stays restored and a second
    // undo must not move anything.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "undo-crash"; }
    auto db = (dir / "t.db").string();
    std::filesystem::current_path(dir);
    ASSERT_EQ(run_cli("organize --rule={ext}/{filename} --format=json --db=" + db +
                          " \"" + inbox.string() + "\"",
                      dir).code, 0);
    ASSERT_TRUE(std::filesystem::exists(dir / "txt" / "a.txt"));
    auto crash = run_cli_fault("undo --format=json --db=" + db, dir, "undo.after-rename");
    EXPECT_NE(crash.code, 0) << "fault point did not fire: " << crash.out;
    auto hist = run_cli("history --format=json --db=" + db, dir);
    ASSERT_EQ(hist.code, 0) << hist.out;
    EXPECT_EQ(hist.out.find("undoing"), std::string::npos) << hist.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "a.txt")) << hist.out;
    EXPECT_FALSE(std::filesystem::exists(dir / "txt" / "a.txt")) << hist.out;
    auto again = run_cli("undo --format=json --db=" + db, dir);
    EXPECT_TRUE(std::filesystem::exists(inbox / "a.txt")) << again.out;
    (void)again;
    std::filesystem::current_path(original_cwd);
}

TEST_F(CliFixture, UndoToleratesTwoSecondMtimeRounding) {
    // exFAT/FAT round mtimes to 2s (and DST shifts them), so the
    // mtime identity gate tolerates <=2s drift when the file ID
    // proves it is the same file. Forged-mtime replacement still
    // fails the file-ID gate (see Tamper tests); a +1s bump on
    // the untouched destination must not false-refuse.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "mtime-data"; }
    auto db = (dir / "t.db").string();
    std::filesystem::current_path(dir);
    ASSERT_EQ(run_cli("organize --rule={ext}/{filename} --format=json --db=" + db +
                          " \"" + inbox.string() + "\"",
                      dir).code, 0);
    auto dst = dir / "txt" / "a.txt";
    ASSERT_TRUE(std::filesystem::exists(dst));
    auto mt = std::filesystem::last_write_time(dst);
    std::filesystem::last_write_time(dst, mt + std::chrono::seconds(1));
    auto und = run_cli("undo --format=json --db=" + db, dir);
    EXPECT_EQ(und.code, 0) << und.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "a.txt")) << und.out;
    std::filesystem::current_path(original_cwd);
}

TEST_F(CliFixture, JsonOutputsCarrySchemaVersion) {
    // Dashboard contract (round-5/6, v2 in v6.5.93): every
    // object-shaped --format=json payload carries
    // "schema_version": 2 as its first field, so bridge shape
    // drift fails here instead of silently changing under the
    // dashboard.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    auto db = (dir / "t.db").string();
    const std::string cmds[] = {
        "stats --format=json", "organize --dry-run --format=json --rule={ext}/{filename}",
        "audit --format=json", "scrub --format=json", "undo --format=json",
        "export --format=json",
    };
    for (const auto& c : cmds) {
        auto r = run_cli(c + " --db=" + db + " \"" + inbox.string() + "\"", dir);
        EXPECT_TRUE(r.out.find("\"schema_version\": 2") != std::string::npos ||
                    r.out.find("\"schema_version\":2") != std::string::npos)
            << c << ": " << r.out;
    }
    auto sim = run_cli("similar --format=json --db=" + db + " \"" + (inbox / "a.txt").string() + "\"", dir);
    if (sim.code == 0) { // provider lanes only; others fail honestly first
        EXPECT_NE(sim.out.find("\"schema_version\": 2"), std::string::npos) << sim.out;
    }
    auto flow = run_cli("flow list --format=json --db=" + db, dir);
    EXPECT_NE(flow.out.find("\"schema_version\": 2"), std::string::npos) << flow.out;
    auto watch = run_cli("watch-status --format=json --db=" + db, dir);
    EXPECT_NE(watch.out.find("\"schema_version\":2"), std::string::npos) << watch.out;
}

TEST_F(CliFixture, JsonArrayRootsAreWrappedV2) {
    // Schema v2 (reviewer round 6): a bare JSON array root can
    // never gain a version field, so array-shaped payloads are
    // wrapped as {"schema_version": 2, "items": [...]}. Every
    // array command must follow; a leading '[' is a contract
    // break the dashboard cannot version-negotiate with.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    { std::ofstream(inbox / "b.txt") << "b"; }
    auto db = (dir / "t.db").string();
    const std::string cmds[] = {
        "scan --format=json", "duplicates --format=json", "hash --format=json",
        "metadata --format=json", "lint --format=json", "search --format=json a",
    };
    for (const auto& c : cmds) {
        auto r = run_cli(c + " --db=" + db + " \"" + inbox.string() + "\"", dir);
        ASSERT_EQ(r.code, 0) << c << ": " << r.out;
        auto first = r.out.find_first_not_of(" \t\r\n");
        ASSERT_NE(first, std::string::npos) << c;
        EXPECT_EQ(r.out[first], '{') << c << ": " << r.out;
        EXPECT_NE(r.out.find("\"schema_version\": 2"), std::string::npos) << c << ": " << r.out;
        EXPECT_NE(r.out.find("\"items\": ["), std::string::npos) << c << ": " << r.out;
    }
    // Path-free array commands follow the same wrapped contract.
    for (const auto& c : {"history --format=json", "ignore --format=json"}) {
        auto r = run_cli(std::string(c) + " --db=" + db, dir);
        ASSERT_EQ(r.code, 0) << c << ": " << r.out;
        auto first = r.out.find_first_not_of(" \t\r\n");
        ASSERT_NE(first, std::string::npos) << c;
        EXPECT_EQ(r.out[first], '{') << c << ": " << r.out;
        EXPECT_NE(r.out.find("\"schema_version\": 2"), std::string::npos) << c << ": " << r.out;
        EXPECT_NE(r.out.find("\"items\": ["), std::string::npos) << c << ": " << r.out;
    }
}

TEST_F(CliFixture, MissingRootFailsEveryPathCommand) {
    // Conformance (per Muse's roast): the missing-path contract
    // — nonzero exit naming the path — must hold for every
    // path-consuming command, not just the ones a bug report
    // happened to name.
    auto missing = (dir / "no-such-place").string();
    auto db = (dir / "t.db").string();
    for (const auto& c : kMissingRootCmds) {
        auto r = run_cli(c + " --db=" + db + " \"" + missing + "\"", dir);
        EXPECT_NE(r.code, 0) << c << ": " << r.out;
        EXPECT_NE(r.out.find("does not exist"), std::string::npos) << c << ": " << r.out;
    }
}

TEST_F(CliFixture, HelpCoversEveryPathCommand) {
    // Completeness guard for the matrix above: parse --help and
    // require every listed command to be either a matrix row or a
    // named exemption (non-path grammar: subcommands, queries,
    // daemons). A new path-consuming command fails this test until
    // it joins the matrix.
    auto help = run_cli("--help", dir);
    ASSERT_EQ(help.code, 0) << help.out;
    const std::string exempt[] = {
        "undo", "history", "audit", "ignore", "watch-status",
        "flow", "clerk", "oracle", "autodev", "sync",
        "vault-init", "vault-lock", "vault-unlock", "vault-list",
        "watch", // covered by WatchStatus tests; daemon blocks
    };
    auto in = [](const std::string& s, const std::string* arr, size_t n) {
        for (size_t i = 0; i < n; ++i) if (arr[i] == s || arr[i].rfind(s + " ", 0) == 0) return true;
        return false;
    };
    bool in_commands = false;
    std::istringstream lines(help.out);
    std::string line;
    int checked = 0;
    while (std::getline(lines, line)) {
        if (line.rfind("Commands:", 0) == 0) { in_commands = true; continue; }
        if (in_commands && line.empty()) break;
        if (!in_commands) continue;
        std::istringstream ls(line);
        std::string name;
        if (!(ls >> name)) continue;
        ++checked;
        EXPECT_TRUE(in(name, kMissingRootCmds, std::size(kMissingRootCmds)) ||
                    in(name, exempt, std::size(exempt)))
            << "command '" << name << "' has no missing-root matrix row and is not exempt";
    }
    EXPECT_GT(checked, 20);
}

#ifdef _WIN32
TEST_F(CliFixture, ScanFindsCjkRootPassedAsArgument) {
    // Muse review: a CJK root passed as an argument arrived at
    // the CRT narrow main as "????" (ANSI code page), so the real
    // shipped binary rejected a directory that exists. Arguments
    // must be sourced from the wide command line as UTF-8.
    auto root = dir / L"\x4e2d\x6587\x76ee\x5f55";
    std::filesystem::create_directories(root);
    { std::ofstream(root / "inner.txt") << "inner"; }
    auto r = run_cli_wide(L"scan --db=" + (dir / "t.db").wstring() +
                          L" \"" + root.wstring() + L"\"", dir);
    ASSERT_EQ(r.code, 0) << r.out;
    EXPECT_NE(r.out.find("inner.txt"), std::string::npos) << r.out;
}
#endif

TEST_F(CliFixture, OrganizeUndoPreservesTreeExactly) {
    // The data-loss invariant: organize followed by undo must
    // restore the exact tree — same paths, same bytes — including
    // nested dirs and a non-ASCII filename.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox / "sub");
    { std::ofstream(inbox / "a.txt") << "alpha"; }
    { std::ofstream(inbox / "sub" / "b.log") << "bravo"; }
    { std::ofstream(inbox / L"t\xebst-\xfc.txt") << "unicode"; }
    // CJK is outside cp1252: if the path plumbing ANSI-mangles
    // names, this file's round-trip breaks visibly (Muse review).
    { std::ofstream(inbox / L"\x4e2d\x6587\x6587\x4ef6.txt") << "cjk"; }
    // And a CJK directory on the path itself, not just a filename.
    std::filesystem::create_directories(inbox / L"\x4e2d\x6587\x76ee\x5f55");
    { std::ofstream(inbox / L"\x4e2d\x6587\x76ee\x5f55" / "inner.txt") << "inner"; }
    auto db = (dir / "t.db").string();

    auto snapshot = [&]() {
        // Contents AND mtimes: an undo that restores bytes but
        // rewrites timestamps would be a quiet lie (Muse roast).
        // Wide keys: generic_string() would ANSI-mangle (and throw
        // on) the CJK fixture name before the product even runs.
        // The whole fixture dir is compared (inbox + organize
        // target area), including the directory set: undo must
        // not leave organize's empty category dirs behind.
        struct Snap {
            std::map<std::wstring, std::pair<std::string, std::filesystem::file_time_type>> files;
            std::set<std::wstring> dirs;
            bool operator==(const Snap& o) const { return files == o.files && dirs == o.dirs; }
        };
        Snap snap;
        for (auto& e : std::filesystem::recursive_directory_iterator(dir)) {
            auto rel = std::filesystem::relative(e.path(), dir).generic_wstring();
            if (rel.find(L"t.db") == 0 || rel.find(L"cli-out") == 0) continue;
            if (e.is_directory()) {
                snap.dirs.insert(rel);
            } else if (e.is_regular_file()) {
                std::ifstream in(e.path(), std::ios::binary);
                std::stringstream ss; ss << in.rdbuf();
                snap.files[rel] = {ss.str(), e.last_write_time()};
            }
        }
        return snap;
    };
    auto before = snapshot();
    ASSERT_EQ(before.files.size(), 5u);

    std::filesystem::current_path(dir);
    auto org = run_cli("organize --rule={ext}/{filename} --db=" + db + " \"" + inbox.string() + "\"", dir);
    std::filesystem::current_path(original_cwd);
    ASSERT_EQ(org.code, 0) << org.out;
    ASSERT_FALSE(std::filesystem::exists(inbox / "a.txt")) << org.out;

    auto undo = run_cli("undo --db=" + db, dir);
    ASSERT_EQ(undo.code, 0) << undo.out;
    EXPECT_EQ(snapshot(), before);
}

TEST_F(CliFixture, SearchFlagsBeforeQueryAndBareQueryWork) {
    // Two search-grammar breaks: (1) `--content` placed before
    // the query was never parsed (the flag scan started after the
    // query slot), silently degrading to a filename search;
    // (2) a bare `search <query>` (search the current directory)
    // was rejected by the missing-path guard, which mistook the
    // query for a path.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "note.txt") << "a needle here"; }
    auto db = (dir / "t.db").string();

    auto flagged = run_cli("search --content needle --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(flagged.code, 0) << flagged.out;
    EXPECT_NE(flagged.out.find("note.txt"), std::string::npos) << flagged.out;

    std::filesystem::current_path(inbox);
    auto bare = run_cli("search needle --content --db=" + db, dir);
    std::filesystem::current_path(original_cwd);
    ASSERT_EQ(bare.code, 0) << bare.out;
    EXPECT_NE(bare.out.find("note.txt"), std::string::npos) << bare.out;
}

TEST_F(CliFixture, UndoAfterDeleteDuplicatesRefusesThenDismisses) {
    // Muse review: after delete-duplicates, undo used to
    // silently reverse the previous command. It must refuse the
    // irreversible delete batch, and only `undo --dismiss` may
    // unblock the older batch.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "same"; }
    { std::ofstream(inbox / "b.txt") << "same"; }
    { std::ofstream(inbox / "c.txt") << "unique"; }
    auto db = (dir / "t.db").string();

    auto ren = run_cli("rename --pattern=\"{name}_v2.{ext}\" --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(ren.code, 0) << ren.out;
    ASSERT_TRUE(std::filesystem::exists(inbox / "a_v2.txt"));

    // delete-duplicates acts on groups persisted by `duplicates`.
    ASSERT_EQ(run_cli("duplicates --db=" + db + " \"" + inbox.string() + "\"", dir).code, 0);
    auto del = run_cli("delete-duplicates --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(del.code, 0) << del.out;
    ASSERT_NE(del.out.find("Deleted 1"), std::string::npos) << del.out;

    auto undo = run_cli("undo --db=" + db, dir);
    EXPECT_NE(undo.code, 0) << undo.out;
    EXPECT_NE(undo.out.find("cannot be undone"), std::string::npos) << undo.out;
    // The older rename batch must NOT have been reversed (c was
    // unique, so c_v2.txt is certainly still there under its
    // renamed name).
    EXPECT_TRUE(std::filesystem::exists(inbox / "c_v2.txt")) << undo.out;
    EXPECT_FALSE(std::filesystem::exists(inbox / "c.txt"));

    auto dismiss = run_cli("undo --dismiss --db=" + db, dir);
    ASSERT_EQ(dismiss.code, 0) << dismiss.out;

    auto undo2 = run_cli("undo --db=" + db, dir);
    ASSERT_EQ(undo2.code, 0) << undo2.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "c.txt")) << undo2.out;
}

TEST_F(CliFixture, UndoReportsSkippedRecordsWithReasons) {
    // Muse review: partial undo was silent. A batch member
    // whose destination vanished must be reported as skipped
    // with a reason while the rest of the batch restores.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    { std::ofstream(inbox / "b.txt") << "b"; }
    auto db = (dir / "t.db").string();

    auto ren = run_cli("rename --pattern=\"{name}_v2.{ext}\" --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(ren.code, 0) << ren.out;
    std::filesystem::remove(inbox / "b_v2.txt");

    auto undo = run_cli("undo --db=" + db, dir);
    ASSERT_EQ(undo.code, 0) << undo.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "a.txt")) << undo.out;
    EXPECT_NE(undo.out.find("Skipped"), std::string::npos) << undo.out;
    EXPECT_NE(undo.out.find("no longer exists"), std::string::npos) << undo.out;
}

TEST_F(CliFixture, UndoRestoresWholeRenameBatch) {
    // One rename command touching two files used to need two
    // undo invocations (per-record undo). A single undo must
    // restore the whole command's batch.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    { std::ofstream(inbox / "b.txt") << "b"; }
    auto db = (dir / "t.db").string();

    auto ren = run_cli("rename --pattern=\"{name}_v2.{ext}\" --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(ren.code, 0) << ren.out;
    ASSERT_TRUE(std::filesystem::exists(inbox / "a_v2.txt"));

    auto undo = run_cli("undo --db=" + db, dir);
    ASSERT_EQ(undo.code, 0) << undo.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "a.txt")) << undo.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "b.txt")) << undo.out;
    EXPECT_FALSE(std::filesystem::exists(inbox / "a_v2.txt"));
    EXPECT_FALSE(std::filesystem::exists(inbox / "b_v2.txt"));
}

TEST_F(CliFixture, ConvertFailureExitsNonzero) {
    // A failed conversion (backend missing, or input it cannot
    // read) printed "Conversion failed" yet exited 0, so bridges
    // reported HTTP 200. Failures must be counted and exit 1,
    // with successes still reported per file.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "not media at all"; }
    auto db = (dir / "t.db").string();
    auto r = run_cli("convert --to=mp4 --db=" + db + " \"" + (inbox / "a.txt").string() + "\"", dir);
    EXPECT_NE(r.code, 0) << r.out;
    EXPECT_NE(r.out.find("Conversion failed"), std::string::npos) << r.out;
    EXPECT_NE(r.out.find("failed"), std::string::npos) << r.out;
}

TEST_F(CliFixture, AfingerprintProcessesEveryInput) {
    // vhash/afingerprint used to process only the first path and
    // silently drop the rest, though the dashboard cards promise
    // comma-separated files. Every input must be processed, and
    // unfingerprintable ones must be named and counted.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    write_tone_wav(inbox / "one.wav");
    write_tone_wav(inbox / "two.wav");
    { std::ofstream(inbox / "bad.txt") << "not audio"; }
    auto db = (dir / "t.db").string();

    auto mixed = run_cli("afingerprint --db=" + db + " \"" + (inbox / "one.wav").string() +
                         "\" \"" + (inbox / "bad.txt").string() + "\" \"" +
                         (inbox / "two.wav").string() + "\"", dir);
    if (mixed.out.find("not found") != std::string::npos &&
        mixed.out.find("Audio:") == std::string::npos) {
        GTEST_SKIP() << "audio fingerprinter not registered in this lane";
    }
    EXPECT_NE(mixed.code, 0) << mixed.out;
    EXPECT_NE(mixed.out.find("one.wav"), std::string::npos) << mixed.out;
    EXPECT_NE(mixed.out.find("two.wav"), std::string::npos) << mixed.out;
    EXPECT_NE(mixed.out.find("bad.txt"), std::string::npos) << mixed.out;

    auto good = run_cli("afingerprint --db=" + db + " \"" + (inbox / "one.wav").string() +
                        "\" \"" + (inbox / "two.wav").string() + "\"", dir);
    ASSERT_EQ(good.code, 0) << good.out;
    EXPECT_NE(good.out.find("one.wav"), std::string::npos) << good.out;
    EXPECT_NE(good.out.find("two.wav"), std::string::npos) << good.out;
}

TEST_F(CliFixture, SimilarRejectsNonDhashAlgorithm) {
    // The stored index only ever holds 'dhash' values (written by
    // `hash --hasher=dhash`), but `similar --phash=phash` used to
    // compare a phash against them and confidently return the
    // perceptual opposite as a match. Non-dhash queries must be
    // refused with the reason.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    write_gradient_bmp(inbox / "imgA.bmp", 0);
    auto db = (dir / "t.db").string();
    auto r = run_cli("similar --phash=phash --db=" + db + " \"" + (inbox / "imgA.bmp").string() + "\"", dir);
    EXPECT_NE(r.code, 0) << r.out;
    EXPECT_TRUE(r.out.find("dhash") != std::string::npos ||
                r.out.find("not found") != std::string::npos)
        << r.out;
}

TEST_F(CliFixture, SimilarFindsNearIdenticalImages) {
    // Positive path for the similar-image workflow: dhash the set,
    // then `similar` on one member must return its near-identical
    // sibling and not the reversed-gradient opposite.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    write_gradient_bmp(inbox / "imgA.bmp", 0);
    write_gradient_bmp(inbox / "imgB.bmp", 1);
    write_gradient_bmp(inbox / "imgC.bmp", 2);
    auto db = (dir / "t.db").string();

    auto hashed = run_cli("hash --hasher=dhash --db=" + db + " \"" + inbox.string() + "\"", dir);
    if (hashed.out.find("hasher not found") != std::string::npos) {
        GTEST_SKIP() << "dhash hasher not registered in this lane";
    }
    ASSERT_EQ(hashed.code, 0) << hashed.out;

    auto sim = run_cli("similar --db=" + db + " \"" + (inbox / "imgA.bmp").string() + "\"", dir);
    if (sim.out.find("Perceptual hasher") != std::string::npos &&
        sim.out.find("not found") != std::string::npos) {
        GTEST_SKIP() << "perceptual hasher not registered in this lane";
    }
    ASSERT_EQ(sim.code, 0) << sim.out;
    EXPECT_NE(sim.out.find("imgB.bmp"), std::string::npos) << sim.out;
    EXPECT_EQ(sim.out.find("imgC.bmp"), std::string::npos) << sim.out;
}

TEST_F(CliFixture, HashReportsFilesItCouldNotHash) {
    // Hashers return "" on failure (dhash on a non-image, fast64 on
    // an unreadable file); the hash command used to print and store
    // those blanks and exit 0. Failures must be named and counted.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "definitely not an image"; }
    auto db = (dir / "t.db").string();

    auto bad = run_cli("hash --hasher=dhash --db=" + db + " \"" + inbox.string() + "\"", dir);
    EXPECT_NE(bad.code, 0) << bad.out;
    EXPECT_TRUE(bad.out.find("failed to hash") != std::string::npos ||
                bad.out.find("hasher not found") != std::string::npos)
        << bad.out;

    auto good = run_cli("hash --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(good.code, 0) << good.out;
    EXPECT_NE(good.out.find("a.txt"), std::string::npos) << good.out;
}

TEST_F(CliFixture, GlobalFlagsDoNotHijackPositionalArgs) {
    // Handlers that index raw argv took the first token after the
    // subcommand literally: `flow run flow-01 --db=x payload`
    // "executed" the workflow on the string '--db=x', and
    // `search --db=x query` searched for the flag. Positional
    // parsing must skip --flag tokens anywhere in the line.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    auto db = (dir / "t.db").string();
    auto payload = inbox.string();

    auto flow = run_cli("flow run flow-01 --db=" + db + " \"" + payload + "\"", dir);
    ASSERT_EQ(flow.code, 0) << flow.out;
    EXPECT_NE(flow.out.find(payload), std::string::npos) << flow.out;
    EXPECT_EQ(flow.out.find("--db="), std::string::npos) << flow.out;

    auto search = run_cli("search --db=" + db + " a.txt \"" + payload + "\"", dir);
    ASSERT_EQ(search.code, 0) << search.out;
    EXPECT_NE(search.out.find("a.txt"), std::string::npos) << search.out;
}

TEST_F(CliFixture, SimilarOnDirectoryIsACleanError) {
    // `similar` takes one image file; handing it a folder used to
    // surface raw OpenCV warnings plus "Failed to compute hash".
    // It must say plainly that a file is expected.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    auto db = (dir / "t.db").string();
    auto r = run_cli("similar --db=" + db + " \"" + inbox.string() + "\"", dir);
    EXPECT_NE(r.code, 0) << r.out;
    // Provider lanes name the file expectation; lanes without a
    // perceptual hasher report the missing provider instead.
    EXPECT_TRUE(r.out.find("not a") != std::string::npos ||
                r.out.find("not found") != std::string::npos)
        << r.out;
}

TEST_F(CliFixture, OcrUnavailableIsAnError) {
    // OCR with no loadable tessdata used to print Tesseract's own
    // errors per file yet exit 0. Unavailability must be a clean
    // CLI error (provider missing in some lanes is also accepted).
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    auto db = (dir / "t.db").string();
    auto r = run_cli("ocr --db=" + db + " \"" + inbox.string() + "\"", dir);
    EXPECT_NE(r.code, 0) << r.out;
    EXPECT_NE(r.out.find("tesseract"), std::string::npos) << r.out;
}

TEST_F(CliFixture, ClassifyWithoutModelIsAnError) {
    // Without model.onnx in the working directory, classify used
    // to print "Model file not found" per file yet exit 0. The CLI
    // must surface the unavailable classifier as a clean error.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    auto db = (dir / "t.db").string();
    auto r = run_cli("classify --db=" + db + " \"" + inbox.string() + "\"", dir);
    EXPECT_NE(r.code, 0) << r.out;
    // Provider lanes name the missing model; lanes built without
    // ONNX report the missing classifier instead. Both are honest.
    EXPECT_TRUE(r.out.find("Model file not found") != std::string::npos ||
                r.out.find("Classifier 'onnx' not found") != std::string::npos)
        << r.out;
}

TEST_F(CliFixture, BogusHasherAndKeepAreErrorsNotCrashes) {
    // `hash --hasher=bogus` used to dereference the null hasher and
    // crash (0xC0000005); `--keep=bogus` was silently treated as
    // "oldest". Both must be clean, named errors.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    auto db = (dir / "t.db").string();

    auto hash = run_cli("hash --hasher=bogus --db=" + db + " \"" + inbox.string() + "\"", dir);
    EXPECT_NE(hash.code, 0) << hash.out;
    EXPECT_NE(hash.out.find("hasher not found"), std::string::npos) << hash.out;

    auto keep = run_cli("duplicates --keep=bogus --db=" + db + " \"" + inbox.string() + "\"", dir);
    EXPECT_NE(keep.code, 0) << keep.out;
    EXPECT_NE(keep.out.find("keep"), std::string::npos) << keep.out;
}

TEST_F(CliFixture, MissingRootIsAnErrorNotEmptySuccess) {
    // A nonexistent path used to produce empty output and exit 0
    // (the std scanner silently skips missing roots), so scripts
    // could not tell "no files" from "wrong path".
    auto db = (dir / "t.db").string();
    auto missing = (dir / "no-such-dir").string();

    auto scan = run_cli("scan --db=" + db + " \"" + missing + "\"", dir);
    EXPECT_NE(scan.code, 0) << scan.out;
    EXPECT_NE(scan.out.find("does not exist"), std::string::npos) << scan.out;

    auto hash = run_cli("hash --db=" + db + " \"" + missing + "\"", dir);
    EXPECT_NE(hash.code, 0) << hash.out;

    // Mixed roots: warn about the missing one, still process the rest.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "keep.txt") << "k"; }
    auto mixed = run_cli("scan --db=" + db + " \"" + inbox.string() + "\" \"" + missing + "\"", dir);
    EXPECT_EQ(mixed.code, 0) << mixed.out;
    EXPECT_NE(mixed.out.find("keep.txt"), std::string::npos) << mixed.out;
    EXPECT_NE(mixed.out.find("does not exist"), std::string::npos) << mixed.out;

    // Unknown export formats must not silently fall back to JSON.
    auto bogus = run_cli("export --format=bogus --db=" + db + " \"" + inbox.string() + "\"", dir);
    EXPECT_NE(bogus.code, 0) << bogus.out;
}

TEST_F(CliFixture, PlainScanDoesNotStartScaffoldServices) {
    // Engine used to construct OmniClerk/AutoDev/Sync/Swarm in its
    // constructor, so every command (even a plain scan) printed
    // "[OmniClerk] Initializing..." banners and registered a swarm
    // peer. Those services must start lazily, only when used.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "a"; }
    auto db = (dir / "t.db").string();
    auto scan = run_cli("scan --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(scan.code, 0) << scan.out;
    EXPECT_EQ(scan.out.find("[OmniClerk]"), std::string::npos) << scan.out;
    EXPECT_EQ(scan.out.find("[AutoDev]"), std::string::npos) << scan.out;
    EXPECT_EQ(scan.out.find("[SyncService]"), std::string::npos) << scan.out;
    EXPECT_EQ(scan.out.find("[SwarmHub]"), std::string::npos) << scan.out;
}

TEST_F(CliFixture, IgnoreGlobFiltersScan) {
    // Ignore patterns are globs ("*.log") as every UI teaches, but
    // is_ignored treated them as regex: "*.log" is an invalid regex
    // and fell back to substring search, so it never matched.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "keep.txt") << "k"; }
    { std::ofstream(inbox / "skip.log") << "l"; }
    auto db = (dir / "t.db").string();

    ASSERT_EQ(run_cli("ignore add \"*.log\" testreason --db=" + db, dir).code, 0);
    auto scan = run_cli("scan --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(scan.code, 0) << scan.out;
    EXPECT_NE(scan.out.find("keep.txt"), std::string::npos) << scan.out;
    EXPECT_EQ(scan.out.find("skip.log"), std::string::npos) << scan.out;
}

TEST_F(CliFixture, UndoWorksFromDifferentCwd) {
    // Organize destinations were journaled CWD-relative, so undo
    // only worked when run from the same directory as the organize.
    // Destinations are now recorded absolute; undo works anywhere.
    auto inbox = dir / "inbox";
    auto elsewhere = dir / "elsewhere";
    std::filesystem::create_directories(inbox);
    std::filesystem::create_directories(elsewhere);
    { std::ofstream(inbox / "photo.txt") << "organize me"; }
    auto db = (dir / "t.db").string();

    std::filesystem::current_path(dir);
    ASSERT_EQ(run_cli("organize --rule={extension}/{name} --db=" + db + " \"" + inbox.string() + "\"", dir).code, 0);
    ASSERT_FALSE(std::filesystem::exists(inbox / "photo.txt"));

    // run_cli inherits this process's CWD, so really move elsewhere.
    std::filesystem::current_path(elsewhere);
    auto undo = run_cli("undo --db=" + db, elsewhere);
    ASSERT_EQ(undo.code, 0) << undo.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "photo.txt")) << undo.out;
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

TEST_F(CliFixture, RenameReportsFailureWhenDestDirMissing) {
    // The destination directory does not exist and rename does not
    // create it: the rename fails, so the run must exit nonzero and
    // leave the file in place. It used to exit 0 and list the rename
    // as if it had happened.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "old.txt") << "rename me"; }
    auto db = (dir / "t.db").string();

    std::filesystem::current_path(dir);
    auto r = run_cli("rename --pattern=no-such-dir/{name} --db=" + db + " \"" + inbox.string() + "\"", dir);
    EXPECT_NE(r.code, 0) << r.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "old.txt")) << r.out;
}

TEST_F(CliFixture, ScanJsonIncludesMtime) {
    // The classic dashboard's date column reads f.mtime from scan
    // JSON; the field was missing, rendering "Invalid Date".
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "x"; }
    auto db = (dir / "t.db").string();

    auto r = run_cli("scan --format=json --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(r.code, 0) << r.out;
    EXPECT_NE(r.out.find("\"mtime\":"), std::string::npos) << r.out;
}

TEST_F(CliFixture, SearchWholeWordContentRespected) {
    // --whole-word was silently ignored for literal content search
    // (only the regex path honored it): "wor" must not match "word".
    auto inbox = dir / "docs";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "the word is here"; }
    auto db = (dir / "t.db").string();

    auto partial = run_cli("search wor --content --whole-word --db=" + db +
                           " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(partial.code, 0) << partial.out;
    EXPECT_NE(partial.out.find("file(s) found"), std::string::npos) << partial.out;
    EXPECT_EQ(partial.out.find("a.txt"), std::string::npos) << partial.out;

    auto whole = run_cli("search word --content --whole-word --db=" + db +
                         " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(whole.code, 0) << whole.out;
    EXPECT_NE(whole.out.find("a.txt"), std::string::npos) << whole.out;
}

TEST_F(CliFixture, RenameBatchReportsFailureOnCollision) {
    // Renaming onto an existing filename would silently overwrite it
    // (MSVC rename replaces); the batch must refuse and exit nonzero.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "IMG_1.txt") << "x"; }
    { std::ofstream(inbox / "Photo_1.txt") << "y"; }
    auto db = (dir / "t.db").string();

    auto r = run_cli("rename-batch --find=IMG_ --replace=Photo_ --db=" + db +
                     " \"" + inbox.string() + "\"", dir);
    EXPECT_NE(r.code, 0) << r.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "IMG_1.txt")) << r.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "Photo_1.txt")) << r.out;
}

TEST_F(CliFixture, DeleteDuplicatesRechecksContentBeforeDeleting) {
    // Stale-authorization guard: if a "duplicate" changes content
    // after the duplicates run, delete-duplicates must not delete it
    // based on the obsolete group. It re-hashes against the kept
    // file at delete time.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "a.txt") << "same-content"; }
    { std::ofstream(inbox / "b.txt") << "same-content"; }
    auto db = (dir / "t.db").string();

    ASSERT_EQ(run_cli("duplicates --db=" + db + " \"" + inbox.string() + "\"", dir).code, 0);
    // b.txt stops being a duplicate (same length, different bytes).
    { std::ofstream(inbox / "b.txt") << "diff-content"; }

    auto del = run_cli("delete-duplicates --format=json --db=" + db + " \"" + inbox.string() + "\"", dir);
    ASSERT_EQ(del.code, 0) << del.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "a.txt")) << del.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "b.txt")) << del.out;
    EXPECT_NE(del.out.find("\"skipped\": 1"), std::string::npos) << del.out;
}

TEST_F(CliFixture, UndoDoesNotOverwriteRecreatedSource) {
    // External drift during recovery: if the original path was
    // recreated after the move, undo must not rename over it
    // (MSVC rename replaces), destroying the new file.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox);
    { std::ofstream(inbox / "photo.txt") << "original"; }
    auto db = (dir / "t.db").string();

    std::filesystem::current_path(dir);
    ASSERT_EQ(run_cli("organize --rule={extension}/{name} --db=" + db + " \"" + inbox.string() + "\"", dir).code, 0);
    ASSERT_FALSE(std::filesystem::exists(inbox / "photo.txt"));
    { std::ofstream(inbox / "photo.txt") << "recreated-externally"; }

    auto undo = run_cli("undo --db=" + db, dir);
    ASSERT_EQ(undo.code, 0) << undo.out;
    std::ifstream in(inbox / "photo.txt");
    std::stringstream ss;
    ss << in.rdbuf();
    EXPECT_EQ(ss.str(), "recreated-externally") << undo.out;
    // The moved original is still at its destination, not lost.
    EXPECT_TRUE(std::filesystem::exists(dir / "txt" / "photo")) << undo.out;
}

TEST_F(CliFixture, DeleteDuplicatesScopedToGivenRoot) {
    // delete-duplicates must act only inside the folder it is given.
    // Groups in the DB come from the last `duplicates` run (here:
    // folder A); pointing delete-duplicates at folder B must not
    // touch A's files. It used to ignore its path argument entirely.
    auto dir_a = dir / "a";
    auto dir_b = dir / "b";
    std::filesystem::create_directories(dir_a);
    std::filesystem::create_directories(dir_b);
    { std::ofstream(dir_a / "x.txt") << "same-content"; }
    { std::ofstream(dir_a / "y.txt") << "same-content"; }
    { std::ofstream(dir_b / "x.txt") << "same-content"; }
    { std::ofstream(dir_b / "y.txt") << "same-content"; }
    auto db = (dir / "t.db").string();

    ASSERT_EQ(run_cli("duplicates --db=" + db + " \"" + dir_a.string() + "\"", dir).code, 0);
    auto del = run_cli("delete-duplicates --db=" + db + " \"" + dir_b.string() + "\"", dir);
    ASSERT_EQ(del.code, 0) << del.out;
    // A's pair is out of scope and must be untouched.
    EXPECT_TRUE(std::filesystem::exists(dir_a / "x.txt")) << del.out;
    EXPECT_TRUE(std::filesystem::exists(dir_a / "y.txt")) << del.out;
}

#ifdef FO_HAVE_OPENSSL
TEST_F(CliFixture, VaultWrongPasswordLeavesNoOutput) {
    // A failed decrypt must not leave a bogus "restored" file behind:
    // decrypt_file streamed unauthenticated plaintext to the dest
    // before the GCM tag check failed (wrong password).
    auto vault = (dir / "vault").string();
    auto src = dir / "secret.txt";
    auto dst = dir / "restored.txt";
    { std::ofstream(src) << "vault wrong-pass payload"; }

    ASSERT_EQ(run_cli("vault-init " + vault + " right-pass", dir).code, 0);
    ASSERT_EQ(run_cli("vault-lock " + vault + " right-pass \"" + src.string() + "\"", dir).code, 0);
    auto list = run_cli("vault-list " + vault + " right-pass", dir);
    ASSERT_EQ(list.code, 0) << list.out;
    auto pos = list.out.find("vault_");
    ASSERT_NE(pos, std::string::npos) << list.out;
    auto id = list.out.substr(pos, list.out.find(' ', pos) - pos);

    auto bad = run_cli("vault-unlock " + vault + " WRONG-pass " + id + " \"" + dst.string() + "\"", dir);
    EXPECT_NE(bad.code, 0) << bad.out;
    EXPECT_FALSE(std::filesystem::exists(dst)) << bad.out;

    // The entry must still unlock with the right password afterward.
    ASSERT_EQ(run_cli("vault-unlock " + vault + " right-pass " + id + " \"" + dst.string() + "\"", dir).code, 0);
    std::ifstream in(dst);
    std::stringstream ss;
    ss << in.rdbuf();
    EXPECT_EQ(ss.str(), "vault wrong-pass payload");
}

TEST_F(CliFixture, VaultUnlockRefusesExistingDestination) {
    // Unlocking onto an existing file must refuse rather than
    // silently replace its bytes, and the vault entry must survive.
    auto vault = (dir / "vault").string();
    auto src = dir / "secret.txt";
    auto dst = dir / "restored.txt";
    { std::ofstream(src) << "vault payload"; }
    { std::ofstream(dst) << "pre-existing precious bytes"; }

    ASSERT_EQ(run_cli("vault-init " + vault + " right-pass", dir).code, 0);
    ASSERT_EQ(run_cli("vault-lock " + vault + " right-pass \"" + src.string() + "\"", dir).code, 0);
    auto list = run_cli("vault-list " + vault + " right-pass", dir);
    ASSERT_EQ(list.code, 0) << list.out;
    auto pos = list.out.find("vault_");
    ASSERT_NE(pos, std::string::npos) << list.out;
    auto id = list.out.substr(pos, list.out.find(' ', pos) - pos);

    auto bad = run_cli("vault-unlock " + vault + " right-pass " + id + " \"" + dst.string() + "\"", dir);
    EXPECT_NE(bad.code, 0) << bad.out;
    std::ifstream in(dst);
    std::stringstream ss;
    ss << in.rdbuf();
    EXPECT_EQ(ss.str(), "pre-existing precious bytes") << bad.out;
    // Entry still in the vault afterward.
    auto list2 = run_cli("vault-list " + vault + " right-pass", dir);
    EXPECT_NE(list2.out.find(id), std::string::npos) << list2.out;
}

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
