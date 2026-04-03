/// @file semantic_tagging.cpp
/// @brief Implementation of the AI Semantic Tagging system.

#include "fo/core/semantic_tagging_interface.hpp"
#include "fo/core/neural_bridge_interface.hpp"
#include "fo/core/registry.hpp"
#include <iostream>

namespace fo::core {

class SemanticTaggingEngineImpl : public ISemanticTaggingEngine {
public:
    std::vector<SemanticTag> analyze_file(const std::filesystem::path& path) override {
        std::vector<SemanticTag> tags;
        auto bridge = Registry<INeuralBridge>::instance().create("default");
        if (!bridge) return tags;

        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".jpg" || ext == ".png" || ext == ".webp") {
            // Image Classification
            auto res = bridge->classifier()->classify(path);
            for (const auto& c : res) {
                tags.push_back({c.label, c.confidence, "MobileNet"});
            }
        } else if (ext == ".pdf" || ext == ".docx" || ext == ".txt") {
            // Document Semantic Analysis
            auto emb = bridge->bert()->embed_file(path);
            if (emb) {
                tags.push_back({"Document", 1.0, "BERT"});
                // Logic to map embedding to known categories
            }
        }

        return tags;
    }

    std::map<std::filesystem::path, std::vector<SemanticTag>> bulk_analyze(const std::vector<std::filesystem::path>& files) override {
        std::map<std::filesystem::path, std::vector<SemanticTag>> results;
        for (const auto& f : files) results[f] = analyze_file(f);
        return results;
    }

    void learn_from_user(const std::filesystem::path&, const std::string&) override {}
};

static auto reg = []() {
    Registry<ISemanticTaggingEngine>::instance().add("default", []() {
        return std::make_unique<SemanticTaggingEngineImpl>();
    });
    return true;
}();

} // namespace fo::core
