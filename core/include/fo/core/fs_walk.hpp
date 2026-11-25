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
#else
#include <unistd.h>
#include <cerrno>
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
// Rename that never replaces an existing destination.
// std::filesystem::rename replaces silently on both MSVC
// and POSIX, so every "check exists, then rename" pair has
// a stale-authorization window where a destination that
// appears in between gets clobbered. On Windows this is
// MoveFileEx without MOVEFILE_REPLACE_EXISTING (atomic
// refusal); elsewhere it falls back to link+unlink for
// regular files, then plain rename for the rest, and
// reports false with ec set when the destination exists.
inline bool rename_no_replace(const std::filesystem::path& src,
                               const std::filesystem::path& dst,
                               std::error_code& ec) {
    ec.clear();
    if (std::filesystem::exists(dst, ec)) {
        ec = std::make_error_code(std::errc::file_exists);
        return false;
    }
#ifdef _WIN32
    // No MOVEFILE_WRITE_THROUGH: it forces a per-rename disk
    // flush (~6ms/file measured on bulk organizes). NTFS
    // journals the rename metadata itself; the intent journal
    // (synchronous=FULL) carries the durability promise.
    if (MoveFileExW(src.c_str(), dst.c_str(), 0)) {
        return true;
    }
    ec = std::error_code(static_cast<int>(GetLastError()), std::system_category());
    return false;
#else
    std::error_code lec;
    if (std::filesystem::is_regular_file(src, lec) && !lec) {
        // link() refuses existing destinations atomically.
        if (::link(src.c_str(), dst.c_str()) == 0) {
            std::filesystem::remove(src, lec);
            if (lec) { ec = lec; return false; }
            return true;
        }
        ec = std::error_code(errno, std::generic_category());
        return false;
    }
    std::filesystem::rename(src, dst, ec);
    return !ec;
#endif
}

// Move a file, never replacing the destination. Rename is
// tried first; on a cross-volume failure it falls back to
// copy-to-temp-on-the-destination-volume, size verification,
// no-replace rename of the temp, then source removal — so a
// crash mid-copy leaves the source intact and only a .part
// residue (a pending journal intent reconciles the rest).
inline bool move_file_no_replace(const std::filesystem::path& src,
                                  const std::filesystem::path& dst,
                                  std::error_code& ec) {
    if (rename_no_replace(src, dst, ec)) return true;
    auto first_error = ec;
    bool cross_volume =
#ifdef _WIN32
        first_error.value() == 17; // ERROR_NOT_SAME_DEVICE
#else
        first_error == std::make_error_code(std::errc::cross_device_link);
#endif
    if (!cross_volume) { ec = first_error; return false; }

    std::error_code fec;
    auto size = std::filesystem::file_size(src, fec);
    if (fec) { ec = fec; return false; }

    auto part = dst;
    part += ".part";
    std::filesystem::remove(part, fec);
    if (!std::filesystem::copy_file(src, part, std::filesystem::copy_options::none, fec) || fec) {
        std::error_code cec;
        std::filesystem::remove(part, cec);
        ec = fec;
        return false;
    }
    auto copied = std::filesystem::file_size(part, fec);
    if (fec || copied != size) {
        std::error_code cec;
        std::filesystem::remove(part, cec);
        ec = fec ? fec : std::make_error_code(std::errc::io_error);
        return false;
    }
    if (!rename_no_replace(part, dst, ec)) {
        std::error_code cec;
        std::filesystem::remove(part, cec);
        return false;
    }
    std::filesystem::remove(src, fec);
    if (fec) { ec = fec; return false; }
    ec.clear();
    return true;
}

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
