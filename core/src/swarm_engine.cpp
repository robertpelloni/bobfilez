/// @file swarm_engine.cpp
/// @brief Implementation of the Distributed Swarm Synchronization system.

#include "fo/core/swarm_engine_interface.hpp"
#include "fo/core/registry.hpp"
#include <iostream>
#include <thread>
#include <map>
#include <set>
#include <mutex>
#include <queue>
#include <atomic>

namespace fo::core {

// Simulated Central Hub for Swarm routing
class SwarmHub {
    std::recursive_mutex mtx_;
    std::set<ISwarmEngine*> peers_;
public:
    static SwarmHub& instance() {
        static SwarmHub h;
        return h;
    }

    void register_peer(ISwarmEngine* p) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        peers_.insert(p);
        std::cout << "[SwarmHub] Registered peer " << p << ". Total peers: " << peers_.size() << "\n";
    }

    void unregister_peer(ISwarmEngine* p) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        peers_.erase(p);
        std::cout << "[SwarmHub] Unregistered peer " << p << ". Remaining peers: " << peers_.size() << "\n";
    }

    void broadcast(ISwarmEngine* sender, const std::string& file_id) {
        std::lock_guard<std::recursive_mutex> lock(mtx_);
        int sent_count = 0;
        for (auto* peer : peers_) {
            if (peer != sender) {
                peer->on_remote_update(file_id);
                sent_count++;
            }
        }
        std::cout << "[SwarmHub] Broadcasted update for " << file_id << " to " << sent_count << " peers (Total in hub: " << peers_.size() << ", Sender: " << sender << ")\n";
    }
};

class SwarmEngineImpl : public ISwarmEngine {
    std::map<std::string, SwarmNode> nodes_;
    bool is_discovering_ = false;
    std::map<uint64_t, std::function<void(const std::string&)>> update_callbacks_;
    std::mutex cb_mtx_;
    std::atomic<uint64_t> next_handle_{1};

public:
    SwarmEngineImpl() {
        SwarmHub::instance().register_peer(this);
    }

    ~SwarmEngineImpl() override {
        SwarmHub::instance().unregister_peer(this);
    }

    void on_remote_update(const std::string& file_id) override {
        std::lock_guard<std::mutex> lock(cb_mtx_);
        for (auto const& [handle, cb] : update_callbacks_) {
            if (cb) cb(file_id);
        }
    }

    void start_discovery() override {
        if (is_discovering_) return;
        is_discovering_ = true;
        std::cout << "[Swarm] Discovery service started (mDNS/UDP). Looking for peers...\n";
        
        // Mock discovery
        SwarmNode n;
        n.id = "node-01-nas"; n.hostname = "HOME-NAS"; n.ip_address = "192.168.1.50";
        n.port = 3131; n.is_online = true;
        nodes_[n.id] = n;
    }

    bool connect_to_peer(const std::string& address) override {
        std::cout << "[Swarm] Connecting to peer: " << address << "\n";
        return true;
    }

    void broadcast_index_update(const std::string& file_id) override {
        if (file_id.empty()) {
            std::cout << "[Swarm] Broadcasting full local index updates to " << nodes_.size() << " peers.\n";
        } else {
            // In simulation, we broadcast to the hub even if local node discovery isn't active
            SwarmHub::instance().broadcast(this, file_id);
        }
    }

    void sync_ledger() override {
        std::cout << "[Swarm] Synchronizing Forensic Audit Ledger with the swarm.\n";
    }

    std::vector<SwarmNode> get_active_nodes() override {
        std::vector<SwarmNode> res;
        for (auto const& [id, node] : nodes_) res.push_back(node);
        return res;
    }

    uint64_t subscribe_to_updates(std::function<void(const std::string&)> callback) override {
        std::lock_guard<std::mutex> lock(cb_mtx_);
        uint64_t handle = next_handle_++;
        update_callbacks_[handle] = std::move(callback);
        return handle;
    }

    void unsubscribe(uint64_t handle) override {
        std::lock_guard<std::mutex> lock(cb_mtx_);
        update_callbacks_.erase(handle);
    }
};

class SwarmEngineStub : public ISwarmEngine {
public:
    void on_remote_update(const std::string&) override {}
    void start_discovery() override {}
    bool connect_to_peer(const std::string&) override { return true; }
    void broadcast_index_update(const std::string& file_id) override {
        SwarmHub::instance().broadcast(this, file_id);
    }
    void sync_ledger() override {}
    std::vector<SwarmNode> get_active_nodes() override { return {}; }
    uint64_t subscribe_to_updates(std::function<void(const std::string&)>) override { return 0; }
    void unsubscribe(uint64_t) override {}
};

static bool reg_swarm = []() {
    Registry<ISwarmEngine>::instance().add("default", []() {
        return std::make_unique<SwarmEngineImpl>();
    });
    return true;
}();

void register_swarm_engine() { (void)reg_swarm; }

} // namespace fo::core
