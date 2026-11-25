#pragma once

#include <filesystem>
#include <vector>

namespace fo::core {

// Collect every regular file under `root` with a manual stack
// DFS (per-level directory_iterator, error codes only).
//
// This exists because the standard recursive directory
// iterator proved unsafe on this platform/toolchain across
// five separate walkers: it crashed with an access violation
// (std scanner), threw out of whole commands (linter), or
// silently truncated results (pruner) whenever a subtree
// crossed MAX_PATH or vanished mid-walk. Per-level iteration
// fails cleanly per subtree: unenterable directories are
// skipped, siblings are still visited. Collect-first also
// makes it safe for callers that mutate the tree (rename):
// they process a snapshot, never a live iteration.
//
// Directory symlinks are not followed unless requested.
// Unreadable/vanished entries are skipped, never thrown on.
inline std::vector<std::filesystem::path> collect_regular_files(
    const std::filesystem::path& root, bool follow_dir_symlinks = false) {
    std::vector<std::filesystem::path> out;
    std::error_code ec;
    if (std::filesystem::is_regular_file(root, ec)) {
        out.push_back(root);
        return out;
    }
    std::vector<std::filesystem::path> stack{root};
    while (!stack.empty()) {
        auto cur = stack.back();
        stack.pop_back();
        std::error_code dec;
        for (std::filesystem::directory_iterator it(cur, dec), end;
             it != end; it.increment(dec)) {
            if (dec) { dec.clear(); break; }
            std::error_code fec;
            if (it->is_directory(fec) && !fec) {
                if (follow_dir_symlinks || !it->is_symlink(fec)) {
                    stack.push_back(it->path());
                }
                continue;
            }
            if (it->is_regular_file(fec) && !fec) out.push_back(it->path());
        }
    }
    return out;
}

} // namespace fo::core
