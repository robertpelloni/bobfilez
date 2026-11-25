#include "fo/core/ignore_repository.hpp"
#include <sqlite3.h>
#include <stdexcept>
#include <regex>

namespace fo::core {

IgnoreRepository::IgnoreRepository(DatabaseManager& db) : db_(db) {}

bool IgnoreRepository::add(const std::string& pattern, const std::string& reason) {
    std::string sql = "INSERT OR IGNORE INTO ignore_list (pattern, reason) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, reason.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    int changed = sqlite3_changes(db_.get_db());
    sqlite3_finalize(stmt);
    return changed > 0;
}

bool IgnoreRepository::remove(const std::string& pattern) {
    std::string sql = "DELETE FROM ignore_list WHERE pattern = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    int changed = sqlite3_changes(db_.get_db());
    sqlite3_finalize(stmt);
    return changed > 0;
}

std::vector<IgnoreRule> IgnoreRepository::get_all() {
    std::vector<IgnoreRule> rules;
    std::string sql = "SELECT id, pattern, reason FROM ignore_list;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return rules;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        IgnoreRule r;
        r.id = sqlite3_column_int64(stmt, 0);
        r.pattern = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* reason = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        if (reason) r.reason = reason;
        rules.push_back(r);
    }
    sqlite3_finalize(stmt);
    return rules;
}

namespace {
// Case-insensitive glob match with '*', '?', and one level of
// '{a,b,c}' alternation. Patterns are globs as every UI teaches
// ("*.log"); treating them as regex made "*.log" an invalid regex
// whose substring fallback could never match, so ignore rules
// silently did nothing.
bool glob_match_ci(const std::string& pattern_in, const std::string& name_in) {
    auto lower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    };
    const std::string name = lower(name_in);
    std::vector<std::string> variants;
    const std::string pattern = lower(pattern_in);
    auto brace_open = pattern.find('{');
    auto brace_close = brace_open == std::string::npos ? std::string::npos
                                                       : pattern.find('}', brace_open);
    if (brace_open != std::string::npos && brace_close != std::string::npos &&
        brace_close > brace_open) {
        std::string head = pattern.substr(0, brace_open);
        std::string tail = pattern.substr(brace_close + 1);
        std::string inner = pattern.substr(brace_open + 1, brace_close - brace_open - 1);
        size_t start = 0;
        while (start <= inner.size()) {
            auto comma = inner.find(',', start);
            if (comma == std::string::npos) comma = inner.size();
            variants.push_back(head + inner.substr(start, comma - start) + tail);
            start = comma + 1;
        }
    } else {
        variants.push_back(pattern);
    }
    for (const auto& pat : variants) {
        size_t p = 0, t = 0, star_p = std::string::npos, star_t = 0;
        while (t < name.size()) {
            if (p < pat.size() && (pat[p] == '?' || pat[p] == name[t])) {
                ++p; ++t;
            } else if (p < pat.size() && pat[p] == '*') {
                star_p = p++; star_t = t;
            } else if (star_p != std::string::npos) {
                p = star_p + 1; t = ++star_t;
            } else {
                break;
            }
        }
        while (p < pat.size() && pat[p] == '*') ++p;
        if (p == pat.size() && t == name.size()) return true;
    }
    return false;
}
} // namespace

bool IgnoreRepository::is_ignored(const std::string& path) {
    auto rules = get_all();
    const std::string filename = std::filesystem::path(path).filename().string();
    for (const auto& r : rules) {
        if (r.pattern.empty()) continue;
        // Match the glob against the filename and the full path, so
        // both "*.log" and "*/temp/*" style patterns work. Patterns
        // without glob characters fall back to substring search.
        if (r.pattern.find_first_of("*?{") != std::string::npos) {
            if (glob_match_ci(r.pattern, filename) || glob_match_ci(r.pattern, path)) {
                return true;
            }
        } else if (path.find(r.pattern) != std::string::npos) {
            return true;
        }
    }
    return false;
}

} // namespace fo::core
