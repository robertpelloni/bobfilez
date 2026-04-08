#include <benchmark/benchmark.h>
#include "fo/core/registry.hpp"
#include "fo/core/interfaces.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

class ScannerFixture : public benchmark::Fixture {
public:
    fs::path test_dir;

    void SetUp(const ::benchmark::State&) override {
        // Ensure providers are registered!
        fo::core::register_all_providers();

        test_dir = fs::temp_directory_path() / "fo_bench_scanner";
        if (fs::exists(test_dir)) fs::remove_all(test_dir);
        fs::create_directories(test_dir);
        
        // Create 100 files in root
        for (int i = 0; i < 100; ++i) {
            std::ofstream(test_dir / ("file_" + std::to_string(i) + ".txt")) << "content";
        }
        // Create 10 subdirs with 10 files each
        for (int d = 0; d < 10; ++d) {
            fs::path sub = test_dir / ("subdir_" + std::to_string(d));
            fs::create_directories(sub);
            for (int i = 0; i < 10; ++i) {
                std::ofstream(sub / ("file_" + std::to_string(i) + ".txt")) << "content";
            }
        }
    }

    void TearDown(const ::benchmark::State&) override {
        fs::remove_all(test_dir);
    }
};

BENCHMARK_DEFINE_F(ScannerFixture, ScanStd)(benchmark::State& state) {
    auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
    if (!scanner) { state.SkipWithError("std scanner not found"); return; }
    
    std::vector<fs::path> roots = {test_dir};
    std::vector<std::string> exts; // all
    
    for (auto _ : state) {
        auto files = scanner->scan(roots, exts, false);
        benchmark::DoNotOptimize(files);
    }
}
BENCHMARK_REGISTER_F(ScannerFixture, ScanStd);

#ifdef _WIN32
BENCHMARK_DEFINE_F(ScannerFixture, ScanWin32)(benchmark::State& state) {
    auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("win32");
    if (!scanner) { state.SkipWithError("win32 scanner not found"); return; }
    
    std::vector<fs::path> roots = {test_dir};
    std::vector<std::string> exts; // all
    
    for (auto _ : state) {
        auto files = scanner->scan(roots, exts, false);
        benchmark::DoNotOptimize(files);
    }
}
BENCHMARK_REGISTER_F(ScannerFixture, ScanWin32);
#endif

static void BM_Hasher_Fast64(benchmark::State& state) {
    fs::path path = fs::temp_directory_path() / "fo_bench_hash.tmp";
    {
        std::ofstream ofs(path, std::ios::binary);
        std::vector<char> data(1024 * 1024, 'a'); // 1MB
        ofs.write(data.data(), data.size());
    }
    
    auto hasher = fo::core::Registry<fo::core::IHasher>::instance().create("fast64");
    if (!hasher) {
        state.SkipWithError("fast64 hasher not found");
        fs::remove(path);
        return;
    }

    for (auto _ : state) {
        std::string h = hasher->fast64(path);
        benchmark::DoNotOptimize(h);
    }
    state.SetBytesProcessed(state.iterations() * 1024 * 1024);
    fs::remove(path);
}
BENCHMARK(BM_Hasher_Fast64);

static void BM_Hasher_Blake3(benchmark::State& state) {
    fs::path path = fs::temp_directory_path() / "fo_bench_hash_b3.tmp";
    {
        std::ofstream ofs(path, std::ios::binary);
        std::vector<char> data(1024 * 1024, 'a'); // 1MB
        ofs.write(data.data(), data.size());
    }
    
    auto hasher = fo::core::Registry<fo::core::IHasher>::instance().create("blake3");
    if (!hasher) {
        state.SkipWithError("blake3 hasher not found");
        fs::remove(path);
        return;
    }

    for (auto _ : state) {
        auto h = hasher->strong(path);
        benchmark::DoNotOptimize(h);
    }
    state.SetBytesProcessed(state.iterations() * 1024 * 1024);
    fs::remove(path);
}
BENCHMARK(BM_Hasher_Blake3);

// ── SearchEngine Benchmarks ──────────────────────────────────────────

#include "fo/core/search_interface.hpp"

class SearchFixture : public benchmark::Fixture {
public:
    fs::path test_dir;

    void SetUp(const ::benchmark::State&) override {
        fo::core::register_all_providers();

        test_dir = fs::temp_directory_path() / "fo_bench_search";
        if (fs::exists(test_dir)) fs::remove_all(test_dir);
        fs::create_directories(test_dir);

        // Create 200 files across 10 subdirs
        for (int d = 0; d < 10; ++d) {
            fs::path sub = test_dir / ("dir_" + std::to_string(d));
            fs::create_directories(sub);
            for (int i = 0; i < 20; ++i) {
                std::ofstream(sub / ("file_" + std::to_string(i) + ".txt"))
                    << "benchmark content line " << i << "\nneedle in haystack\nmore data";
            }
        }
    }

    void TearDown(const ::benchmark::State&) override {
        fs::remove_all(test_dir);
    }
};

BENCHMARK_DEFINE_F(SearchFixture, LiteralFilename)(benchmark::State& state) {
    fo::core::SearchEngine engine;
    fo::core::SearchOptions opts;
    opts.search_roots = {test_dir};
    opts.query = "file_5";
    opts.match_mode = fo::core::SearchOptions::MatchMode::Literal;

    for (auto _ : state) {
        auto results = engine.search(opts);
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK_REGISTER_F(SearchFixture, LiteralFilename);

BENCHMARK_DEFINE_F(SearchFixture, WildcardFilename)(benchmark::State& state) {
    fo::core::SearchEngine engine;
    fo::core::SearchOptions opts;
    opts.search_roots = {test_dir};
    opts.query = "file_*.txt";
    opts.match_mode = fo::core::SearchOptions::MatchMode::Wildcard;

    for (auto _ : state) {
        auto results = engine.search(opts);
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK_REGISTER_F(SearchFixture, WildcardFilename);

BENCHMARK_DEFINE_F(SearchFixture, RegexFilename)(benchmark::State& state) {
    fo::core::SearchEngine engine;
    fo::core::SearchOptions opts;
    opts.search_roots = {test_dir};
    opts.query = R"(file_\d+\.txt)";
    opts.match_mode = fo::core::SearchOptions::MatchMode::Regex;

    for (auto _ : state) {
        auto results = engine.search(opts);
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK_REGISTER_F(SearchFixture, RegexFilename);

BENCHMARK_DEFINE_F(SearchFixture, ContentLiteral)(benchmark::State& state) {
    fo::core::SearchEngine engine;
    fo::core::SearchOptions opts;
    opts.search_roots = {test_dir};
    opts.query = ""; // match all filenames
    opts.search_content = true;
    opts.content_query = "needle";
    opts.content_match_mode = fo::core::SearchOptions::MatchMode::Literal;

    for (auto _ : state) {
        auto results = engine.search(opts);
        benchmark::DoNotOptimize(results);
    }
}
BENCHMARK_REGISTER_F(SearchFixture, ContentLiteral);

// ── Linter Benchmark ──────────────────────────────────────────────────

#include "fo/core/lint_interface.hpp"

static void BM_Linter_Std(benchmark::State& state) {
    fo::core::register_all_providers();

    fs::path test_dir = fs::temp_directory_path() / "fo_bench_linter";
    if (fs::exists(test_dir)) fs::remove_all(test_dir);
    fs::create_directories(test_dir);

    // Create a mix of files
    for (int i = 0; i < 100; ++i) {
        std::ofstream(test_dir / ("file_" + std::to_string(i) + ".txt")) << "data";
    }
    for (int i = 0; i < 10; ++i) {
        std::ofstream(test_dir / ("temp_" + std::to_string(i) + ".bak")) << "temp";
    }
    for (int i = 0; i < 5; ++i) {
        std::ofstream(test_dir / ("empty_" + std::to_string(i) + ".txt")); // empty
    }
    auto empty_dir = test_dir / "empty_sub";
    fs::create_directories(empty_dir);

    auto linter = fo::core::Registry<fo::core::ILinter>::instance().create("std");
    if (!linter) { state.SkipWithError("std linter not found"); return; }

    for (auto _ : state) {
        auto results = linter->lint({test_dir});
        benchmark::DoNotOptimize(results);
    }

    fs::remove_all(test_dir);
}
BENCHMARK(BM_Linter_Std);

BENCHMARK_MAIN();
