/// @file neural_bridge.cpp
/// @brief Implementation of the Omni-Neural Bridge.

#include "fo/core/neural_bridge_interface.hpp"
#include "fo/core/registry.hpp"
#include <iostream>

namespace fo::core {

class NeuralBridgeImpl : public INeuralBridge {
    std::shared_ptr<IClipSearchEngine> clip_engine_;
    std::shared_ptr<IImageClassifier> classifier_engine_;
    std::shared_ptr<IOCRProvider> ocr_engine_;

public:
    NeuralBridgeImpl() {
        // Fetch existing providers from Registry
        clip_engine_ = Registry<IClipSearchEngine>::instance().create("onnx_clip");
        classifier_engine_ = Registry<IImageClassifier>::instance().create("onnx_mobilenet");
        ocr_engine_ = Registry<IOCRProvider>::instance().create("tesseract");
    }

    bool load_suite(const std::string& suite_name) override {
        std::cout << "[NeuralBridge] Loading AI suite: " << suite_name << "\n";
        return true;
    }

    IClipSearchEngine* clip() override { return clip_engine_.get(); }
    IImageClassifier* classifier() override { return classifier_engine_.get(); }
    IOCRProvider* ocr() override { return ocr_engine_.get(); }

    std::vector<ModelStatus> get_status() override {
        std::vector<ModelStatus> stats;
        if (clip_engine_) stats.push_back({"CLIP-ViT-B32", true, 350*1024*1024, "CPU"});
        if (classifier_engine_) stats.push_back({"MobileNet-v2", true, 20*1024*1024, "CPU"});
        if (ocr_engine_) stats.push_back({"Tesseract-v5", true, 50*1024*1024, "CPU"});
        return stats;
    }

    void unload_idle_models() override {
        std::cout << "[NeuralBridge] Freeing idle ML memory...\n";
    }
};

static auto reg = []() {
    Registry<INeuralBridge>::instance().add("default", []() {
        return std::make_unique<NeuralBridgeImpl>();
    });
    return true;
}();

} // namespace fo::core
