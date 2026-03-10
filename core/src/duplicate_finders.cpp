#include "fo/core/duplicate_finders.hpp"
#include <map>
#include <fstream>

namespace fo::core {

std::vector<DuplicateGroup> SizeHashDuplicateFinder::group(const std::vector<FileInfo>& files, IHasher& hasher) {
    std::map<std::pair<std::uintmax_t, std::string>, std::vector<FileInfo>> groups;

    for (const auto& file : files) {
        std::string fast_hash = hasher.fast64(std::filesystem::path(file.uri));
        groups[{file.size, fast_hash}].push_back(file);
    }

    std::vector<DuplicateGroup> result;
    for (auto const& [key, val] : groups) {
        if (val.size() > 1) {
            result.push_back({key.first, key.second, val});
        }
    }

    return result;
}

static bool compare_files(const std::filesystem::path& p1, const std::filesystem::path& p2) {
    std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

    if (f1.fail() || f2.fail()) {
        return false; // File could not be opened
    }

    if (f1.tellg() != f2.tellg()) {
        return false; // File sizes differ
    }

    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);

    std::istreambuf_iterator<char> begin1(f1);
    std::istreambuf_iterator<char> begin2(f2);

    std::istreambuf_iterator<char> end;

    return std::equal(begin1, end, begin2, end);
}

std::vector<DuplicateGroup> SizeHashByteDuplicateFinder::group(const std::vector<FileInfo>& files, IHasher& hasher) {
    auto initial_groups = SizeHashDuplicateFinder().group(files, hasher);
    std::vector<DuplicateGroup> result;

    for (auto& group : initial_groups) {
        if (group.files.size() <= 1) continue;

        std::vector<FileInfo> verified_files;
        verified_files.push_back(group.files[0]);

        for (size_t i = 1; i < group.files.size(); ++i) {
            if (compare_files(std::filesystem::path(group.files[0].uri), std::filesystem::path(group.files[i].uri))) {
                verified_files.push_back(group.files[i]);
            }
        }

        if (verified_files.size() > 1) {
            result.push_back({group.size, group.fast64, verified_files});
        }
    }

    return result;
}

}
