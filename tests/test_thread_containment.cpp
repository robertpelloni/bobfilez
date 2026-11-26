// Worker-thread exception containment (Claude Round-12
// follow-up, checks 1-3): the CLI catch-all and the C API
// catch(...) boundaries only protect their own call stacks.
// An exception escaping a std::thread body is std::terminate,
// not a clean error. batch_convert runs converters on worker
// threads; a throwing converter must degrade to per-file
// failed results, never kill the process.
#include <gtest/gtest.h>

#include "fo/core/conversion_interface.hpp"

#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace {

class ThrowingConverter : public fo::core::IConverter {
public:
    std::string name() const override { return "thrower"; }
    std::string description() const override { return "always throws"; }
    std::vector<std::string> supported_input_extensions() const override {
        return {"bin"};
    }
    std::vector<std::string> supported_output_extensions(
        const std::string&) const override {
        return {"out"};
    }
    std::vector<fo::core::ConvertOption> available_options(
        const std::string&, const std::string&) const override {
        return {};
    }
    fo::core::ConversionResult convert(
        const std::filesystem::path& input,
        const std::filesystem::path&,
        const std::map<std::string, std::string>&) override {
        throw std::runtime_error("backend exploded on " + input.string());
    }
};

} // namespace

TEST(ThreadContainment, BatchConvertThrowingBackendFailsPerFile) {
    auto dir = std::filesystem::temp_directory_path() / "bobfilez-thread-containment";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    std::filesystem::create_directories(dir);
    std::vector<std::filesystem::path> inputs;
    for (int i = 0; i < 3; ++i) {
        auto p = dir / ("f" + std::to_string(i) + ".bin");
        std::ofstream(p) << "x";
        inputs.push_back(p);
    }

    fo::core::ConversionEngine engine;
    engine.register_converter(std::make_shared<ThrowingConverter>());
    // num_threads=2 forces the worker-thread path. Survival of
    // this call at all is the primary assertion: an escaping
    // exception would std::terminate the whole test binary.
    auto results = engine.batch_convert(inputs, "out", dir / "out", {}, 2, nullptr);
    ASSERT_EQ(results.size(), inputs.size());
    for (const auto& r : results) {
        EXPECT_FALSE(r.success);
        EXPECT_NE(r.error.find("backend exploded"), std::string::npos) << r.error;
    }
    std::filesystem::remove_all(dir, ec);
}

TEST(ThreadContainment, BatchConvertThrowingBackendSingleThreadAlsoContained) {
    // The sequential path shares the same per-item contract.
    auto dir = std::filesystem::temp_directory_path() / "bobfilez-thread-containment-1t";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    std::filesystem::create_directories(dir);
    auto p = dir / "only.bin";
    { std::ofstream(p) << "x"; }
    fo::core::ConversionEngine engine;
    engine.register_converter(std::make_shared<ThrowingConverter>());
    auto results = engine.batch_convert({p}, "out", dir / "out", {}, 1, nullptr);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_FALSE(results[0].success);
    EXPECT_FALSE(results[0].error.empty());
    std::filesystem::remove_all(dir, ec);
}
