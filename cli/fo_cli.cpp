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
              << "  export       Export scan results to JSON/CSV/HTML\n"
              << "  undo         Undo the last file operation\n"
              << "  history      Show operation history\n"
              << "\nOptions:\n"
              << "  --scanner=<name>    Select scanner (e.g., std, win32, dirent)\n"
              << "  --hasher=<name>     Select hasher (e.g., fast64, blake3)\n"
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

int main(int argc, char** argv) {
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
    bool dry_run = false;
    bool prune = false;
    bool include_thumbnails = false;
    int threshold = 10;
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
        } else if (!a.empty() && a[0] == '-') {
            std::cerr << "Unknown option: " << a << "\n";
            return 2;
        } else {
            roots.emplace_back(a);
        }
    }

    try {
        fo::core::Engine engine(cfg);

        if (command == "scan") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            if (format == "json") {
                std::cout << "[\n";
                for (size_t i = 0; i < files.size(); ++i) {
                    std::cout << "  {\"path\": \"" << fo::core::Exporter::json_escape(files[i].path.string())
                              << "\", \"size\": " << files[i].size << "}";
                    if (i + 1 < files.size()) std::cout << ",";
                    std::cout << "\n";
                }
                std::cout << "]\n";
            } else {
                for (const auto& f : files) {
                    std::cout << f.path.string() << "\n";
                }
            }
        } else if (command == "duplicates") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            auto groups = engine.find_duplicates(files);
            if (format == "json") {
                std::cout << "[\n";
                for (size_t i = 0; i < groups.size(); ++i) {
                    const auto& g = groups[i];
                    std::cout << "  {\"size\": " << g.size << ", \"hash\": \"" << g.fast64 << "\", \"files\": [\n";
                    for (size_t j = 0; j < g.files.size(); ++j) {
                        std::cout << "    \"" << fo::core::Exporter::json_escape(g.files[j].path.string()) << "\"";
                        if (j + 1 < g.files.size()) std::cout << ",";
                        std::cout << "\n";
                    }
                    std::cout << "  ]}";
                    if (i + 1 < groups.size()) std::cout << ",";
                    std::cout << "\n";
                }
                std::cout << "]\n";
            } else {
                for (const auto& g : groups) {
                    std::cout << "== size=" << g.size << ", fast64=" << g.fast64 << "\n";
                    for (const auto& f : g.files) {
                        std::cout << "  " << f.path.string() << "\n";
                    }
                }
            }
        } else if (command == "hash") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            auto& hasher = engine.hasher();
            if (format == "json") {
                std::cout << "[\n";
                for (size_t i = 0; i < files.size(); ++i) {
                    std::string h = hasher.fast64(files[i].path);
                    std::cout << "  {\"path\": \"" << fo::core::Exporter::json_escape(files[i].path.string())
                              << "\", \"hash\": \"" << h << "\"}";
                    if (i + 1 < files.size()) std::cout << ",";
                    std::cout << "\n";
                    if (files[i].id != 0) {
                        engine.file_repository().add_hash(files[i].id, hasher.name(), h);
                    }
                }
                std::cout << "]\n";
            } else {
                for (const auto& f : files) {
                    std::string h = hasher.fast64(f.path);
                    std::cout << h << "  " << f.path.string() << "\n";
                    if (f.id != 0) {
                        engine.file_repository().add_hash(f.id, hasher.name(), h);
                    }
                }
            }
        } else if (command == "metadata") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
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
                    if (provider->read(f.path, meta)) {
                        if (!first) std::cout << ",\n";
                        first = false;
                        std::cout << "  {\"path\": \"" << fo::core::Exporter::json_escape(f.path.string()) << "\"";
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
                    if (provider->read(f.path, meta)) {
                        std::cout << f.path.string() << ":\n";
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
            auto provider = fo::core::Registry<fo::core::IOCRProvider>::instance().create("tesseract");
            if (!provider) {
                std::cerr << "OCR provider 'tesseract' not found.\n";
                return 1;
            }
            for (const auto& f : files) {
                auto result = provider->recognize(f.path, lang);
                if (result) {
                    std::cout << f.path.string() << ":\n";
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
                                  << ", \"path\": \"" << fo::core::Exporter::json_escape(fi->path.string()) << "\""
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
                        std::cout << "  " << fi->path.string() << "\n";
                    }
                }
            }
        } else if (command == "classify") {
            auto files = engine.scan(roots, exts, follow_symlinks, prune);
            auto provider = fo::core::Registry<fo::core::IImageClassifier>::instance().create("onnx");
            if (!provider) {
                std::cerr << "Classifier 'onnx' not found.\n";
                return 1;
            }

            if (format == "json") {
                std::cout << "[\n";
                bool first_file = true;
                for (const auto& f : files) {
                    auto results = provider->classify(f.path);
                    if (!results.empty()) {
                        if (!first_file) std::cout << ",\n";
                        first_file = false;
                        std::cout << "  {\"path\": \"" << fo::core::Exporter::json_escape(f.path.string()) << "\""
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
                    auto results = provider->classify(f.path);
                    if (!results.empty()) {
                        std::cout << f.path.string() << ":\n";
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
                if (new_path_opt && *new_path_opt != f.path) {
                    auto new_path = *new_path_opt;
                    moves.push_back({f.path.string(), new_path.string()});
                    if (!dry_run) {
                        try {
                            std::filesystem::create_directories(new_path.parent_path());
                            std::filesystem::rename(f.path, new_path);

                            fo::core::OperationRepository op_repo(engine.database());
                            fo::core::OperationRecord rec;
                            rec.timestamp = std::chrono::system_clock::now();
                            rec.type = fo::core::OperationType::Move;
                            rec.source_path = f.path.string();
                            rec.dest_path = new_path.string();
                            rec.file_size = f.size;
                            op_repo.log_operation(rec);
                        } catch (const std::exception& e) {
                            std::cerr << "Failed to move " << f.path.string() << ": " << e.what() << "\n";
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
                    if (keep_strategy == "shortest") return a.path.string().length() < b.path.string().length();
                    if (keep_strategy == "longest") return a.path.string().length() > b.path.string().length();
                    return a.mtime < b.mtime;
                });

                const auto& keep = members[0];
                kept_count++;
                std::vector<std::string> deleted_paths;

                for (size_t i = 1; i < members.size(); ++i) {
                    const auto& del = members[i];
                    deleted_paths.push_back(del.path.string());
                    if (!dry_run) {
                        try {
                            std::filesystem::remove(del.path);
                            deleted_count++;
                            fo::core::OperationRepository op_repo(engine.database());
                            fo::core::OperationRecord rec;
                            rec.timestamp = std::chrono::system_clock::now();
                            rec.type = fo::core::OperationType::Delete;
                            rec.source_path = del.path.string();
                            rec.file_size = del.size;
                            op_repo.log_operation(rec);
                        } catch (const std::exception& e) {
                            std::cerr << "Failed to delete: " << e.what() << "\n";
                        }
                    }
                }
                results.push_back({keep.path.string(), deleted_paths});
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
                if (new_path_opt && *new_path_opt != f.path) {
                    auto new_path = *new_path_opt;
                    renames.push_back({f.path.string(), new_path.string()});
                    if (!dry_run) {
                        try {
                            std::filesystem::rename(f.path, new_path);
                            fo::core::OperationRepository op_repo(engine.database());
                            fo::core::OperationRecord rec;
                            rec.timestamp = std::chrono::system_clock::now();
                            rec.type = fo::core::OperationType::Rename;
                            rec.source_path = f.path.string();
                            rec.dest_path = new_path.string();
                            rec.file_size = f.size;
                            op_repo.log_operation(rec);
                        } catch (const std::exception& e) {
                            std::cerr << "Failed to rename " << f.path.string() << ": " << e.what() << "\n";
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

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 3;
    }
}
