// CLI-level integration: drives the real fo_cli.exe built beside the
// tests (build-*/cli/fo_cli.exe) for behaviors that only exist at the
// command surface: audit --limit validation and the vault round-trip.
#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <map>
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
    // A journaled Delete (not reversible) must not block undoing an
    // earlier Move: undo_last used to pick the Delete, fail silently,
    // and report "No operations to undo" forever.
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
    ASSERT_EQ(undo.code, 0) << undo.out;
    EXPECT_TRUE(std::filesystem::exists(inbox / "photo.txt")) << undo.out;
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

TEST_F(CliFixture, OrganizeUndoPreservesTreeExactly) {
    // The data-loss invariant: organize followed by undo must
    // restore the exact tree — same paths, same bytes — including
    // nested dirs and a non-ASCII filename.
    auto inbox = dir / "inbox";
    std::filesystem::create_directories(inbox / "sub");
    { std::ofstream(inbox / "a.txt") << "alpha"; }
    { std::ofstream(inbox / "sub" / "b.log") << "bravo"; }
    { std::ofstream(inbox / L"t\xebst-\xfc.txt") << "unicode"; }
    auto db = (dir / "t.db").string();

    auto snapshot = [&]() {
        // Contents AND mtimes: an undo that restores bytes but
        // rewrites timestamps would be a quiet lie (Muse roast).
        std::map<std::string, std::pair<std::string, std::filesystem::file_time_type>> files;
        for (auto& e : std::filesystem::recursive_directory_iterator(inbox)) {
            if (!e.is_regular_file()) continue;
            std::ifstream in(e.path(), std::ios::binary);
            std::stringstream ss; ss << in.rdbuf();
            files[std::filesystem::relative(e.path(), inbox).generic_string()] =
                {ss.str(), e.last_write_time()};
        }
        return files;
    };
    auto before = snapshot();
    ASSERT_EQ(before.size(), 3u);

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
