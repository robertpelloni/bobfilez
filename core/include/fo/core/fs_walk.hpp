#pragma once

#include <filesystem>
#include <vector>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace fo::core {

// True for anything that aliases another location: a symlink
// or (Windows) any reparse point. std::filesystem's
// is_symlink does NOT report NTFS junctions on MSVC — they
// surface only via FILE_ATTRIBUTE_REPARSE_POINT — and a
// junction inside a scanned root otherwise lets walkers
// escape the root (observed: organize moved a file through
// an ancestor junction out of tree).
inline bool is_link_like(const std::filesystem::path& p) {
    std::error_code ec;
    if (std::filesystem::is_symlink(p, ec)) return true;
#ifdef _WIN32
    DWORD attrs = GetFileAttributesW(p.c_str());
    if (attrs != INVALID_FILE_ATTRIBUTES &&
        (attrs & FILE_ATTRIBUTE_REPARSE_POINT)) return true;
#endif
    return false;
}

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
                if (follow_dir_symlinks || !is_link_like(it->path())) {
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
