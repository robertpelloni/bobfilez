/// @file omni_graph.cpp
/// @brief Implementation of the Semantic Knowledge Graph Engine.

#include "fo/core/omni_graph_interface.hpp"
#include "fo/core/registry.hpp"
#include <iostream>
#include <map>
#include <set>
#include <algorithm>

namespace fo::core {

class OmniGraphImpl : public IOmniGraph {
    std::map<std::string, GraphNode> nodes_;
    std::vector<GraphEdge> edges_;

public:
    OmniGraphImpl() {
        // Initialize with a mock semantic map (for the QML visualization)
        // In a real scenario, this would be populated by the Semantic Tagging Engine 
        // scanning files and identifying entities (NLP & Vision).
        upsert_node({"file_1", NodeType::File, "trip_budget.xlsx", "document", 1.2f});
        upsert_node({"file_2", NodeType::File, "paris_hotel_receipt.pdf", "document", 1.0f});
        upsert_node({"file_3", NodeType::File, "img_4882.jpg", "image", 1.5f});
        upsert_node({"file_4", NodeType::File, "eiffel_tower_vid.mp4", "video", 2.0f});
        
        upsert_node({"loc_1", NodeType::Location, "Paris, France", "location", 2.5f});
        upsert_node({"person_1", NodeType::Person, "Alice", "person", 3.0f});
        upsert_node({"concept_1", NodeType::Concept, "Vacation", "concept", 2.8f});
        
        add_edge("file_2", "loc_1", "LOCATED_IN", 1.0f);
        add_edge("file_3", "loc_1", "LOCATED_IN", 0.95f);
        add_edge("file_4", "loc_1", "LOCATED_IN", 0.99f);
        
        add_edge("file_3", "person_1", "DEPICTS", 0.88f);
        add_edge("file_4", "person_1", "DEPICTS", 0.91f);
        
        add_edge("file_1", "concept_1", "MENTIONS", 0.85f);
        add_edge("file_2", "concept_1", "MENTIONS", 0.70f);
        add_edge("loc_1", "concept_1", "RELATED_TO", 0.60f);
    }

    void upsert_node(const GraphNode& node) override {
        nodes_[node.id] = node;
    }

    void add_edge(const std::string& src, const std::string& tgt, const std::string& rel, float weight) override {
        edges_.push_back({src, tgt, rel, weight});
    }

    std::vector<GraphNode> get_connected_nodes(const std::string& node_id, int max_depth) override {
        std::vector<GraphNode> result;
        std::set<std::string> seen;
        // Simple BFS for connections (omitted for brevity)
        return result;
    }

    std::vector<GraphEdge> find_path(const std::string& start_id, const std::string& end_id) override {
        // Dijkstra's algorithm (omitted for brevity)
        return {};
    }

    std::pair<std::vector<GraphNode>, std::vector<GraphEdge>> export_topology() override {
        std::vector<GraphNode> nl;
        for (auto const& [id, n] : nodes_) nl.push_back(n);
        return {nl, edges_};
    }
};

static auto reg = []() {
    Registry<IOmniGraph>::instance().add("default", []() {
        return std::make_unique<OmniGraphImpl>();
    });
    return true;
}();

} // namespace fo::core
