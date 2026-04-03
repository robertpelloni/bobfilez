#pragma once
/// @file omni_graph_interface.hpp
/// @brief Semantic Knowledge Graph Engine for bobfilez.
///
/// Breaks free from rigid folder hierarchies by mapping the relationships
/// between files, people, locations, and concepts. 
/// Powered by the Omni-Neural Bridge (NLP + Vision).

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace fo::core {

enum class NodeType {
    File,
    Directory,
    Person,
    Location,
    Event,
    Concept
};

struct GraphNode {
    std::string id;          // UUID or File Path
    NodeType type;
    std::string label;       // e.g., "Paris", "Alice", "receipt.pdf"
    std::string group;       // For UI color coding
    float importance;        // Node size in visualization
};

struct GraphEdge {
    std::string source_id;
    std::string target_id;
    std::string relationship; // e.g., "DEPICTS", "LOCATED_IN", "MENTIONS", "SIMILAR_TO"
    float weight;            // 0.0 to 1.0 (Strength of connection)
};

class IOmniGraph {
public:
    virtual ~IOmniGraph() = default;

    /// Add or update a node in the knowledge graph
    virtual void upsert_node(const GraphNode& node) = 0;

    /// Establish a relationship between two nodes
    virtual void add_edge(const std::string& source_id, const std::string& target_id, const std::string& relation, float weight) = 0;

    /// Find all nodes connected to a specific entity (e.g., Everything related to "Alice")
    virtual std::vector<GraphNode> get_connected_nodes(const std::string& node_id, int max_depth = 1) = 0;

    /// Query the graph to find the shortest relationship path between two files
    virtual std::vector<GraphEdge> find_path(const std::string& start_id, const std::string& end_id) = 0;

    /// Export the graph topology for the QML UI renderer
    virtual std::pair<std::vector<GraphNode>, std::vector<GraphEdge>> export_topology() = 0;
};

} // namespace fo::core
