#include "fo/core/engine.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include "fo/core/ocr_interface.hpp"
#include "fo/core/perceptual_hash_interface.hpp"
#include "fo/core/classification_interface.hpp"
#include "fo/core/rule_engine.hpp"
#include "fo/core/export.hpp"
#include "fo/core/version.hpp"
#include "fo/core/operation_repository.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <map>
#include <unordered_map>
#include <thread>
#include <mutex>

// Parse size string with optional K/M/G suffix (e.g., "1M" -> 1048576)
static std::uintmax_t parse_size_string(const std::string& s) {
    if (s.empty()) return 0;
    char suffix = s.back();
    std::uintmax_t multiplier = 1;
    std::string num_part = s;
    if (suffix == 'K' || suffix == 'k') { multiplier = 1024ULL; num_part.pop_back(); }
    else if (suffix == 'M' || suffix == 'm') { multiplier = 1024ULL * 1024; num_part.pop_back(); }
    else if (suffix == 'G' || suffix == 'g') { multiplier = 1024ULL * 1024 * 1024; num_part.pop_back(); }
    return static_cast<std::uintmax_t>(std::stoull(num_part)) * multiplier;
}

// Simple glob matching (supports * and ? wildcards against filename only)
static bool glob_match(const std::string& pattern, const std::string& text) {
    size_t pi = 0, ti = 0;
    size_t star_p = std::string::npos, star_t = 0;
    while (ti < text.size()) {
        if (pi < pattern.size() && (pattern[pi] == text[ti] || pattern[pi] == '?')) {
            ++pi; ++ti;
        } else if (pi < pattern.size() && pattern[pi] == '*') {
            star_p = pi++; star_t = ti;
        } else if (star_p != std::string::npos) {
            pi = star_p + 1; ti = ++star_t;
        } else {
            return false;
        }
    }
    while (pi < pattern.size() && pattern[pi] == '*') ++pi;
    return pi == pattern.size();
}

static std::string format_human_size(std::uintmax_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double val = static_cast<double>(bytes);
    int unit = 0;
    while (val >= 1024.0 && unit < 4) { val /= 1024.0; ++unit; }
    std::ostringstream oss;
    if (unit == 0) oss << bytes << " B";
    else oss << std::fixed << std::setprecision(2) << val << " " << units[unit];
    return oss.str();
}

void lint_command(const std::vector<std::string>& args);

using namespace std::chrono;

static void print_usage() {
    std::cout << "filez v" << fo::core::FO_VERSION << "\n";
    std::cout << "Usage: fo_cli <command> [options] [paths...]\n"
              << "Commands:\n"
              << "  scan         Scan for files\n"
              << "  duplicates   Find duplicate files\n"
              << "  hash         Compute file hashes\n"
              << "  metadata     Extract file metadata\n"
              << "  ocr          Extract text from images\n"
              << "  similar      Find similar images\n"
              << "  classify     Classify images using AI\n"
              << "  organize     Organize files based on rules\n"
              << "  delete-duplicates Delete duplicate files\n"
              << "  rename       Rename files based on pattern\n"
              << "  stats        Show file statistics and distribution\n"
              << "  lint         Lint filesystem for empty files/broken links\n"
              << "  export       Export scan results to JSON/CSV/HTML\n"
              << "  undo         Undo the last file operation\n"
              << "  history      Show operation history\n"
              << "\nOptions:\n"
              << "  --scanner=<name>    Select scanner (e.g., std, win32, s3, gdrive)\n"
              << "  --hasher=<name>     Select hasher (e.g., fast64, blake3)\n"
              << "  --s3-bucket=<bkt>   S3 bucket name for S3Scanner\n"
              << "  --gdrive-token=<tk> Google Drive Access Token for GDriveScanner\n"
              << "  --db=<path>         Database path (default: fo.db)\n"
              << "  --rule=<template>   Organization rule (e.g., '/Photos/{year}/{month}')\n"
              << "  --rules=<file.yaml> Load organization rules from YAML file\n"
              << "  --pattern=<tmpl>    Rename pattern (e.g., '{year}_{name}.{ext}')\n"
              << "  --keep=<strategy>   Keep strategy: oldest, newest, shortest, longest (default: oldest)\n"
              << "  --output=<path>     Output file path for export command\n"
              << "  --dry-run           Simulate organization without moving files\n"
              << "  --ext=<.jpg,.png>   Comma-separated list of extensions\n"
              << "  --follow-symlinks   Follow symbolic links\n"
              << "  --format=<fmt>      Output format (json, csv, html)\n"
              << "  --threshold=<N>     Similarity threshold (default: 10)\n"
              << "  --phash=<algo>      Perceptual hash algorithm (dhash, phash, ahash)\n"
              << "  --min-size=<N>      Minimum file size filter (supports K/M/G suffix)\n"
              << "  --max-size=<N>      Maximum file size filter (supports K/M/G suffix)\n"
              << "  --exclude=<glob>    Exclude files matching glob pattern (repeatable)\n"
              << "  --mode=<mode>       Verification mode: fast, safe, paranoid (default: fast)\n"
              << "  --threads=<N>       Number of hashing threads (default: 1)\n"
              << "  --no-recursive      Only scan files directly in specified directories\n"
              << "  --time              Display elapsed wall-clock time after command\n"
              << "  --verbose           Show progress during long operations\n"
              << "  --use-ads-cache     Use Windows NTFS Alternate Data Streams for hash caching\n"
              << "  --thumbnails        Include thumbnails in HTML export (images only)\n"
              << "  --list-scanners     List available scanners\n"
              << "  --list-hashers      List available hashers\n"
              << "  --list-metadata     List available metadata providers\n"
              << "  --list-ocr          List available OCR providers\n"
              << "  --list-classifiers  List available classifiers\n"
              << "  --list-phash        List available perceptual hash algorithms\n"
              << "  --modules           List all registered modules/providers\n"
              << "  --download-models   Download default AI models\n";
}

#ifdef FO_HAVE_S3
namespace fo::core {
    void init_aws_api();
    void shutdown_aws_api();
}
#endif

int main(int argc, char** argv) {
#ifdef FO_HAVE_S3
    struct AwsLifecycle {
        AwsLifecycle() { fo::core::init_aws_api(); }
        ~AwsLifecycle() { fo::core::shutdown_aws_api(); }
    } aws_lifecycle;
#endif

    fo::core::register_all_providers();

    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string command = argv[1];
    if (command == "-h" || command == "--help") {
        print_usage();
        return 0;
    }
    if (command == "--download-models") {
        std::cout << "Downloading models...\n";
        // Placeholder for actual download logic
        // In a real scenario, we would use libcurl or a system command
        // For now, just print instructions
        std::cout << "Please download the following files to your working directory:\n";
        std::cout << "1. ResNet50 ONNX model: https://github.com/onnx/models/raw/main/vision/classification/resnet/model/resnet50-v2-7.onnx (rename to model.onnx)\n";
        std::cout << "2. ImageNet Labels: https://raw.githubusercontent.com/onnx/models/main/vision/classification/synset.txt (rename to labels.txt)\n";
        return 0;
    }
    if (command == "-v" || command == "--version") {
        std::cout << "filez v" << fo::core::FO_VERSION << "\n";
        std::cout << "Build Date: " << __DATE__ << " " << __TIME__ << "\n";
        return 0;
    }
    if (command == "--modules") {
        std::cout << "Registered Modules:\n";
        
        auto& scanners = fo::core::Registry<fo::core::IFileScanner>::instance();
        std::cout << "  Scanners: ";
        for (const auto& n : scanners.names()) std::cout << n << " ";
        std::cout << "\n";

        auto& hashers = fo::core::Registry<fo::core::IHasher>::instance();
        std::cout << "  Hashers: ";
        for (const auto& n : hashers.names()) std::cout << n << " ";
        std::cout << "\n";

        auto& meta = fo::core::Registry<fo::core::IMetadataProvider>::instance();
        std::cout << "  Metadata: ";
        for (const auto& n : meta.names()) std::cout << n << " ";
        std::cout << "\n";

        auto& ocr = fo::core::Registry<fo::core::IOCRProvider>::instance();
        std::cout << "  OCR: ";
        for (const auto& n : ocr.names()) std::cout << n << " ";
        std::cout << "\n";

        auto& classifiers = fo::core::Registry<fo::core::IImageClassifier>::instance();
        std::cout << "  Classifiers: ";
        for (const auto& n : classifiers.names()) std::cout << n << " ";
        std::cout << "\n";

        auto& phash = fo::core::Registry<fo::core::IPerceptualHasher>::instance();
        std::cout << "  Perceptual Hash: ";
        for (const auto& n : phash.names()) std::cout << n << " ";
        std::cout << "\n";

        return 0;
    }
    if (command == "--list-scanners") {
        auto& reg = fo::core::Registry<fo::core::IFileScanner>::instance();
        std::cout << "Available scanners:";
        for (auto& n : reg.names()) std::cout << " " << n;
        std::cout << "\n";
        return 0;
    }
    if (command == "--list-hashers") {
        auto& reg = fo::core::Registry<fo::core::IHasher>::instance();
        std::cout << "Available hashers:";
        for (auto& n : reg.names()) std::cout << " " << n;
        std::cout << "\n";
        return 0;
    }
    if (command == "--list-metadata") {
        auto& reg = fo::core::Registry<fo::core::IMetadataProvider>::instance();
        std::cout << "Available metadata providers:";
        for (auto& n : reg.names()) std::cout << " " << n;
        std::cout << "\n";
        return 0;
    }
    if (command == "--list-ocr") {
        auto& reg = fo::core::Registry<fo::core::IOCRProvider>::instance();
        std::cout << "Available OCR providers:";
        for (auto& n : reg.names()) std::cout << " " << n;
        std::cout << "\n";
        return 0;
    }
    if (command == "--list-classifiers") {
        auto& reg = fo::core::Registry<fo::core::IImageClassifier>::instance();
        std::cout << "Available classifiers:";
        for (auto& n : reg.names()) std::cout << " " << n;
        std::cout << "\n";
        return 0;
    }
    if (command == "--list-phash") {
        auto& reg = fo::core::Registry<fo::core::IPerceptualHasher>::instance();
        std::cout << "Available perceptual hash algorithms:";
        for (auto& n : reg.names()) std::cout << " " << n;
        std::cout << "\n";
        return 0;
    }

    std::vector<std::filesystem::path> roots;
    std::vector<std::string> exts;
    bool follow_symlinks = false;
    std::string format;
    std::string lang = "eng";
    std::string rule_template;
    std::string rules_file;
    std::string rename_pattern;
    std::string keep_strategy = "oldest";
    std::string output_path;
    std::string phash_algo = "dhash";
    std::string verification_mode = "fast";
    bool dry_run = false;
    bool prune = false;
    bool include_thumbnails = false;
    bool no_recursive = false;
    bool show_time = false;
    bool verbose = false;
    int threshold = 10;
    int num_threads = 1;
    std::uintmax_t min_size = 0;
    std::uintmax_t max_size = std::numeric_limits<std::uintmax_t>::max();
    std::vector<std::string> exclude_patterns;
    fo::core::EngineConfig cfg;

    for (int i = 2; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "-h" || a == "--help") { print_usage(); return 0; }
        else if (a == "--list-scanners") {
            auto& reg = fo::core::Registry<fo::core::IFileScanner>::instance();
            std::cout << "Available scanners:";
            for (auto& n : reg.names()) std::cout << " " << n;
            std::cout << "\n";
            return 0;
        }
        else if (a == "--list-hashers") {
            auto& reg = fo::core::Registry<fo::core::IHasher>::instance();
            std::cout << "Available hashers:";
            for (auto& n : reg.names()) std::cout << " " << n;
            std::cout << "\n";
            return 0;
        }
        else if (a == "--list-metadata") {
            auto& reg = fo::core::Registry<fo::core::IMetadataProvider>::instance();
            std::cout << "Available metadata providers:";
            for (auto& n : reg.names()) std::cout << " " << n;
            std::cout << "\n";
            return 0;
        }
        else if (a == "--list-ocr") {
            auto& reg = fo::core::Registry<fo::core::IOCRProvider>::instance();
            std::cout << "Available OCR providers:";
            for (auto& n : reg.names()) std::cout << " " << n;
            std::cout << "\n";
            return 0;
        }
        else if (a == "--list-classifiers") {
            auto& reg = fo::core::Registry<fo::core::IImageClassifier>::instance();
            std::cout << "Available classifiers:";
            for (auto& n : reg.names()) std::cout << " " << n;
            std::cout << "\n";
            return 0;
        }
        else if (a == "--list-phash") {
            auto& reg = fo::core::Registry<fo::core::IPerceptualHasher>::instance();
            std::cout << "Available perceptual hash algorithms:";
            for (auto& n : reg.names()) std::cout << " " << n;
            std::cout << "\n";
            return 0;
        }
        else if (a.rfind("--phash=", 0) == 0) phash_algo = a.substr(8);
        else if (a.rfind("--scanner=", 0) == 0) cfg.scanner = a.substr(10);
        else if (a.rfind("--hasher=", 0) == 0) cfg.hasher = a.substr(9);
        else if (a.rfind("--s3-bucket=", 0) == 0) cfg.s3_bucket = a.substr(12);
        else if (a.rfind("--gdrive-token=", 0) == 0) cfg.gdrive_token = a.substr(15);
        else if (a.rfind("--azure-connection=", 0) == 0) cfg.azure_connection_str = a.substr(19);
        else if (a.rfind("--azure-container=", 0) == 0) cfg.azure_container = a.substr(18);
        else if (a.rfind("--db=", 0) == 0) cfg.db_path = a.substr(5);
        else if (a.rfind("--rule=", 0) == 0) rule_template = a.substr(7);
        else if (a.rfind("--rules=", 0) == 0) rules_file = a.substr(8);
        else if (a.rfind("--pattern=", 0) == 0) rename_pattern = a.substr(10);
        else if (a.rfind("--keep=", 0) == 0) keep_strategy = a.substr(7);
        else if (a.rfind("--output=", 0) == 0) output_path = a.substr(9);
        else if (a == "--dry-run") dry_run = true;
        else if (a == "--prune" || a == "--incremental") prune = true;
        else if (a == "--use-ads-cache") cfg.use_ads_cache = true;
        else if (a == "--thumbnails") include_thumbnails = true;
        else if (a.rfind("--lang=", 0) == 0) lang = a.substr(7);
        else if (a.rfind("--threshold=", 0) == 0) threshold = std::stoi(a.substr(12));
        else if (a.rfind("--ext=", 0) == 0) {
            auto list = a.substr(6);
            size_t pos = 0;
            while (pos < list.size()) {
                auto comma = list.find(',', pos);
                if (comma == std::string::npos) comma = list.size();
                exts.push_back(list.substr(pos, comma - pos));
                pos = comma + 1;
            }
        } else if (a.rfind("--format=", 0) == 0) {
            format = a.substr(9);
        } else if (a == "--follow-symlinks") {
            follow_symlinks = true;
        } else if (a.rfind("--min-size=", 0) == 0) {
            min_size = parse_size_string(a.substr(11));
        } else if (a.rfind("--max-size=", 0) == 0) {
            max_size = parse_size_string(a.substr(11));
        } else if (a.rfind("--exclude=", 0) == 0) {
            exclude_patterns.push_back(a.substr(10));
        } else if (a.rfind("--mode=", 0) == 0) {
            verification_mode = a.substr(7);
            if (verification_mode != "fast" && verification_mode != "safe" && verification_mode != "paranoid") {
                std::cerr << "Unknown mode: " << verification_mode << " (use fast, safe, or paranoid)\n";
                return 2;
            }
        } else if (a == "--no-recursive") {
            no_recursive = true;
        } else if (a == "--time") {
            show_time = true;
        } else if (a == "--verbose") {
            verbose = true;
        } else if (a.rfind("--threads=", 0) == 0) {
            num_threads = std::stoi(a.substr(10));
            if (num_threads < 1) num_threads = 1;
        } else if (!a.empty() && a[0] == '-') {
            std::cerr << "Unknown option: " << a << "\n";
            return 2;
        } else {
            roots.emplace_back(a);
        }
    }

#ifdef _WIN32
    if (!cfg.s3_bucket.empty()) _putenv_s("S3_BUCKET", cfg.s3_bucket.c_str());
    if (!cfg.gdrive_token.empty()) _putenv_s("GDRIVE_TOKEN", cfg.gdrive_token.c_str());
    if (!cfg.azure_connection_str.empty()) _putenv_s("AZURE_STORAGE_CONNECTION_STRING", cfg.azure_connection_str.c_str());
    if (!cfg.azure_container.empty()) _putenv_s("AZURE_CONTAINER", cfg.azure_container.c_str());
#else
    if (!cfg.s3_bucket.empty()) setenv("S3_BUCKET", cfg.s3_bucket.c_str(), 1);
    if (!cfg.gdrive_token.empty()) setenv("GDRIVE_TOKEN", cfg.gdrive_token.c_str(), 1);
    if (!cfg.azure_connection_str.empty()) setenv("AZURE_STORAGE_CONNECTION_STRING", cfg.azure_connection_str.c_str(), 1);
    if (!cfg.azure_container.empty()) setenv("AZURE_CONTAINER", cfg.azure_container.c_str(), 1);
#endif

    try {
        auto timer_start = std::chrono::steady_clock::now();
        fo::core::Engine engine(cfg);

        // Post-scan filter lambda for size, exclude patterns, and depth
        auto apply_filters = [&](std::vector<fo::core::FileInfo>& files) {
            if (min_size > 0 || max_size < std::numeric_limits<std::uintmax_t>::max() || !exclude_patterns.empty() || no_recursive) {
                // Build normalized root set for depth filtering
                std::vector<std::string> norm_roots;
                if (no_recursive) {
                    for (const auto& r : roots) {
                        auto p = std::filesystem::canonical(r).string();
                        if (!p.empty() && p.back() != '/' && p.back() != '\\') p += '\\';
                        norm_roots.push_back(p);
                    }
                }
                std::erase_if(files, [&](const fo::core::FileInfo& f) {
                    if (f.size < min_size || f.size > max_size) return true;
                    // Extract filename from URI
                    std::string fname = f.uri;
                    auto slash = fname.find_last_of("/\\");
                    if (!exclude_patterns.empty()) {
                        std::string name_only = (slash != std::string::npos) ? fname.substr(slash + 1) : fname;
                        for (const auto& pat : exclude_patterns) {
                            if (glob_match(pat, name_only)) return true;
                        }
                    }
                    if (no_recursive) {
                        // File must be directly inside one of the roots (no extra subdirectory)
                        std::string dir = (slash != std::string::npos) ? fname.substr(0, slash + 1) : "";
                        try {
                            auto canon_dir = std::filesystem::canonical(dir).string();
                            if (!canon_dir.empty() && canon_dir.back() != '/' && canon_dir.back() != '\\') canon_dir += '\\';
                            bool in_root = false;
                            for (const auto& nr : norm_roots) {
                                if (canon_dir == nr) { in_root = true; break; }
                            }
                            if (!in_root) return true;
                        } catch (...) { return true; }
                    }
                    return false;
                });
            }
        };

        if (command == "scan") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            apply_filters(files);
            if (format == "json") {
                std::cout << "[\n";
                for (size_t i = 0; i < files.size(); ++i) {
                    std::cout << "  {\"path\": \"" << fo::core::Exporter::json_escape(files[i].uri)
                              << "\", \"size\": " << files[i].size << "}";
                    if (i + 1 < files.size()) std::cout << ",";
                    std::cout << "\n";
                }
                std::cout << "]\n";
            } else {
                for (const auto& f : files) {
                    std::cout << f.uri << "\n";
                }
            if (verbose) std::cerr << "Scanned " << files.size() << " files\n";
            }
        } else if (command == "duplicates") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            apply_filters(files);
            if (verbose) std::cerr << "Scanned " << files.size() << " files, finding duplicates...\n";
            auto groups = engine.find_duplicates(files);
            if (verbose) std::cerr << "Found " << groups.size() << " duplicate groups\n";

            // Apply verification mode
            if (verification_mode == "safe" || verification_mode == "paranoid") {
                if (verbose) std::cerr << "Running strong hash verification...\n";
                // Collect all files that need strong hashing across all groups
                struct StrongHashTask { size_t group_idx; size_t file_idx; std::string hash; };
                std::vector<StrongHashTask> tasks;
                for (size_t gi = 0; gi < groups.size(); ++gi) {
                    for (size_t fi = 0; fi < groups[gi].files.size(); ++fi) {
                        tasks.push_back({gi, fi, ""});
                    }
                }

                auto& hasher = engine.hasher();
                if (num_threads > 1 && tasks.size() > 1) {
                    // Parallel strong hashing
                    std::vector<std::thread> threads;
                    size_t chunk = (tasks.size() + num_threads - 1) / num_threads;
                    for (int t = 0; t < num_threads; ++t) {
                        size_t start = t * chunk;
                        size_t end = std::min(start + chunk, tasks.size());
                        if (start >= tasks.size()) break;
                        threads.emplace_back([&, start, end]() {
                            auto th = fo::core::Registry<fo::core::IHasher>::instance().create(hasher.name());
                            for (size_t i = start; i < end; ++i) {
                                auto strong = th->strong(std::filesystem::path(groups[tasks[i].group_idx].files[tasks[i].file_idx].uri));
                                tasks[i].hash = strong.value_or("unknown");
                            }
                        });
                    }
                    for (auto& th : threads) th.join();
                } else {
                    // Sequential strong hashing
                    for (auto& task : tasks) {
                        auto strong = hasher.strong(std::filesystem::path(groups[task.group_idx].files[task.file_idx].uri));
                        task.hash = strong.value_or("unknown");
                    }
                }

                // Regroup by strong hash within each original group
                std::vector<fo::core::DuplicateGroup> refined;
                size_t task_idx = 0;
                for (size_t gi = 0; gi < groups.size(); ++gi) {
                    std::unordered_map<std::string, std::vector<fo::core::FileInfo>> by_strong;
                    for (size_t fi = 0; fi < groups[gi].files.size(); ++fi) {
                        by_strong[tasks[task_idx].hash].push_back(std::move(groups[gi].files[fi]));
                        ++task_idx;
                    }
                    for (auto& kv : by_strong) {
                        if (kv.second.size() >= 2) {
                            fo::core::DuplicateGroup rg;
                            rg.size = groups[gi].size;
                            rg.fast64 = groups[gi].fast64;
                            rg.files = std::move(kv.second);
                            refined.push_back(std::move(rg));
                        }
                    }
                }
                groups = std::move(refined);
                if (verbose) std::cerr << "Strong hash refined to " << groups.size() << " groups\n";
            }

            if (verification_mode == "paranoid") {
                // Byte-by-byte comparison: confirm duplicates by reading file contents
                std::vector<fo::core::DuplicateGroup> verified;
                for (auto& g : groups) {
                    if (g.files.size() < 2) continue;
                    // Compare all files byte-by-byte against the first file
                    auto& reference = g.files[0];
                    std::vector<fo::core::FileInfo> matches;
                    matches.push_back(reference);

                    std::ifstream ref_stream(reference.uri, std::ios::binary);
                    if (!ref_stream) continue;
                    std::vector<char> ref_buf(std::istreambuf_iterator<char>(ref_stream), {});

                    for (size_t i = 1; i < g.files.size(); ++i) {
                        std::ifstream cmp_stream(g.files[i].uri, std::ios::binary);
                        if (!cmp_stream) continue;
                        std::vector<char> cmp_buf(std::istreambuf_iterator<char>(cmp_stream), {});
                        if (ref_buf == cmp_buf) {
                            matches.push_back(g.files[i]);
                        }
                    }
                    if (matches.size() >= 2) {
                        fo::core::DuplicateGroup vg;
                        vg.size = g.size;
                        vg.fast64 = g.fast64;
                        vg.files = std::move(matches);
                        verified.push_back(std::move(vg));
                    }
                }
                groups = std::move(verified);
            }

            if (format == "json") {
                std::cout << "[\n";
                for (size_t i = 0; i < groups.size(); ++i) {
                    const auto& g = groups[i];
                    std::cout << "  {\"size\": " << g.size << ", \"hash\": \"" << g.fast64
                              << "\", \"mode\": \"" << verification_mode << "\", \"files\": [\n";
                    for (size_t j = 0; j < g.files.size(); ++j) {
                        std::cout << "    \"" << fo::core::Exporter::json_escape(g.files[j].uri) << "\"";
                        if (j + 1 < g.files.size()) std::cout << ",";
                        std::cout << "\n";
                    }
                    std::cout << "  ]}";
                    if (i + 1 < groups.size()) std::cout << ",";
                    std::cout << "\n";
                }
                std::cout << "]\n";
            } else {
                std::cout << "Verification mode: " << verification_mode << "\n";
                for (const auto& g : groups) {
                    std::cout << "== size=" << g.size << ", fast64=" << g.fast64 << "\n";
                    for (const auto& f : g.files) {
                        std::cout << "  " << f.uri << "\n";
                    }
                }
            }
        } else if (command == "hash") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            apply_filters(files);
            auto& hasher = engine.hasher();

            // Compute hashes (parallel if --threads > 1)
            struct HashResult { size_t idx; std::string hash; };
            std::vector<HashResult> results(files.size());

            if (num_threads > 1 && files.size() > 1) {
                // Parallel hashing with thread pool
                std::vector<std::thread> threads;
                size_t chunk = (files.size() + num_threads - 1) / num_threads;

                for (int t = 0; t < num_threads; ++t) {
                    size_t start = t * chunk;
                    size_t end = std::min(start + chunk, files.size());
                    if (start >= files.size()) break;

                    threads.emplace_back([&, start, end]() {
                        // Each thread gets its own hasher instance to avoid contention
                        auto thread_hasher = fo::core::Registry<fo::core::IHasher>::instance().create(hasher.name());
                        for (size_t i = start; i < end; ++i) {
                            results[i] = { i, thread_hasher->fast64(std::filesystem::path(files[i].uri)) };
                        }
                    });
                }
                for (auto& th : threads) th.join();
            } else {
                // Sequential hashing
                for (size_t i = 0; i < files.size(); ++i) {
                    results[i] = { i, hasher.fast64(std::filesystem::path(files[i].uri)) };
                }
            }

            // Output results (always on main thread)
            if (format == "json") {
                std::cout << "[\n";
                for (size_t i = 0; i < results.size(); ++i) {
                    std::cout << "  {\"path\": \"" << fo::core::Exporter::json_escape(files[i].uri)
                              << "\", \"hash\": \"" << results[i].hash << "\"}";
                    if (i + 1 < results.size()) std::cout << ",";
                    std::cout << "\n";
                    if (files[i].id != 0) {
                        engine.file_repository().add_hash(files[i].id, hasher.name(), results[i].hash);
                    }
                }
                std::cout << "]\n";
            } else {
                for (size_t i = 0; i < results.size(); ++i) {
                    std::cout << results[i].hash << "  " << files[i].uri << "\n";
                    if (files[i].id != 0) {
                        engine.file_repository().add_hash(files[i].id, hasher.name(), results[i].hash);
                    }
                }
            }
        } else if (command == "metadata") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            apply_filters(files);
            auto provider = fo::core::Registry<fo::core::IMetadataProvider>::instance().create("tinyexif");
            if (!provider) {
                std::cerr << "Metadata provider 'tinyexif' not found.\n";
                return 1;
            }
            if (format == "json") {
                std::cout << "[\n";
                bool first = true;
                for (const auto& f : files) {
                    fo::core::ImageMetadata meta;
                    if (provider->read(std::filesystem::path(f.uri), meta)) {
                        if (!first) std::cout << ",\n";
                        first = false;
                        std::cout << "  {\"path\": \"" << fo::core::Exporter::json_escape(f.uri) << "\"";
                        if (meta.date.has_taken) {
                            auto t = std::chrono::system_clock::to_time_t(meta.date.taken);
                            std::tm tm_buf;
#ifdef _WIN32
                            localtime_s(&tm_buf, &t);
#else
                            localtime_r(&t, &tm_buf);
#endif
                            std::ostringstream oss;
                            oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S");
                            std::cout << ", \"taken\": \"" << oss.str() << "\"";
                        }
                        if (meta.has_gps) {
                            std::cout << ", \"gps_lat\": " << meta.gps_lat << ", \"gps_lon\": " << meta.gps_lon;
                        }
                        std::cout << "}";
                    }
                }
                std::cout << "\n]\n";
            } else {
                for (const auto& f : files) {
                    fo::core::ImageMetadata meta;
                    if (provider->read(std::filesystem::path(f.uri), meta)) {
                        std::cout << f.uri << ":\n";
                        if (meta.date.has_taken) {
                            auto t = std::chrono::system_clock::to_time_t(meta.date.taken);
                            std::cout << "  Taken: " << std::ctime(&t);
                        }
                        if (meta.has_gps) {
                            std::cout << "  GPS: " << meta.gps_lat << ", " << meta.gps_lon << "\n";
                        }
                    }
                }
            }
        } else if (command == "ocr") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            apply_filters(files);
            auto provider = fo::core::Registry<fo::core::IOCRProvider>::instance().create("tesseract");
            if (!provider) {
                std::cerr << "OCR provider 'tesseract' not found.\n";
                return 1;
            }
            for (const auto& f : files) {
                auto result = provider->recognize(std::filesystem::path(f.uri), lang);
                if (result) {
                    std::cout << f.uri << ":\n";
                    std::cout << "  Text: " << result->text << "\n";
                }
            }
        } else if (command == "similar") {
            if (roots.empty()) {
                std::cerr << "Usage: fo similar <image_path> [--threshold=10] [--phash=dhash|phash|ahash]\n";
                return 1;
            }

            auto provider = fo::core::Registry<fo::core::IPerceptualHasher>::instance().create(phash_algo);
            if (!provider) {
                std::cerr << "Perceptual hasher '" << phash_algo << "' not found. Use --list-phash to see available options.\n";
                return 1;
            }

            auto res = provider->compute(roots[0]);
            if (!res) {
                std::cerr << "Failed to compute hash for " << roots[0] << "\n";
                return 1;
            }

            auto matches = engine.file_repository().find_similar_images(res->value, threshold);

            if (format == "json") {
                std::cout << "{\"query\": \"" << fo::core::Exporter::json_escape(roots[0].string()) << "\""
                          << ", \"hash\": \"" << res->value << "\""
                          << ", \"algorithm\": \"" << phash_algo << "\""
                          << ", \"threshold\": " << threshold
                          << ", \"matches\": [\n";
                for (size_t i = 0; i < matches.size(); ++i) {
                    auto fi = engine.file_repository().get_by_id(matches[i]);
                    if (fi) {
                        std::cout << "    {\"id\": " << fi->id
                                  << ", \"path\": \"" << fo::core::Exporter::json_escape(fi->uri) << "\""
                                  << ", \"size\": " << fi->size
                                  << "}" << (i + 1 < matches.size() ? "," : "") << "\n";
                    }
                }
                std::cout << "  ]\n}\n";
            } else {
                std::cout << "Target hash: " << res->value << " (" << res->method << ")\n";
                std::cout << "Algorithm: " << phash_algo << ", Threshold: " << threshold << "\n";
                std::cout << "Found " << matches.size() << " similar images:\n";
                for (auto id : matches) {
                    auto fi = engine.file_repository().get_by_id(id);
                    if (fi) {
                        std::cout << "  " << fi->uri << "\n";
                    }
                }
            }
        } else if (command == "classify") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            apply_filters(files);
            auto provider = fo::core::Registry<fo::core::IImageClassifier>::instance().create("onnx");
            if (!provider) {
                std::cerr << "Classifier 'onnx' not found.\n";
                return 1;
            }

            if (format == "json") {
                std::cout << "[\n";
                bool first_file = true;
                for (const auto& f : files) {
                    auto results = provider->classify(std::filesystem::path(f.uri));
                    if (!results.empty()) {
                        if (!first_file) std::cout << ",\n";
                        first_file = false;
                        std::cout << "  {\"path\": \"" << fo::core::Exporter::json_escape(f.uri) << "\""
                                  << ", \"classifications\": [";
                        for (size_t i = 0; i < results.size(); ++i) {
                            const auto& r = results[i];
                            std::cout << "{\"label\": \"" << fo::core::Exporter::json_escape(r.label) << "\""
                                      << ", \"confidence\": " << r.confidence << "}";
                            if (i + 1 < results.size()) std::cout << ", ";
                            if (f.id != 0) {
                                engine.file_repository().add_tag(f.id, r.label, r.confidence, "ai");
                            }
                        }
                        std::cout << "]}";
                    }
                }
                std::cout << "\n]\n";
            } else {
                for (const auto& f : files) {
                    auto results = provider->classify(std::filesystem::path(f.uri));
                    if (!results.empty()) {
                        std::cout << f.uri << ":\n";
                        for (const auto& r : results) {
                            std::cout << "  " << r.label << " (" << r.confidence << ")\n";
                            if (f.id != 0) {
                                engine.file_repository().add_tag(f.id, r.label, r.confidence, "ai");
                            }
                        }
                    }
                }
            }
        } else if (command == "organize") {
            if (rule_template.empty() && rules_file.empty()) {
                std::cerr << "Error: --rule or --rules argument is required for organize command.\n";
                return 1;
            }

            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            apply_filters(files);
            fo::core::RuleEngine rule_engine;

            if (!rule_template.empty()) {
                rule_engine.add_rule({"cli_rule", "", rule_template});
            }

            if (!rules_file.empty()) {
                if (!rule_engine.load_rules_from_yaml(rules_file)) {
                    std::cerr << "Failed to load rules from " << rules_file << "\n";
                    return 1;
                }
            }

            std::vector<std::pair<std::string, std::string>> moves;

            for (const auto& f : files) {
                std::vector<std::string> tags;
                if (f.id != 0) {
                    auto tag_pairs = engine.file_repository().get_tags(f.id);
                    for (const auto& p : tag_pairs) tags.push_back(p.first);
                }

                auto new_path_opt = rule_engine.apply_rules(f, tags);
                if (new_path_opt && new_path_opt->string() != f.uri) {
                    auto new_path = *new_path_opt;
                    moves.push_back({f.uri, new_path.string()});
                    if (!dry_run) {
                        try {
                            std::filesystem::create_directories(new_path.parent_path());
                            std::filesystem::rename(f.uri, new_path);

                            fo::core::OperationRepository op_repo(engine.database());
                            fo::core::OperationRecord rec;
                            rec.timestamp = std::chrono::system_clock::now();
                            rec.type = fo::core::OperationType::Move;
                            rec.source_path = f.uri;
                            rec.dest_path = new_path.string();
                            rec.file_size = f.size;
                            op_repo.log_operation(rec);
                        } catch (const std::exception& e) {
                            std::cerr << "Failed to move " << f.uri << ": " << e.what() << "\n";
                        }
                    }
                }
            }

            if (format == "json") {
                std::cout << "{\"dry_run\": " << (dry_run ? "true" : "false")
                          << ", \"total_files\": " << files.size()
                          << ", \"moves\": [\n";
                for (size_t i = 0; i < moves.size(); ++i) {
                    std::cout << "    {\"source\": \"" << fo::core::Exporter::json_escape(moves[i].first) << "\""
                              << ", \"dest\": \"" << fo::core::Exporter::json_escape(moves[i].second) << "\"}"
                              << (i + 1 < moves.size() ? "," : "") << "\n";
                }
                std::cout << "  ]\n}\n";
            } else {
                std::cout << "Organizing " << files.size() << " files...\n";
                if (dry_run) std::cout << "(Dry run - no files will be moved)\n";
                for (const auto& m : moves) {
                    std::cout << m.first << " -> " << m.second << "\n";
                }
            }
        } else if (command == "delete-duplicates") {
            auto groups = engine.duplicate_repository().get_all_groups();

            int deleted_count = 0;
            int kept_count = 0;
            std::vector<std::pair<std::string, std::vector<std::string>>> results; // kept, deleted[]

            for (const auto& g : groups) {
                std::vector<fo::core::FileInfo> members;
                auto p = engine.file_repository().get_by_id(g.primary_file_id);
                if (p) members.push_back(*p);
                for (auto mid : g.member_ids) {
                    if (mid == g.primary_file_id) continue;
                    auto m = engine.file_repository().get_by_id(mid);
                    if (m) members.push_back(*m);
                }

                if (members.size() < 2) continue;

                std::sort(members.begin(), members.end(), [&](const fo::core::FileInfo& a, const fo::core::FileInfo& b) {
                    if (keep_strategy == "newest") return a.mtime > b.mtime;
                    if (keep_strategy == "shortest") return a.uri.length() < b.uri.length();
                    if (keep_strategy == "longest") return a.uri.length() > b.uri.length();
                    return a.mtime < b.mtime;
                });

                const auto& keep = members[0];
                kept_count++;
                std::vector<std::string> deleted_paths;

                for (size_t i = 1; i < members.size(); ++i) {
                    const auto& del = members[i];
                    deleted_paths.push_back(del.uri);
                    if (!dry_run) {
                        try {
                            std::filesystem::remove(del.uri);
                            deleted_count++;
                            fo::core::OperationRepository op_repo(engine.database());
                            fo::core::OperationRecord rec;
                            rec.timestamp = std::chrono::system_clock::now();
                            rec.type = fo::core::OperationType::Delete;
                            rec.source_path = del.uri;
                            rec.file_size = del.size;
                            op_repo.log_operation(rec);
                        } catch (const std::exception& e) {
                            std::cerr << "Failed to delete: " << e.what() << "\n";
                        }
                    }
                }
                results.push_back({keep.uri, deleted_paths});
            }

            if (format == "json") {
                std::cout << "{\"dry_run\": " << (dry_run ? "true" : "false")
                          << ", \"strategy\": \"" << keep_strategy << "\""
                          << ", \"groups\": " << groups.size()
                          << ", \"kept\": " << kept_count
                          << ", \"deleted\": " << deleted_count
                          << ", \"results\": [\n";
                for (size_t i = 0; i < results.size(); ++i) {
                    const auto& [kept_path, deleted_paths] = results[i];
                    std::cout << "    {\"kept\": \"" << fo::core::Exporter::json_escape(kept_path) << "\""
                              << ", \"deleted\": [";
                    for (size_t j = 0; j < deleted_paths.size(); ++j) {
                        std::cout << "\"" << fo::core::Exporter::json_escape(deleted_paths[j]) << "\"";
                        if (j + 1 < deleted_paths.size()) std::cout << ", ";
                    }
                    std::cout << "]}" << (i + 1 < results.size() ? "," : "") << "\n";
                }
                std::cout << "  ]\n}\n";
            } else {
                std::cout << "Found " << groups.size() << " duplicate groups.\n";
                if (dry_run) std::cout << "(Dry run - no files will be deleted)\n";
                for (const auto& [kept_path, deleted_paths] : results) {
                    std::cout << "Keeping: " << kept_path << "\n";
                    for (const auto& dp : deleted_paths) {
                        std::cout << "  Deleting: " << dp << "\n";
                    }
                }
                std::cout << "Deleted " << deleted_count << " files. Kept " << kept_count << " files.\n";
            }

        } else if (command == "rename") {
            if (rename_pattern.empty()) {
                std::cerr << "Error: --pattern argument is required for rename command.\n";
                return 1;
            }

            if (rename_pattern.find("{parent}") == std::string::npos &&
                rename_pattern.find("/") == std::string::npos &&
                rename_pattern.find("\\") == std::string::npos) {
                rename_pattern = "{parent}/" + rename_pattern;
            }

            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            fo::core::RuleEngine rule_engine;
            rule_engine.add_rule({"rename_rule", "", rename_pattern});

            std::vector<std::pair<std::string, std::string>> renames;

            for (const auto& f : files) {
                std::vector<std::string> tags;
                if (f.id != 0) {
                    auto tag_pairs = engine.file_repository().get_tags(f.id);
                    for (const auto& p : tag_pairs) tags.push_back(p.first);
                }

                auto new_path_opt = rule_engine.apply_rules(f, tags);
                if (new_path_opt && new_path_opt->string() != f.uri) {
                    auto new_path = *new_path_opt;
                    renames.push_back({f.uri, new_path.string()});
                    if (!dry_run) {
                        try {
                            std::filesystem::rename(f.uri, new_path);
                            fo::core::OperationRepository op_repo(engine.database());
                            fo::core::OperationRecord rec;
                            rec.timestamp = std::chrono::system_clock::now();
                            rec.type = fo::core::OperationType::Rename;
                            rec.source_path = f.uri;
                            rec.dest_path = new_path.string();
                            rec.file_size = f.size;
                            op_repo.log_operation(rec);
                        } catch (const std::exception& e) {
                            std::cerr << "Failed to rename " << f.uri << ": " << e.what() << "\n";
                        }
                    }
                }
            }

            if (format == "json") {
                std::cout << "{\"dry_run\": " << (dry_run ? "true" : "false")
                          << ", \"pattern\": \"" << fo::core::Exporter::json_escape(rename_pattern) << "\""
                          << ", \"total_files\": " << files.size()
                          << ", \"renames\": [\n";
                for (size_t i = 0; i < renames.size(); ++i) {
                    std::cout << "    {\"source\": \"" << fo::core::Exporter::json_escape(renames[i].first) << "\""
                              << ", \"dest\": \"" << fo::core::Exporter::json_escape(renames[i].second) << "\"}"
                              << (i + 1 < renames.size() ? "," : "") << "\n";
                }
                std::cout << "  ]\n}\n";
            } else {
                std::cout << "Renaming " << files.size() << " files using pattern: " << rename_pattern << "\n";
                if (dry_run) std::cout << "(Dry run - no files will be renamed)\n";
                for (const auto& r : renames) {
                    std::cout << r.first << " -> " << r.second << "\n";
                }
            }

        } else if (command == "stats") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            apply_filters(files);

            // Compute statistics
            std::uintmax_t total_size = 0;
            int dir_count = 0, file_count = 0;
            std::map<std::string, int> ext_counts;
            std::map<std::string, std::uintmax_t> ext_sizes;
            // Size buckets: 0-1KB, 1KB-1MB, 1MB-100MB, 100MB-1GB, 1GB+
            int buckets[5] = {0, 0, 0, 0, 0};
            const char* bucket_names[] = {"0-1KB", "1KB-1MB", "1MB-100MB", "100MB-1GB", "1GB+"};
            std::chrono::file_clock::time_point oldest_time = std::chrono::file_clock::time_point::max();
            std::chrono::file_clock::time_point newest_time = std::chrono::file_clock::time_point::min();
            std::string oldest_file, newest_file;

            for (const auto& f : files) {
                if (f.is_dir) { dir_count++; continue; }
                file_count++;
                total_size += f.size;

                // Extension
                auto dot = f.uri.find_last_of('.');
                auto slash = f.uri.find_last_of("/\\");
                std::string ext = (dot != std::string::npos && (slash == std::string::npos || dot > slash))
                    ? f.uri.substr(dot) : "(none)";
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                ext_counts[ext]++;
                ext_sizes[ext] += f.size;

                // Size buckets
                if (f.size < 1024ULL) buckets[0]++;
                else if (f.size < 1024ULL * 1024) buckets[1]++;
                else if (f.size < 100ULL * 1024 * 1024) buckets[2]++;
                else if (f.size < 1024ULL * 1024 * 1024) buckets[3]++;
                else buckets[4]++;

                // Oldest/newest
                if (f.mtime < oldest_time) { oldest_time = f.mtime; oldest_file = f.uri; }
                if (f.mtime > newest_time) { newest_time = f.mtime; newest_file = f.uri; }
            }

            if (format == "json") {
                std::cout << "{\n"
                          << "  \"total_files\": " << file_count << ",\n"
                          << "  \"total_directories\": " << dir_count << ",\n"
                          << "  \"total_size\": " << total_size << ",\n"
                          << "  \"total_size_human\": \"" << format_human_size(total_size) << "\",\n";

                // Extensions
                std::vector<std::pair<std::string, int>> sorted_exts(ext_counts.begin(), ext_counts.end());
                std::sort(sorted_exts.begin(), sorted_exts.end(), [](auto& a, auto& b) { return a.second > b.second; });
                std::cout << "  \"extensions\": [\n";
                int ext_limit = std::min(static_cast<int>(sorted_exts.size()), 20);
                for (int i = 0; i < ext_limit; ++i) {
                    std::cout << "    {\"ext\": \"" << fo::core::Exporter::json_escape(sorted_exts[i].first)
                              << "\", \"count\": " << sorted_exts[i].second
                              << ", \"size\": " << ext_sizes[sorted_exts[i].first] << "}"
                              << (i + 1 < ext_limit ? "," : "") << "\n";
                }
                std::cout << "  ],\n";

                // Buckets
                std::cout << "  \"size_distribution\": {\n";
                for (int i = 0; i < 5; ++i) {
                    std::cout << "    \"" << bucket_names[i] << "\": " << buckets[i]
                              << (i < 4 ? "," : "") << "\n";
                }
                std::cout << "  }\n}\n";
            } else {
                std::cout << "File Statistics\n" << std::string(50, '=') << "\n\n";
                std::cout << "  Files:       " << file_count << "\n";
                std::cout << "  Directories: " << dir_count << "\n";
                std::cout << "  Total Size:  " << format_human_size(total_size) << "\n\n";

                if (!oldest_file.empty()) {
                    std::cout << "  Oldest: " << oldest_file << "\n";
                    std::cout << "  Newest: " << newest_file << "\n\n";
                }

                // Extension breakdown
                std::vector<std::pair<std::string, int>> sorted_exts(ext_counts.begin(), ext_counts.end());
                std::sort(sorted_exts.begin(), sorted_exts.end(), [](auto& a, auto& b) { return a.second > b.second; });
                std::cout << "Extension Breakdown (top 20)\n" << std::string(50, '-') << "\n";
                int ext_limit = std::min(static_cast<int>(sorted_exts.size()), 20);
                for (int i = 0; i < ext_limit; ++i) {
                    std::cout << "  " << std::setw(10) << std::left << sorted_exts[i].first
                              << " " << std::setw(6) << std::right << sorted_exts[i].second << " files"
                              << "  (" << format_human_size(ext_sizes[sorted_exts[i].first]) << ")\n";
                }

                // Size distribution
                std::cout << "\nSize Distribution\n" << std::string(50, '-') << "\n";
                for (int i = 0; i < 5; ++i) {
                    std::cout << "  " << std::setw(12) << std::left << bucket_names[i]
                              << " " << std::setw(6) << std::right << buckets[i] << " files\n";
                }
            }

        } else if (command == "export") {
            // Scan and find duplicates to get data for export
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            auto groups = engine.find_duplicates(files);
            auto stats = fo::core::Exporter::compute_stats(files, groups);

            // Determine format
            fo::core::ExportFormat exp_format = fo::core::ExportFormat::JSON;
            if (format == "csv") exp_format = fo::core::ExportFormat::CSV;
            else if (format == "html") exp_format = fo::core::ExportFormat::HTML;

            if (output_path.empty()) {
                // Output to stdout
                if (exp_format == fo::core::ExportFormat::JSON) {
                    fo::core::Exporter::to_json(std::cout, files, groups, stats);
                } else if (exp_format == fo::core::ExportFormat::CSV) {
                    fo::core::Exporter::to_csv(std::cout, files);
                } else if (exp_format == fo::core::ExportFormat::HTML) {
                    fo::core::Exporter::to_html(std::cout, files, groups, stats, include_thumbnails);
                }
            } else {
                // Output to file
                if (fo::core::Exporter::export_to_file(output_path, files, groups, stats, exp_format, include_thumbnails)) {
                    std::cout << "Exported to " << output_path << "\n";
                } else {
                    std::cerr << "Failed to export to " << output_path << "\n";
                    return 1;
                }
            }

        } else if (command == "undo") {
            fo::core::OperationRepository op_repo(engine.database());
            auto undone = op_repo.undo_last();

            if (format == "json") {
                if (undone) {
                    std::string type_str;
                    switch (undone->type) {
                        case fo::core::OperationType::Move: type_str = "move"; break;
                        case fo::core::OperationType::Copy: type_str = "copy"; break;
                        case fo::core::OperationType::Rename: type_str = "rename"; break;
                        case fo::core::OperationType::Delete: type_str = "delete"; break;
                    }
                    std::cout << "{\"success\": true"
                              << ", \"type\": \"" << type_str << "\""
                              << ", \"source\": \"" << fo::core::Exporter::json_escape(undone->source_path) << "\""
                              << ", \"dest\": \"" << fo::core::Exporter::json_escape(undone->dest_path) << "\""
                              << "}\n";
                } else {
                    std::cout << "{\"success\": false, \"message\": \"No operations to undo\"}\n";
                }
            } else if (undone) {
                std::cout << "Undone: ";
                switch (undone->type) {
                    case fo::core::OperationType::Move:
                        std::cout << "move " << undone->dest_path << " -> " << undone->source_path;
                        break;
                    case fo::core::OperationType::Copy:
                        std::cout << "copy (deleted " << undone->dest_path << ")";
                        break;
                    case fo::core::OperationType::Rename:
                        std::cout << "rename " << undone->dest_path << " -> " << undone->source_path;
                        break;
                    case fo::core::OperationType::Delete:
                        std::cout << "delete (cannot restore)";
                        break;
                }
                std::cout << "\n";
            } else {
                std::cout << "No operations to undo.\n";
            }

        } else if (command == "history") {
            fo::core::OperationRepository op_repo(engine.database());
            auto ops = op_repo.get_all(50);

            if (format == "json") {
                std::cout << "[\n";
                for (size_t i = 0; i < ops.size(); ++i) {
                    const auto& op = ops[i];
                    auto t = std::chrono::system_clock::to_time_t(op.timestamp);
                    std::string type_str;
                    switch (op.type) {
                        case fo::core::OperationType::Move: type_str = "move"; break;
                        case fo::core::OperationType::Copy: type_str = "copy"; break;
                        case fo::core::OperationType::Rename: type_str = "rename"; break;
                        case fo::core::OperationType::Delete: type_str = "delete"; break;
                    }
                    std::ostringstream ts;
                    ts << std::put_time(std::localtime(&t), "%Y-%m-%dT%H:%M:%S");
                    std::cout << "  {\"id\": " << op.id
                              << ", \"type\": \"" << type_str << "\""
                              << ", \"source\": \"" << fo::core::Exporter::json_escape(op.source_path) << "\""
                              << ", \"dest\": \"" << fo::core::Exporter::json_escape(op.dest_path) << "\""
                              << ", \"timestamp\": \"" << ts.str() << "\""
                              << ", \"undone\": " << (op.undone ? "true" : "false")
                              << "}" << (i + 1 < ops.size() ? "," : "") << "\n";
                }
                std::cout << "]\n";
            } else if (ops.empty()) {
                std::cout << "No operation history.\n";
            } else {
                std::cout << "Operation History (most recent first):\n";
                std::cout << std::string(80, '-') << "\n";
                for (const auto& op : ops) {
                    auto t = std::chrono::system_clock::to_time_t(op.timestamp);
                    std::string type_str;
                    switch (op.type) {
                        case fo::core::OperationType::Move: type_str = "MOVE"; break;
                        case fo::core::OperationType::Copy: type_str = "COPY"; break;
                        case fo::core::OperationType::Rename: type_str = "RENAME"; break;
                        case fo::core::OperationType::Delete: type_str = "DELETE"; break;
                    }
                    std::cout << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S") << " "
                              << std::setw(8) << type_str << " "
                              << (op.undone ? "[UNDONE] " : "")
                              << op.source_path;
                    if (!op.dest_path.empty()) {
                        std::cout << " -> " << op.dest_path;
                    }
                    std::cout << "\n";
                }
            }

        } else {
            std::cerr << "Unknown command: " << command << "\n";
            return 1;
        }

        if (show_time) {
            auto timer_end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timer_end - timer_start).count();
            if (elapsed < 1000) {
                std::cerr << "\nElapsed: " << elapsed << "ms\n";
            } else {
                double secs = elapsed / 1000.0;
                std::cerr << "\nElapsed: " << std::fixed << std::setprecision(2) << secs << "s\n";
            }
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 3;
    }
}
