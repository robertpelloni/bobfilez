#pragma once
/// @file swarm_engine_interface.hpp
/// @brief Multi-device synchronization and Peer-to-Peer file sharing for bobfilez.
///
/// Implements the "Distributed Nexus" - allowing multiple bobfilez nodes
/// (Laptop, Desktop, NAS) to form a "Swarm".
/// Features:
///   - Instant metadata sync (Search local files on other devices)
///   - Remote file pull (Transfer between nodes without cloud)
///   - Decentralized Audit Ledger synchronization
///   - Shared AI training/results

#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace fo::core {

struct SwarmNode {
    std::string id;
    std::string hostname;
    std::string ip_address;
    int port;
    bool is_online;
    int64_t last_seen;
};

class ISwarmEngine {
public:
    virtual ~ISwarmEngine() = default;

    /// Start the node discovery service (mDNS/UDP)
    virtual void start_discovery() = 0;

    /// Connect to a specific peer
    virtual bool connect_to_peer(const std::string& address) = 0;

    /// Broadcast an index update to all online peers
    virtual void broadcast_index_update() = 0;

    /// Sync the forensic audit ledger with the swarm
    virtual void sync_ledger() = 0;

    /// Get list of discovered swarm nodes
    virtual std::vector<SwarmNode> get_active_nodes() = 0;
};

} // namespace fo::core
