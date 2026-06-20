#pragma once

#include "interfaces.hpp"
#include "registry.hpp"
#include "database.hpp"
#include "file_repository.hpp"
#include "duplicate_repository.hpp"
#include "ignore_repository.hpp"
#include "scan_session_repository.hpp"
#include "omniflow_engine_interface.hpp"
#include "omniclerk_interface.hpp"
#include "omni_oracle_interface.hpp"
#include "autonomous_dev_protocol_interface.hpp"
#include "autonomous_sync_interface.hpp"
#include "nexus_interface.hpp"
#include "swarm_engine_interface.hpp"
#include <memory>

namespace fo::core {

class SearchEngine;

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
    explicit Engine(EngineConfig cfg = {});
    ~Engine();

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

    SearchEngine& search_engine();
    IOmniFlowEngine& omniflow_engine() { return *omniflow_; }
    IOmniClerkEngine& omniclerk_engine() { return *omniclerk_; }
    IOmniOracle& omni_oracle() { return *oracle_; }
    IAutonomousDevProtocol& autodev_protocol() { return *autodev_; }
    IAutonomousSyncService& sync_service() { return *sync_; }
    INexus& nexus() { return *nexus_; }
    ISwarmEngine& swarm_engine() { return *swarm_; }

    struct AutonomousHealth {
        uint64_t total_workflows_executed;
        double avg_workflow_duration;
        uint64_t total_sync_uploads;
        bool last_live_validation_success;
        int active_swarm_nodes;
    };
    AutonomousHealth get_autonomous_health();

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
    SearchEngine* search_engine_ = nullptr;
    
    // Coordination services
    std::shared_ptr<IOmniFlowEngine> omniflow_;
    std::shared_ptr<IOmniClerkEngine> omniclerk_;
    std::shared_ptr<IOmniOracle> oracle_;
    std::shared_ptr<IAutonomousDevProtocol> autodev_;
    std::shared_ptr<IAutonomousSyncService> sync_;
    std::shared_ptr<INexus> nexus_;
    std::shared_ptr<ISwarmEngine> swarm_;
};

} // namespace fo::core
