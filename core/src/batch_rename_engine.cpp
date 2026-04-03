/// @file batch_rename_engine.cpp
/// @brief Implementation of the comprehensive batch file renaming engine.

#include "fo/core/batch_rename_interface.hpp"
#include "fo/core/registry.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <random>
#include <chrono>
#include <unordered_set>
#include <iostream>

namespace fo::core {

//─────────────────────────── Helper Functions ────────────────────────────────

// Simple wildcard to regex converter (e.g. "*.jpg" -> ".*\.jpg")
static std::string wildcard_to_regex(const std::string& wildcard) {
    std::string reg;
    for (char c : wildcard) {
        if (c == '*') reg += ".*";
        else if (c == '?') reg += ".";
        else if (std::string(".+{}[]()|\\^$").find(c) != std::string::npos) {
            reg += "\\";
            reg += c;
        }
        else reg += c;
    }
    return reg;
}

// Check if string contains only ASCII
static bool is_ascii(const std::string& s) {
    return std::all_of(s.begin(), s.end(), [](unsigned char c) { return c < 128; });
}

// To lower/upper (ASCII only for simplicity, robust unicode needs ICU)
static std::string to_lower_ascii(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

static std::string to_upper_ascii(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
    return s;
}

//─────────────────────────── Rule Descriptions ───────────────────────────────

std::string ReplaceRule::description() const {
    return "Replace '" + find + "' with '" + replace + "'" + (use_regex ? " (Regex)" : "");
}

std::string InsertRule::description() const {
    return "Insert '" + text + "' at position " + std::to_string(static_cast<int>(position));
}

std::string DeleteRule::description() const {
    return "Delete " + std::to_string(count) + " chars from index " + std::to_string(start_index);
}

std::string TrimRule::description() const {
    return "Trim '" + (chars.empty() ? "whitespace" : chars) + "'";
}

std::string CaseRule::description() const {
    return "Change case mode " + std::to_string(static_cast<int>(mode));
}

std::string NumberRule::description() const {
    return "Number from " + std::to_string(start) + " step " + std::to_string(step);
}

std::string DateTimeRule::description() const {
    return "Insert date/time (" + format + ")";
}

std::string MetadataRule::description() const {
    return "Insert metadata field: " + field;
}

std::string ExtensionRule::description() const {
    return "Extension mode " + std::to_string(static_cast<int>(mode));
}

std::string TruncateRule::description() const {
    return "Truncate to " + std::to_string(max_length) + " chars";
}

std::string SanitizeRule::description() const {
    return "Sanitize illegal characters to '" + replacement + "'";
}

std::string HashNameRule::description() const {
    return "Replace name with " + algorithm + " hash";
}

std::string TransliterateRule::description() const {
    return "Transliterate Unicode to ASCII";
}

std::string RegexGroupsRule::description() const {
    return "Regex Groups: " + pattern + " -> " + output_template;
}

//─────────────────────────── Rule Apply Methods ──────────────────────────────

std::string ReplaceRule::apply(const std::string& s, int, const std::filesystem::path&, const std::map<std::string, std::string>&) const {
    if (find.empty() || !enabled) return s;

    std::string result = s;
    if (use_regex) {
        try {
            auto flags = std::regex_constants::ECMAScript;
            if (!case_sensitive) flags |= std::regex_constants::icase;
            std::regex re(whole_word ? ("\\b" + find + "\\b") : find, flags);
            
            if (replace_all) {
                result = std::regex_replace(s, re, replace);
            } else {
                result = std::regex_replace(s, re, replace, std::regex_constants::format_first_only);
            }
        } catch (...) {
            // Bad regex, ignore
            return s;
        }
    } else {
        // Plain substring replace
        std::string target = case_sensitive ? find : to_lower_ascii(find);
        std::string search_in = case_sensitive ? result : to_lower_ascii(result);
        
        size_t pos = 0;
        int count = 0;
        while ((pos = search_in.find(target, pos)) != std::string::npos) {
            count++;
            if (!replace_all && count > 1) break;
            if (replace_nth > 0 && count != replace_nth) {
                pos += target.length();
                continue;
            }

            result.replace(pos, find.length(), replace);
            search_in = case_sensitive ? result : to_lower_ascii(result);
            pos += replace.length();
        }
    }
    return result;
}

std::string InsertRule::apply(const std::string& s, int, const std::filesystem::path&, const std::map<std::string, std::string>&) const {
    if (!enabled) return s;
    std::string result = s;
    if (position == Position::Start) {
        result.insert(0, text);
    } else if (position == Position::End) {
        result.append(text);
    } else if (position == Position::AtIndex) {
        int actual_idx = count_from_end ? std::max(0, static_cast<int>(s.length()) - index) : index;
        actual_idx = std::min(actual_idx, static_cast<int>(s.length()));
        result.insert(actual_idx, text);
    } else if (position == Position::BeforePattern || position == Position::AfterPattern) {
        size_t pos = result.find(pattern);
        if (pos != std::string::npos) {
            if (position == Position::AfterPattern) pos += pattern.length();
            result.insert(pos, text);
        }
    }
    return result;
}

std::string DeleteRule::apply(const std::string& s, int, const std::filesystem::path&, const std::map<std::string, std::string>&) const {
    if (!enabled || count <= 0 || s.empty()) return s;
    int len = static_cast<int>(s.length());
    int actual_start = count_from_end ? std::max(0, len - start_index - count) : start_index;
    
    if (actual_start >= len || actual_start < 0) return s;
    int actual_count = std::min(count, len - actual_start);
    
    std::string result = s;
    result.erase(actual_start, actual_count);
    return result;
}

std::string TrimRule::apply(const std::string& s, int, const std::filesystem::path&, const std::map<std::string, std::string>&) const {
    if (!enabled || s.empty()) return s;
    std::string result = s;
    std::string strip = chars.empty() ? " \t\n\r\v\f" : chars;
    
    if (trim_start) {
        size_t start = result.find_first_not_of(strip);
        if (start == std::string::npos) return ""; // All trimmable
        result = result.substr(start);
    }
    if (trim_end) {
        size_t end = result.find_last_not_of(strip);
        if (end != std::string::npos) result = result.substr(0, end + 1);
    }
    return result;
}

std::string CaseRule::apply(const std::string& s, int, const std::filesystem::path&, const std::map<std::string, std::string>&) const {
    if (!enabled || s.empty()) return s;
    std::string result = s;
    
    if (mode == Mode::Lower) {
        result = to_lower_ascii(result);
    } else if (mode == Mode::Upper) {
        result = to_upper_ascii(result);
    } else if (mode == Mode::Toggle) {
        for (char& c : result) {
            if (std::islower(c)) c = std::toupper(c);
            else if (std::isupper(c)) c = std::tolower(c);
        }
    } else if (mode == Mode::TitleCase) {
        bool capitalize_next = true;
        for (char& c : result) {
            if (std::isspace(c) || c == '_' || c == '-') {
                capitalize_next = true;
            } else if (std::isalpha(c)) {
                if (capitalize_next) { c = std::toupper(c); capitalize_next = false; }
                else { c = std::tolower(c); }
            }
        }
    } else if (mode == Mode::SentenceCase) {
        result = to_lower_ascii(result);
        for (char& c : result) {
            if (std::isalpha(c)) { c = std::toupper(c); break; }
        }
    } else if (mode == Mode::snake_case || mode == Mode::KebabCase) {
        char sep = (mode == Mode::snake_case) ? '_' : '-';
        std::string out;
        bool last_was_sep = false;
        for (size_t i = 0; i < result.length(); ++i) {
            char c = result[i];
            if (std::isspace(c) || c == '_' || c == '-') {
                if (!last_was_sep && !out.empty()) { out += sep; last_was_sep = true; }
            } else if (std::isupper(c)) {
                if (!last_was_sep && !out.empty() && i > 0 && std::islower(result[i-1])) {
                    out += sep;
                }
                out += std::tolower(c);
                last_was_sep = false;
            } else {
                out += std::tolower(c);
                last_was_sep = false;
            }
        }
        result = out;
    }
    return result;
}

std::string NumberRule::apply(const std::string& s, int idx, const std::filesystem::path&, const std::map<std::string, std::string>&) const {
    if (!enabled) return s;
    
    // Note: per_folder logic requires state tracking across batch.
    // For now, idx is the raw batch index. A more robust implementation would
    // pass a folder-relative index.
    int current_num = start + (idx * step);
    
    std::ostringstream oss;
    oss << std::setw(pad_width) << std::setfill(pad_char) << current_num;
    std::string num_str = prefix + oss.str() + suffix;
    
    std::string result = s;
    if (position == Position::Start) result.insert(0, num_str);
    else if (position == Position::End) result.append(num_str);
    else if (position == Position::Replace) result = num_str;
    else if (position == Position::AtIndex) {
        int pos = std::min(index_pos, static_cast<int>(result.length()));
        result.insert(pos, num_str);
    }
    return result;
}

std::string DateTimeRule::apply(const std::string& s, int, const std::filesystem::path& p, const std::map<std::string, std::string>& meta) const {
    if (!enabled) return s;
    
    std::time_t t = 0;
    if (source == Source::CurrentTime) {
        t = std::time(nullptr);
    } else if (source == Source::ExifDate) {
        auto it = meta.find("date_taken");
        if (it != meta.end() && !it->second.empty()) {
            // Simplistic: if we have string, just use it. Real parsing needed for format string.
            std::string date_str = it->second;
            std::string result = s;
            if (position == Position::Start) result = date_str + separator + result;
            else if (position == Position::End) result = result + separator + date_str;
            else result = date_str;
            return result;
        }
        // Fallback to modified time
        auto ftime = std::filesystem::last_write_time(p);
        t = std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(ftime));
    } else {
        // Fallback to modified
        auto ftime = std::filesystem::last_write_time(p);
        t = std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(ftime));
    }
    
    std::ostringstream oss;
#ifdef _WIN32
    std::tm tm_buf;
    localtime_s(&tm_buf, &t);
    oss << std::put_time(&tm_buf, format.c_str());
#else
    oss << std::put_time(std::localtime(&t), format.c_str());
#endif
    std::string dstr = oss.str();
    
    std::string result = s;
    if (position == Position::Start) result = dstr + separator + result;
    else if (position == Position::End) result = result + separator + dstr;
    else result = dstr;
    
    return result;
}

std::string MetadataRule::apply(const std::string& s, int, const std::filesystem::path&, const std::map<std::string, std::string>& meta) const {
    if (!enabled) return s;
    
    std::string val = fallback;
    auto it = meta.find(field);
    if (it != meta.end() && !it->second.empty()) val = it->second;
    
    if (val.empty()) return s;
    
    std::string result = s;
    if (position == Position::Start) result = val + separator + result;
    else if (position == Position::End) result = result + separator + val;
    else result = val;
    return result;
}

std::string ExtensionRule::apply(const std::string& s, int, const std::filesystem::path& p, const std::map<std::string, std::string>&) const {
    // Note: This rule typically operates at the engine level (on the full name), 
    // but here we just pass the original extension logic back to the caller.
    return s; // Handled specially in the engine
}

std::string TruncateRule::apply(const std::string& s, int, const std::filesystem::path&, const std::map<std::string, std::string>&) const {
    if (!enabled || s.length() <= static_cast<size_t>(max_length)) return s;
    
    std::string result = s;
    if (at_word_boundary) {
        size_t cut = max_length;
        while (cut > 0 && !std::isspace(result[cut]) && result[cut] != '_' && result[cut] != '-') {
            cut--;
        }
        if (cut > 0) result = result.substr(0, cut);
        else result = result.substr(0, max_length);
    } else {
        result = result.substr(0, max_length);
    }
    return result + ellipsis;
}

std::string SanitizeRule::apply(const std::string& s, int, const std::filesystem::path&, const std::map<std::string, std::string>&) const {
    if (!enabled || s.empty()) return s;
    
    std::string bad_chars;
    if (target_windows) bad_chars += "\\/:*?\"<>|";
    if (target_linux) bad_chars += "/";
    // Also strip non-printable
    for (int i = 0; i < 32; ++i) bad_chars += static_cast<char>(i);
    
    std::string result;
    bool last_was_rep = false;
    
    for (char c : s) {
        if (bad_chars.find(c) != std::string::npos) {
            if (collapse_replacement && last_was_rep) continue;
            result += replacement;
            last_was_rep = true;
        } else {
            result += c;
            last_was_rep = false;
        }
    }
    return result;
}

std::string HashNameRule::apply(const std::string& s, int, const std::filesystem::path& p, const std::map<std::string, std::string>&) const {
    if (!enabled) return s;
    // In a real implementation, we would hash the file content here.
    // For now, we return a pseudo-hash based on filename and size as placeholder.
    std::hash<std::string> h;
    return std::to_string(h(p.string() + std::to_string(std::filesystem::file_size(p))));
}

std::string TransliterateRule::apply(const std::string& s, int, const std::filesystem::path&, const std::map<std::string, std::string>&) const {
    if (!enabled) return s;
    // Extremely basic fallback mapping. A real implementation uses iconv or ICU.
    std::string result = s;
    // Placeholder logic
    return result;
}

std::string RegexGroupsRule::apply(const std::string& s, int, const std::filesystem::path&, const std::map<std::string, std::string>&) const {
    if (!enabled || pattern.empty() || output_template.empty()) return s;
    try {
        std::regex re(pattern);
        return std::regex_replace(s, re, output_template);
    } catch (...) {
        return s;
    }
}

//─────────────────────────── Engine Implementation ───────────────────────────

std::string BatchRenameEngine::apply_rules(
    const std::string& original_stem,
    const std::vector<std::shared_ptr<IRenameRule>>& rules,
    int index,
    const std::filesystem::path& path,
    const std::map<std::string, std::string>& metadata) 
{
    std::string current = original_stem;
    for (const auto& rule : rules) {
        if (rule->enabled && rule->rule_type() != "extension") {
            current = rule->apply(current, index, path, metadata);
        }
    }
    return current;
}

std::map<std::string, std::string> BatchRenameEngine::extract_metadata(const std::filesystem::path& file) {
    // In a full implementation, this calls exiv2 / mediainfo.
    std::map<std::string, std::string> meta;
    meta["file_size"] = std::to_string(std::filesystem::file_size(file));
    return meta;
}

std::vector<RenamePreview> BatchRenameEngine::preview(
    const std::vector<std::filesystem::path>& files,
    const std::vector<std::shared_ptr<IRenameRule>>& rules,
    bool include_extension,
    int start_index)
{
    std::vector<RenamePreview> previews;
    std::unordered_set<std::string> output_paths; // for duplicate detection
    
    // Find extension rule if any
    std::shared_ptr<ExtensionRule> ext_rule;
    for (auto& r : rules) {
        if (r->rule_type() == "extension" && r->enabled) {
            ext_rule = std::static_pointer_cast<ExtensionRule>(r);
            break;
        }
    }

    for (size_t i = 0; i < files.size(); ++i) {
        const auto& p = files[i];
        RenamePreview prev;
        prev.original_path = p;
        
        std::string stem = p.stem().string();
        std::string ext = p.extension().string();
        
        if (include_extension) stem += ext; // Operate on whole name
        
        auto meta = extract_metadata(p);
        std::string new_stem = apply_rules(stem, rules, start_index + i, p, meta);
        
        // Handle extension
        std::string new_ext = include_extension ? "" : ext;
        if (ext_rule) {
            if (ext_rule->mode == ExtensionRule::Mode::Remove) new_ext = "";
            else if (ext_rule->mode == ExtensionRule::Mode::Change) new_ext = ext_rule->new_ext;
            else if (ext_rule->mode == ExtensionRule::Mode::LowerCase) new_ext = to_lower_ascii(ext);
            else if (ext_rule->mode == ExtensionRule::Mode::UpperCase) new_ext = to_upper_ascii(ext);
            else if (ext_rule->mode == ExtensionRule::Mode::AddIfMissing && ext.empty()) new_ext = ext_rule->new_ext;
            
            if (!new_ext.empty() && new_ext[0] != '.') new_ext = "." + new_ext;
        }
        
        std::string new_name = new_stem + new_ext;
        prev.new_path = p.parent_path() / new_name;
        
        if (prev.new_path != p) {
            std::string out_str = to_lower_ascii(prev.new_path.string());
            
            // Check cross-batch duplicates
            if (output_paths.count(out_str)) {
                prev.duplicate = true;
                prev.error = "Name collision within batch";
            }
            output_paths.insert(out_str);
            
            // Check disk conflicts
            if (std::filesystem::exists(prev.new_path)) {
                prev.conflict = true;
                if (prev.error.empty()) prev.error = "File already exists on disk";
            }
        }
        
        previews.push_back(std::move(prev));
    }
    
    return previews;
}

std::vector<RenameResult> BatchRenameEngine::execute(
    const std::vector<RenamePreview>& previews,
    bool skip_conflicts,
    bool dry_run)
{
    std::vector<RenameResult> results;
    
    for (const auto& p : previews) {
        RenameResult rr;
        rr.original_path = p.original_path;
        rr.new_path = p.new_path;
        
        if (p.original_path == p.new_path) {
            rr.success = true; // No-op is success
            results.push_back(rr);
            continue;
        }
        
        if ((p.conflict || p.duplicate) && skip_conflicts) {
            rr.success = false;
            rr.error = p.error;
            results.push_back(rr);
            continue;
        }
        
        if (!dry_run) {
            try {
                std::filesystem::rename(p.original_path, p.new_path);
                rr.success = true;
            } catch (const std::exception& e) {
                rr.success = false;
                rr.error = e.what();
            }
        } else {
            rr.success = true; // Dry run pretends success
        }
        
        results.push_back(std::move(rr));
    }
    
    return results;
}

// Stubs for preset saving/loading
void BatchRenameEngine::save_preset(const RenamePreset&, const std::filesystem::path&) {}
RenamePreset BatchRenameEngine::load_preset(const std::filesystem::path&) { return {}; }
std::vector<RenamePreset> BatchRenameEngine::builtin_presets() { return {}; }

} // namespace fo::core
