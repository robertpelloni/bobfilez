#include "fo/core/engine.hpp"
#include "fo/core/ads_cache.hpp"
#include <unordered_map>
#include <algorithm>
#include <iostream>

namespace fo::core {

std::vector<FileInfo> Engine::scan(const std::vector<std::filesystem::path>& roots,
                                   const std::vector<std::string>& include_exts,
                                   bool follow_symlinks,
                                   bool prune) {
    if (!scanner_) throw std::runtime_error("scanner not found: " + cfg_.scanner);
    
    int64_t session_id = session_repo_.start_session();
    std::vector<FileInfo> files;
    
    try {
        files = scanner_->scan(roots, include_exts, follow_symlinks);
        
        // Filter ignored files
        auto ignore_rules = ignore_repo_.get_all();
        if (!ignore_rules.empty()) {
            std::erase_if(files, [&](const FileInfo& f) {
                return ignore_repo_.is_ignored(f.uri);
            });
        }

        db_manager_.execute("BEGIN TRANSACTION;");

        // 1. Identify existing vs new files
        std::vector<FileInfo*> new_files;
        std::vector<int64_t> present_ids;
        present_ids.reserve(files.size());

        for (auto& f : files) {
            auto existing = file_repo_.get_by_path(f.uri);
            if (existing) {
                f.id = existing->id;
                present_ids.push_back(f.id);
                // Update metadata if changed
                file_repo_.upsert(f); 
            } else {
                new_files.push_back(&f);
            }
        }

        // 2. Detect moves (if we have new files)
        if (!new_files.empty()) {
            auto missing_candidates = file_repo_.get_missing_files(roots, present_ids);
            
            // Optimization: build a map of missing files by size
            std::unordered_map<std::uintmax_t, std::vector<FileInfo>> missing_by_size;
            for (const auto& m : missing_candidates) {
                missing_by_size[m.size].push_back(m);
            }

            for (auto* new_f : new_files) {
                auto it = missing_by_size.find(new_f->size);
                bool found_move = false;
                
                if (it != missing_by_size.end()) {
                    auto& candidates = it->second;
                    // Find match by mtime
                    auto match_it = std::find_if(candidates.begin(), candidates.end(), [&](const FileInfo& c) {
                        return c.mtime == new_f->mtime;
                    });

                    if (match_it != candidates.end()) {
                        // Found a move!
                        file_repo_.update_path(match_it->id, new_f->uri);
                        new_f->id = match_it->id;
                        present_ids.push_back(new_f->id);
                        
                        // Update metadata just in case
                        file_repo_.upsert(*new_f);

                        // Remove from candidates
                        candidates.erase(match_it);
                        if (candidates.empty()) missing_by_size.erase(it);
                        found_move = true;
                    }
                }
                
                if (!found_move) {
                    // Truly new
                    file_repo_.upsert(*new_f);
                    present_ids.push_back(new_f->id);
                }
            }
        }

        // 3. Prune if requested
        if (prune) {
            file_repo_.prune_missing(present_ids, roots);
        }

        db_manager_.execute("COMMIT;");
        
        session_repo_.end_session(session_id, "completed", static_cast<int>(files.size()));
    } catch (...) {
        try { db_manager_.execute("ROLLBACK;"); } catch(...) {}
        session_repo_.end_session(session_id, "failed", 0);
        throw;
    }
    
    return files;
}

std::vector<DuplicateGroup> Engine::find_duplicates(const std::vector<FileInfo>& files) {
    if (!hasher_) throw std::runtime_error("hasher not found: " + cfg_.hasher);
    // use size+fast64 strategy for now
    SizeHashDuplicateFinder local(cfg_.use_ads_cache);
    auto groups = local.group(files, *hasher_);

    // Persist duplicates
    db_manager_.execute("BEGIN TRANSACTION;");
    try {
        duplicate_repo_.clear_all();
        for (auto& g : groups) {
            if (g.files.empty()) continue;
            // Use first file as primary for now
            int64_t primary_id = g.files[0].id;
            // Ensure primary_id is valid (it should be if scan ran)
            if (primary_id == 0) continue; 

            int64_t gid = duplicate_repo_.create_group(primary_id);
            for (auto& f : g.files) {
                if (f.id != 0) {
                    duplicate_repo_.add_member(gid, f.id);
                }
            }
        }
        db_manager_.execute("COMMIT;");
    } catch (...) {
        db_manager_.execute("ROLLBACK;");
        throw;
    }

    return groups;
}

std::vector<DuplicateGroup> Engine::SizeHashDuplicateFinder::group(const std::vector<FileInfo>& files, IHasher& hasher) {
    // Forward to the implementation in dupe_size_fast.cpp without exposing it
    // Duplicate minimal logic here to avoid cross-odr issues
    std::unordered_map<std::uintmax_t, std::vector<const FileInfo*>> by_size;
    by_size.reserve(files.size());
    for (auto& f : files) {
        if (f.size == static_cast<std::uintmax_t>(-1)) continue;
        by_size[f.size].push_back(&f);
    }

    std::vector<DuplicateGroup> groups;
    for (auto& kv : by_size) {
        auto& vec = kv.second;
        if (vec.size() < 2) continue;
        std::unordered_map<std::string, std::vector<const FileInfo*>> by_fast;
        for (auto* fi : vec) {
            std::string h;

            // Try ADS cache first if enabled
            if (use_ads_) {
                auto cached = ADSCache::get_hash(fi->uri, "fast64");
                if (cached) {
                    h = *cached;
                } else {
                    h = hasher.fast64(fi->uri);
                    ADSCache::set_hash(fi->uri, "fast64", h);
                }
            } else {
                h = hasher.fast64(fi->uri);
            }

            by_fast[h].push_back(fi);
        }
        for (auto& hv : by_fast) {
            if (hv.second.size() < 2) continue;
            DuplicateGroup g;
            g.size = kv.first;
            g.fast64 = hv.first;
            for (auto* fi : hv.second) g.files.push_back(*fi);
            groups.push_back(std::move(g));
        }
    }
    return groups;
}

} // namespace fo::core
