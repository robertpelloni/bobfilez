#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include "fo/core/rule_engine.hpp"

// libFuzzer entry point
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    if (Size == 0) return 0;

    // Convert fuzzed data to string for the template
    std::string fuzzed_template(reinterpret_cast<const char*>(Data), Size);

    fo::core::RuleEngine engine;
    fo::core::OrganizationRule rule{"FuzzRule", "FUZZ_TAG", fuzzed_template};
    engine.add_rule(rule);

    fo::core::FileInfo file;
    file.path = "C:/dummy/test/image.jpg";
    file.size = 1048576;
    
    std::vector<std::string> tags = {"FUZZ_TAG"};

    try {
        // This will invoke evaluate which eventually calls expand_template
        engine.apply_rules(file, tags);
    } catch (...) {
        // Exception throwing from malformed templates is expected and safe
    }

    return 0;
}
