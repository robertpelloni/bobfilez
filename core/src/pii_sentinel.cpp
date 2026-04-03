/// @file pii_sentinel.cpp
/// @brief Implementation of PII detection logic.

#include "fo/core/pii_sentinel.hpp"
#include <fstream>
#include <iostream>

namespace fo::core {

std::vector<PiiMatch> PIISentinel::scan_file(const std::filesystem::path& path) {
    std::vector<PiiMatch> matches;
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return matches;

    std::string line;
    int64_t line_num = 1;
    while (std::getline(ifs, line)) {
        auto line_matches = scan_string(line);
        for (auto& m : line_matches) {
            m.line_number = line_num;
            matches.push_back(std::move(m));
        }
        line_num++;
        if (matches.size() > 100) break; // Limit per file
    }
    return matches;
}

std::vector<PiiMatch> PIISentinel::scan_string(const std::string& content) {
    std::vector<PiiMatch> matches;
    for (const auto& p : patterns_) {
        std::smatch m;
        std::string search_in = content;
        while (std::regex_search(search_in, m, p.regex)) {
            PiiMatch match;
            match.type = p.name;
            match.snippet = mask(m.str(), p.name);
            match.confidence = p.weight;
            matches.push_back(std::move(match));
            search_in = m.suffix().str();
        }
    }
    return matches;
}

std::string PIISentinel::mask(const std::string& pii, const std::string& type) {
    if (pii.length() < 4) return "****";
    if (type == "SSN") return "XXX-XX-" + pii.substr(pii.length() - 4);
    if (type == "CreditCard") return "****-****-****-" + pii.substr(pii.length() - 4);
    return pii.substr(0, 2) + "********" + pii.substr(pii.length() - 2);
}

} // namespace fo::core
