#pragma once
/// @file omnicluster_interface.hpp
/// @brief Distributed Grid Computing Engine for bobfilez.
///
/// OmniCluster leverages the P2P Data Swarm to distribute heavy workloads
/// (FFmpeg transcoding, Hashcat cracking, Tesseract OCR) across multiple
/// machines on the local network, drastically reducing processing time.

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace fo::core {

struct ComputeNode {
    std::string node_id;
    std::string hostname;
    int cpu_cores;
    std::string gpu_model;
    double current_cpu_load;
    double current_gpu_load;
    bool is_idle;
};

struct ClusterTask {
    std::string task_id;
    std::string task_type;      // "FFmpeg_Transcode", "Hashcat_Crack", "Batch_OCR"
    std::string target_file;
    int total_chunks;
    int completed_chunks;
    double overall_progress;    // 0.0 to 1.0
    double network_throughput_mbps;
};

class IOmniClusterEngine {
public:
    virtual ~IOmniClusterEngine() = default;

    /// Discover and register compute nodes from the active Data Swarm
    virtual std::vector<ComputeNode> get_available_nodes() = 0;

    /// Dispatch a video rendering job across the cluster
    virtual std::string dispatch_distributed_render(const std::filesystem::path& project_file, const std::filesystem::path& output_path) = 0;

    /// Dispatch a password cracking job across the cluster's GPUs
    virtual std::string dispatch_distributed_crack(const std::filesystem::path& hash_file, const std::filesystem::path& wordlist) = 0;

    /// Get the real-time status of all active grid tasks
    virtual std::vector<ClusterTask> get_active_tasks() = 0;
};

} // namespace fo::core
