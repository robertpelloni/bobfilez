#include "fo/core/classification_interface.hpp"
#include "fo/core/registry.hpp"

#if defined(FO_HAVE_ONNXRUNTIME) && defined(FO_HAVE_OPENCV)
#include <onnxruntime_cxx_api.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <numeric>

namespace fo::core {

class OnnxRuntimeClassifier : public IImageClassifier {
public:
    OnnxRuntimeClassifier() : env_(ORT_LOGGING_LEVEL_WARNING, "filez") {
        // Initialize session options
        session_options_.SetIntraOpNumThreads(1);
        session_options_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_BASIC);
    }

    std::string name() const override { return "onnx"; }

    std::vector<ClassificationResult> classify(const std::filesystem::path& image_path, int top_k) override {
        // Lazy load model if not loaded
        if (!session_) {
            if (!load_model()) return {};
        }

        try {
            // 1. Load and preprocess image
            cv::Mat img = cv::imread(image_path.string());
            if (img.empty()) return {};

            // Resize to 224x224 (standard ImageNet)
            cv::Mat resized;
            cv::resize(img, resized, cv::Size(224, 224));
            
            // Convert to RGB
            cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);
            
            // Convert to float and normalize
            resized.convertTo(resized, CV_32F, 1.0 / 255.0);

            // Subtract mean and divide by std (ImageNet standards)
            // Mean: [0.485, 0.456, 0.406], Std: [0.229, 0.224, 0.225]
            cv::Scalar mean(0.485, 0.456, 0.406);
            cv::Scalar std(0.229, 0.224, 0.225);
            cv::subtract(resized, mean, resized);
            cv::divide(resized, std, resized);

            // HWC to CHW
            std::vector<float> input_tensor_values;
            input_tensor_values.reserve(1 * 3 * 224 * 224);
            std::vector<cv::Mat> channels(3);
            cv::split(resized, channels);
            for (auto& c : channels) {
                input_tensor_values.insert(input_tensor_values.end(), (float*)c.datastart, (float*)c.dataend);
            }

            // 2. Create Tensor
            std::vector<int64_t> input_shape = {1, 3, 224, 224};
            auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
            auto input_tensor = Ort::Value::CreateTensor<float>(
                memory_info, input_tensor_values.data(), input_tensor_values.size(), input_shape.data(), input_shape.size());

            // 3. Run Inference
            const char* input_names[] = {input_name_.c_str()};
            const char* output_names[] = {output_name_.c_str()};
            
            auto output_tensors = session_->Run(
                Ort::RunOptions{nullptr}, input_names, &input_tensor, 1, output_names, 1);

            // 4. Postprocess
            float* floatarr = output_tensors.front().GetTensorMutableData<float>();
            size_t count = output_tensors.front().GetTensorTypeAndShapeInfo().GetElementCount();
            
            // Softmax
            std::vector<float> probs(floatarr, floatarr + count);
            // (Optional: apply softmax if model output is logits. Most are.)
            
            // Get top K
            std::vector<size_t> indices(probs.size());
            std::iota(indices.begin(), indices.end(), 0);
            std::partial_sort(indices.begin(), indices.begin() + top_k, indices.end(),
                              [&probs](size_t i1, size_t i2) { return probs[i1] > probs[i2]; });

            std::vector<ClassificationResult> results;
            for (int i = 0; i < top_k; ++i) {
                size_t idx = indices[i];
                std::string label = (idx < labels_.size()) ? labels_[idx] : std::to_string(idx);
                results.push_back({label, probs[idx]});
            }
            return results;

        } catch (const std::exception& e) {
            std::cerr << "ONNX Inference error: " << e.what() << std::endl;
            return {};
        }
    }

private:
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    std::unique_ptr<Ort::Session> session_;
    std::string input_name_;
    std::string output_name_;
    std::vector<std::string> labels_;

    bool load_model() {
        // Look for model.onnx and labels.txt in current dir or executable dir
        // For now, hardcoded to "model.onnx" in CWD
        std::string model_path = "model.onnx";
        std::string labels_path = "labels.txt";

        if (!std::filesystem::exists(model_path)) {
            std::cerr << "Model file not found: " << model_path << std::endl;
            return false;
        }

        try {
            // Load labels
            std::ifstream f(labels_path);
            if (f.is_open()) {
                std::string line;
                while (std::getline(f, line)) labels_.push_back(line);
            }

            // Load session
            // On Windows, path must be wide string
            std::wstring wpath(model_path.begin(), model_path.end());
            session_ = std::make_unique<Ort::Session>(env_, wpath.c_str(), session_options_);

            // Get input/output names
            Ort::AllocatorWithDefaultOptions allocator;
            auto input_name_ptr = session_->GetInputNameAllocated(0, allocator);
            input_name_ = input_name_ptr.get();
            
            auto output_name_ptr = session_->GetOutputNameAllocated(0, allocator);
            output_name_ = output_name_ptr.get();

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load model: " << e.what() << std::endl;
            return false;
        }
    }
};

namespace {
    struct OnnxRegistrar {
        OnnxRegistrar() {
            Registry<IImageClassifier>::instance().add("onnx", []() {
                return std::make_unique<OnnxRuntimeClassifier>();
            });
        }
    };
    static OnnxRegistrar registrar;
}

} // namespace fo::core

#endif // FO_HAVE_ONNXRUNTIME && FO_HAVE_OPENCV
