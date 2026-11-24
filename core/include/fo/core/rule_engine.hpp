#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include "fo/core/types.hpp"

namespace fo::core {

struct OrganizationRule {
    std::string name;
    std::string filter_tag; // Simple filter: requires this tag
    std::string destination_template; // e.g. "/Photos/{year}/{month}/"
    // Optional filename glob (e.g. "*.{cr2,nef}" or "access*.log");
    // empty = all files. Declared last so existing aggregate
    // initializers {name, filter_tag, destination} keep meaning.
    std::string pattern;
};

class RuleEngine {
public:
    RuleEngine();
    
    void add_rule(const OrganizationRule& rule);
    
    // Load rules from a YAML file
    // Returns true on success
    bool load_rules_from_yaml(const std::filesystem::path& yaml_path);

    // Returns the new path if a rule matches, otherwise std::nullopt
    std::optional<std::filesystem::path> apply_rules(const FileInfo& file, const std::vector<std::string>& tags);

private:
    std::vector<OrganizationRule> rules_;
    
    std::string expand_template(const std::string& tmpl, const FileInfo& file, const std::vector<std::string>& tags);
};

} // namespace fo::core
