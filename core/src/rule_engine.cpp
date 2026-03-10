#include "fo/core/rule_engine.hpp"
#include <regex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

#ifdef FO_HAVE_YAMLCPP
#include <yaml-cpp/yaml.h>
#endif

namespace fo::core {

RuleEngine::RuleEngine() {}

void RuleEngine::add_rule(const OrganizationRule& rule) {
    rules_.push_back(rule);
}

bool RuleEngine::load_rules_from_yaml(const std::filesystem::path& yaml_path) {
#ifdef FO_HAVE_YAMLCPP
    try {
        YAML::Node config = YAML::LoadFile(yaml_path.string());
        if (config["rules"]) {
            for (const auto& node : config["rules"]) {
                OrganizationRule rule;
                if (node["name"]) rule.name = node["name"].as<std::string>();
                if (node["filter_tag"]) rule.filter_tag = node["filter_tag"].as<std::string>();
                if (node["destination"]) rule.destination_template = node["destination"].as<std::string>();
                
                if (!rule.destination_template.empty()) {
                    add_rule(rule);
                }
            }
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading YAML rules: " << e.what() << "\n";
    }
    return false;
#else
    (void)yaml_path;
    std::cerr << "YAML support not enabled. Please build with yaml-cpp.\n";
    return false;
#endif
}

std::optional<std::filesystem::path> RuleEngine::apply_rules(const FileInfo& file, const std::vector<std::string>& tags) {
    for (const auto& rule : rules_) {
        // Check filter
        bool match = false;
        if (rule.filter_tag.empty()) {
            match = true;
        } else {
            for (const auto& tag : tags) {
                if (tag == rule.filter_tag) {
                    match = true;
                    break;
                }
            }
        }

        if (match) {
            std::string expanded = expand_template(rule.destination_template, file, tags);
            std::filesystem::path new_path(expanded);
            
            // If template ends with separator, append filename
            char last = rule.destination_template.back();
            if (last == '/' || last == '\\') {
                std::filesystem::path uri_path(file.uri);
                new_path /= uri_path.filename();
            }
            
            return new_path;
        }
    }
    return std::nullopt;
}

std::string RuleEngine::expand_template(const std::string& tmpl, const FileInfo& file, const std::vector<std::string>& tags) {
    std::string result = tmpl;
    
    // Extract date
    // Portable conversion from file_time_type to system_clock is messy in C++20 until C++23
    // We use a rough approximation
    auto ftime = file.mtime;
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - std::chrono::file_clock::now() + std::chrono::system_clock::now()
    );
    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
    std::tm* gmt = std::gmtime(&tt);

    if (gmt) {
        std::stringstream year_ss; year_ss << (gmt->tm_year + 1900);
        std::stringstream month_ss; month_ss << std::setw(2) << std::setfill('0') << (gmt->tm_mon + 1);
        std::stringstream day_ss; day_ss << std::setw(2) << std::setfill('0') << gmt->tm_mday;

        auto replace_all = [&](std::string& str, const std::string& from, const std::string& to) {
            size_t start_pos = 0;
            while((start_pos = str.find(from, start_pos)) != std::string::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }
        };

        replace_all(result, "{year}", year_ss.str());
        replace_all(result, "{month}", month_ss.str());
        replace_all(result, "{day}", day_ss.str());
    }

    auto replace_all = [&](std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    };

    std::filesystem::path uri_path(file.uri);
    if (uri_path.has_extension()) {
        replace_all(result, "{ext}", uri_path.extension().string().substr(1));
    }
    replace_all(result, "{name}", uri_path.stem().string());
    replace_all(result, "{parent}", uri_path.parent_path().string());

    return result;
}

} // namespace fo::core
