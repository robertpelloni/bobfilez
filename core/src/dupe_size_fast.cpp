#include "fo/core/interfaces.hpp"
#include "fo/core/registry.hpp"
#include <unordered_map>

namespace fo::core {

class SizeHashDuplicateFinder : public IDuplicateFinder {
public:
    std::string name() const override { return "size+fast64"; }

    std::vector<DuplicateGroup> group(const std::vector<FileInfo>& files, IHasher& hasher) override {
        std::unordered_map<std::uintmax_t, std::vector<const FileInfo*>> by_size;
        by_size.reserve(files.size());
        for (auto& f : files) {
            if (f.size == static_cast<std::uintmax_t>(-1)) continue;
            by_size[f.size].push_back(&f);
        }

        std::vector<DuplicateGroup> groups;
        for (auto& kv : by_size) {
            auto& vec = kv.second;
            if (vec.size() < 2) continue; // no dups possible
            std::unordered_map<std::string, std::vector<const FileInfo*>> by_fast;
            for (auto* fi : vec) {
                auto h = hasher.fast64(std::filesystem::path(fi->uri));
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
};

// Note: we don't register duplicate finders in a registry yet; they are used directly by name in Engine

} // namespace fo::core
