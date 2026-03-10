#pragma once

#include "interfaces.hpp"
#include "registry.hpp"
#include "database.hpp"
#include "file_repository.hpp"
#include "duplicate_repository.hpp"
#include "ignore_repository.hpp"
#include "scan_session_repository.hpp"
#include <memory>

namespace fo::core {

struct EngineConfig {
    std::string scanner = "std";
    std::string hasher = "fast64";
    std::string db_path = "fo.db";
    bool use_ads_cache = false;  // Use Windows NTFS Alternate Data Streams for hash caching
    std::string s3_bucket;
    std::string gdrive_token;
    std::string azure_connection_str;
    std::string azure_container;
};

class Engine {
public:
    explicit Engine(EngineConfig cfg = {})
        : cfg_(std::move(cfg))
        , scanner_(Registry<IFileScanner>::instance().create(cfg_.scanner))
        , hasher_(Registry<IHasher>::instance().create(cfg_.hasher))
        , file_repo_(db_manager_)
        , duplicate_repo_(db_manager_)
        , ignore_repo_(db_manager_)
        , session_repo_(db_manager_)
    {
        db_manager_.open(cfg_.db_path);
        db_manager_.migrate();
    }

    std::vector<FileInfo> scan(const std::vector<std::filesystem::path>& roots,
                               const std::vector<std::string>& include_exts,
                               bool follow_symlinks,
                               bool prune = false);

    std::vector<DuplicateGroup> find_duplicates(const std::vector<FileInfo>& files);

    IHasher& hasher() { return *hasher_; }
    FileRepository& file_repository() { return file_repo_; }
    DuplicateRepository& duplicate_repository() { return duplicate_repo_; }
    IgnoreRepository& ignore_repository() { return ignore_repo_; }
    ScanSessionRepository& session_repository() { return session_repo_; }
    DatabaseManager& database() { return db_manager_; }

    bool use_ads_cache() const { return cfg_.use_ads_cache; }

private:
    // local implementation of duplicate finder from dupe_size_fast.cpp
    class SizeHashDuplicateFinder : public IDuplicateFinder {
    public:
        explicit SizeHashDuplicateFinder(bool use_ads = false) : use_ads_(use_ads) {}
        std::string name() const override { return "size+fast64"; }
        std::vector<DuplicateGroup> group(const std::vector<FileInfo>& files, IHasher& hasher) override;
    private:
        bool use_ads_ = false;
    };

    EngineConfig cfg_{};
    std::unique_ptr<IFileScanner> scanner_{};
    std::unique_ptr<IHasher> hasher_{};
    DatabaseManager db_manager_;
    FileRepository file_repo_;
    DuplicateRepository duplicate_repo_;
    IgnoreRepository ignore_repo_;
    ScanSessionRepository session_repo_;
};

} // namespace fo::core
