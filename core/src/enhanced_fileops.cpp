/// @file enhanced_fileops.cpp
/// @brief Implementation of the EnhancedCopyEngine with FastCopy/TeraCopy parity.

#include "fo/core/enhanced_fileops_interface.hpp"
#include "fo/core/registry.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <system_error>
#include <uuid/uuid.h> // Assuming a UUID generator or we'll generate a simple one

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace fo::core {

// Simple UUID generator for job IDs
static std::string generate_uuid() {
    static uint64_t counter = 0;
    return "job-" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "-" + std::to_string(++counter);
}

// Check free space on destination
static bool has_free_space(const std::filesystem::path& dest_dir, uintmax_t required_bytes) {
    std::error_code ec;
    auto space_info = std::filesystem::space(dest_dir, ec);
    if (ec) return true; // Can't determine, assume okay
    return space_info.available >= required_bytes;
}

// Compute hash of file
static std::string compute_hash(const std::filesystem::path& p, const std::string& algo) {
    auto hasher = Registry<IHasher>::instance().create(algo);
    if (!hasher) return "";
    auto result = hasher->strong(p);
    return result ? *result : "";
}

//─────────────────────────── EnhancedCopyEngine ──────────────────────────────

std::string EnhancedCopyEngine::enqueue(
    const std::vector<std::filesystem::path>& sources,
    const std::filesystem::path& dest,
    const EnhancedCopyOptions& opts,
    FileOpProgressCb progress_cb,
    ErrorHandlerCb error_cb,
    FileOpResultCb result_cb)
{
    TransferJob job;
    job.id = generate_uuid();
    job.name = (opts.move_mode ? "Move " : "Copy ") + std::to_string(sources.size()) + " items to " + dest.filename().string();
    job.sources = sources;
    job.dest = dest;
    job.opts = opts;
    job.created_at = std::chrono::system_clock::now();
    job.state = TransferJob::State::Queued;

    if (opts.write_log) {
        job.log_path = (opts.log_dir.empty() ? std::filesystem::temp_directory_path() : opts.log_dir).string() + "/" + job.id + ".log";
    }

    {
        std::lock_guard<std::mutex> lock(queue_mtx_);
        jobs_.push_back(job);
    }

    // In a real application, a background thread pool would pick up jobs from the queue.
    // For this engine implementation, we provide the execution logic directly.
    // Let's spawn a thread for it right now.
    std::thread([this, job_id = job.id, error_cb, progress_cb, result_cb]() {
        TransferJob* target_job = nullptr;
        {
            std::lock_guard<std::mutex> lock(queue_mtx_);
            for (auto& j : jobs_) {
                if (j.id == job_id) {
                    target_job = &j;
                    break;
                }
            }
        }
        if (target_job) {
            run_job(*target_job, error_cb, progress_cb, result_cb);
        }
    }).detach();

    return job.id;
}

void EnhancedCopyEngine::run_job(TransferJob& job, ErrorHandlerCb error_cb,
                                 FileOpProgressCb progress_cb, FileOpResultCb result_cb)
{
    job.state = TransferJob::State::Running;
    job.started_at = std::chrono::system_clock::now();

    // 1. Scan phase (Estimation)
    job.stats.current_phase = "Scanning";
    if (progress_cb) {
        FileOpProgress p;
        p.phase = "Scanning";
        progress_cb(p);
    }

    struct Item {
        std::filesystem::path src;
        std::filesystem::path dst;
        uintmax_t size;
        bool is_dir;
    };
    std::vector<Item> items;

    uintmax_t total_size = 0;
    int total_files = 0;

    auto add_item = [&](const std::filesystem::path& s, const std::filesystem::path& d, bool is_dir, uintmax_t sz) {
        items.push_back({s, d, sz, is_dir});
        if (!is_dir) {
            total_size += sz;
            total_files++;
        }
    };

    for (const auto& src : job.sources) {
        std::error_code ec;
        if (std::filesystem::is_directory(src, ec)) {
            add_item(src, job.dest / src.filename(), true, 0);
            if (job.opts.recursive) {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(src, ec)) {
                    if (paused_all_) { while (paused_all_) std::this_thread::sleep_for(std::chrono::milliseconds(100)); }
                    if (job.state == TransferJob::State::Cancelled) break;

                    auto rel_path = std::filesystem::relative(entry.path(), src);
                    auto dst_path = job.dest / src.filename() / rel_path;
                    
                    bool is_dir = entry.is_directory(ec);
                    uintmax_t sz = is_dir ? 0 : entry.file_size(ec);
                    add_item(entry.path(), dst_path, is_dir, sz);
                }
            }
        } else if (std::filesystem::is_regular_file(src, ec)) {
            add_item(src, job.dest / src.filename(), false, std::filesystem::file_size(src, ec));
        }
    }

    job.stats.bytes_total = total_size;
    job.stats.files_total = total_files;

    if (job.state == TransferJob::State::Cancelled) return;

    if (job.opts.estimate_only) {
        job.state = TransferJob::State::Done;
        job.finished_at = std::chrono::system_clock::now();
        return;
    }

    if (job.opts.check_free_space && !has_free_space(job.dest, total_size)) {
        if (error_cb) {
            FileError err{FileError::Type::DiskFull, "", job.dest, "Insufficient free space"};
            error_cb(err);
        }
        job.state = TransferJob::State::Failed;
        return;
    }

    // 2. Transfer phase
    job.stats.current_phase = "Copying";
    auto t_start = std::chrono::steady_clock::now();

    std::ofstream log_file;
    if (job.opts.write_log) {
        log_file.open(job.log_path, std::ios::app);
        if (log_file) log_file << "Job Started: " << job.name << "\n";
    }

    for (const auto& item : items) {
        if (paused_all_ || job.state == TransferJob::State::Paused) {
            while (paused_all_ || job.state == TransferJob::State::Paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (job.state == TransferJob::State::Cancelled) break;
            }
        }
        if (job.state == TransferJob::State::Cancelled) break;

        job.stats.current_file = item.src;
        
        FileOpResult res;
        if (item.is_dir) {
            std::error_code ec;
            std::filesystem::create_directories(item.dst, ec);
            res.success = !ec;
            res.source = item.src;
            res.dest = item.dst;
            res.type = FileOpType::CreateDir;
        } else {
            res = copy_single_enhanced(item.src, item.dst, job.opts, error_cb, job.stats);
        }

        job.results.push_back(res);
        if (result_cb) result_cb(res);

        if (!item.is_dir) {
            job.stats.files_done++;
            if (!res.success) job.stats.files_failed++;
        }

        // Update stats
        auto t_now = std::chrono::steady_clock::now();
        job.stats.elapsed_sec = std::chrono::duration<double>(t_now - t_start).count();
        if (job.stats.elapsed_sec > 0) {
            job.stats.average_speed_bps = job.stats.bytes_done / job.stats.elapsed_sec;
        }

        if (progress_cb) {
            FileOpProgress p;
            p.phase = job.stats.current_phase;
            p.current_file = job.stats.current_file;
            p.bytes_done = job.stats.bytes_done;
            p.bytes_total = job.stats.bytes_total;
            p.files_done = job.stats.files_done;
            p.files_total = job.stats.files_total;
            p.speed_bytes_sec = job.stats.current_speed_bps;
            p.eta_sec = (job.stats.current_speed_bps > 0) ? (job.stats.bytes_total - job.stats.bytes_done) / job.stats.current_speed_bps : 0;
            progress_cb(p);
        }

        if (log_file) {
            log_file << (res.success ? "[OK] " : "[FAIL] ") << item.src.string() << " -> " << item.dst.string() << "\n";
            if (!res.success && !res.error.empty()) log_file << "  Error: " << res.error << "\n";
        }
    }

    job.state = (job.state == TransferJob::State::Cancelled) ? TransferJob::State::Cancelled : TransferJob::State::Done;
    job.finished_at = std::chrono::system_clock::now();
    if (log_file) log_file << "Job Finished. Status: " << static_cast<int>(job.state) << "\n";
}

FileOpResult EnhancedCopyEngine::copy_single_enhanced(
    const std::filesystem::path& src,
    const std::filesystem::path& dst,
    const EnhancedCopyOptions& opts,
    ErrorHandlerCb error_cb,
    TransferStats& stats)
{
    FileOpResult res;
    res.source = src;
    res.dest = dst;
    res.type = opts.move_mode ? FileOpType::Move : FileOpType::Copy;

    std::error_code ec;
    auto src_size = std::filesystem::file_size(src, ec);
    if (ec) {
        res.error = "Cannot read source size: " + ec.message();
        if (error_cb) error_cb(FileError{FileError::Type::ReadError, src, dst, res.error});
        return res;
    }

    // Collision handling
    std::filesystem::path target = dst;
    if (std::filesystem::exists(target, ec)) {
        if (opts.collision == CollisionPolicy::Skip) {
            res.success = true; // Technically skipped
            return res;
        } else if (opts.collision == CollisionPolicy::OverwriteOlder) {
            auto src_time = std::filesystem::last_write_time(src, ec);
            auto dst_time = std::filesystem::last_write_time(target, ec);
            if (src_time <= dst_time) {
                res.success = true;
                return res;
            }
        } else if (opts.collision == CollisionPolicy::RenameAuto) {
            int counter = 2;
            auto stem = target.stem().string();
            auto ext = target.extension().string();
            auto dir = target.parent_path();
            while (std::filesystem::exists(target, ec)) {
                target = dir / (stem + " (" + std::to_string(counter++) + ")" + ext);
            }
            res.dest = target;
        } else if (opts.collision == CollisionPolicy::Ask && error_cb) {
            FileError err{FileError::Type::FileExists, src, target, "File already exists"};
            auto action = error_cb(err);
            if (action == FileErrorAction::Skip) return res;
            if (action == FileErrorAction::Abort) { res.error = "Aborted"; return res; }
            // If Overwrite, proceed.
        }
    }

    auto t0 = std::chrono::steady_clock::now();

    // Fast paths
    if (opts.create_hardlinks) {
        std::filesystem::create_hard_link(src, target, ec);
        if (!ec) { res.success = true; return res; }
    } else if (opts.create_symlinks) {
        std::filesystem::create_symlink(src, target, ec);
        if (!ec) { res.success = true; return res; }
    }

    if (opts.move_mode && opts.smart_mode) {
        std::filesystem::rename(src, target, ec);
        if (!ec) {
            stats.bytes_done += src_size;
            res.success = true;
            res.bytes_transferred = src_size;
            return res;
        }
        // Fallback to copy+delete if cross-device
    }

    // Buffered copy loop
    std::ifstream is;
    std::ofstream os;

#ifdef _WIN32
    // Optional: Use Win32 CreateFile with FILE_FLAG_NO_BUFFERING for FastCopy parity.
    // For portability in this codebase, we use standard streams with custom buffers.
#endif

    is.open(src, std::ios::binary);
    if (!is) {
        res.error = "Cannot open source";
        if (error_cb) error_cb(FileError{FileError::Type::ReadError, src, target, res.error});
        return res;
    }

    os.open(target, std::ios::binary | std::ios::trunc);
    if (!os) {
        res.error = "Cannot open destination";
        if (error_cb) error_cb(FileError{FileError::Type::WriteError, src, target, res.error});
        return res;
    }

    size_t buf_size = opts.read_buffer_size > 0 ? opts.read_buffer_size : (4 * 1024 * 1024);
    std::vector<char> buffer(buf_size);

    int64_t copied = 0;
    auto t_last_speed = t0;
    int64_t bytes_since_last_speed = 0;

    while (is && os) {
        apply_throttle(buf_size);

        is.read(buffer.data(), buffer.size());
        std::streamsize bytes_read = is.gcount();
        if (bytes_read > 0) {
            os.write(buffer.data(), bytes_read);
            if (!os) {
                res.error = "Write failed";
                break;
            }
            copied += bytes_read;
            stats.bytes_done += bytes_read;
            bytes_since_last_speed += bytes_read;

            auto t_now = std::chrono::steady_clock::now();
            auto dt = std::chrono::duration<double>(t_now - t_last_speed).count();
            if (dt > 0.5) { // Update speed every 500ms
                stats.current_speed_bps = bytes_since_last_speed / dt;
                if (stats.current_speed_bps > stats.peak_speed_bps) {
                    stats.peak_speed_bps = stats.current_speed_bps;
                }
                // Ring buffer for graph
                stats.speed_history.push_back(stats.current_speed_bps);
                if (stats.speed_history.size() > 60) stats.speed_history.erase(stats.speed_history.begin());

                bytes_since_last_speed = 0;
                t_last_speed = t_now;
            }
        }
    }

    is.close();
    os.close();

    res.success = (copied == src_size);
    res.bytes_transferred = copied;

    if (res.success && opts.verify_checksums) {
        stats.current_phase = "Verifying";
        res.checksum_before = compute_hash(src, opts.verify_hash);
        res.checksum_after = compute_hash(target, opts.verify_hash);
        res.checksum_match = (res.checksum_before == res.checksum_after);
        if (!res.checksum_match) {
            res.success = false;
            res.error = "Checksum mismatch";
            if (error_cb) error_cb(FileError{FileError::Type::ChecksumMismatch, src, target, res.error});
        }
        stats.current_phase = "Copying";
    }

    if (res.success && opts.preserve_timestamps) {
        std::filesystem::last_write_time(target, std::filesystem::last_write_time(src, ec), ec);
    }

    if (res.success && opts.move_mode) {
        std::filesystem::remove(src, ec);
    }

    auto t1 = std::chrono::steady_clock::now();
    res.duration_sec = std::chrono::duration<double>(t1 - t0).count();

    return res;
}

void EnhancedCopyEngine::apply_throttle(size_t bytes_transferred) {
    int64_t limit = throttle_bps_.load();
    if (limit <= 0) return;

    // Simple throttle: calculate expected time for this chunk, sleep if we're too fast
    double expected_sec = static_cast<double>(bytes_transferred) / limit;
    std::this_thread::sleep_for(std::chrono::duration<double>(expected_sec));
}

void EnhancedCopyEngine::pause(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(queue_mtx_);
    for (auto& j : jobs_) {
        if (j.id == job_id && j.state == TransferJob::State::Running) {
            j.state = TransferJob::State::Paused;
        }
    }
}

void EnhancedCopyEngine::resume(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(queue_mtx_);
    for (auto& j : jobs_) {
        if (j.id == job_id && j.state == TransferJob::State::Paused) {
            j.state = TransferJob::State::Running;
        }
    }
}

void EnhancedCopyEngine::cancel(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(queue_mtx_);
    for (auto& j : jobs_) {
        if (j.id == job_id) {
            j.state = TransferJob::State::Cancelled;
        }
    }
}

void EnhancedCopyEngine::cancel_all() {
    std::lock_guard<std::mutex> lock(queue_mtx_);
    for (auto& j : jobs_) {
        j.state = TransferJob::State::Cancelled;
    }
}

const TransferJob* EnhancedCopyEngine::get_job(const std::string& job_id) const {
    std::lock_guard<std::mutex> lock(queue_mtx_);
    for (const auto& j : jobs_) {
        if (j.id == job_id) return &j;
    }
    return nullptr;
}

std::vector<const TransferJob*> EnhancedCopyEngine::all_jobs() const {
    std::lock_guard<std::mutex> lock(queue_mtx_);
    std::vector<const TransferJob*> res;
    for (const auto& j : jobs_) res.push_back(&j);
    return res;
}

void EnhancedCopyEngine::move_job_up(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(queue_mtx_);
    for (size_t i = 1; i < jobs_.size(); ++i) {
        if (jobs_[i].id == job_id) {
            std::swap(jobs_[i], jobs_[i-1]);
            break;
        }
    }
}

void EnhancedCopyEngine::move_job_down(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(queue_mtx_);
    for (size_t i = 0; i + 1 < jobs_.size(); ++i) {
        if (jobs_[i].id == job_id) {
            std::swap(jobs_[i], jobs_[i+1]);
            break;
        }
    }
}

void EnhancedCopyEngine::clear_finished() {
    std::lock_guard<std::mutex> lock(queue_mtx_);
    jobs_.erase(std::remove_if(jobs_.begin(), jobs_.end(), [](const TransferJob& j) {
        return j.state == TransferJob::State::Done || j.state == TransferJob::State::Failed || j.state == TransferJob::State::Cancelled;
    }), jobs_.end());
}

void EnhancedCopyEngine::set_throttle(int64_t bytes_per_sec) {
    throttle_bps_.store(bytes_per_sec);
}

int64_t EnhancedCopyEngine::estimate(const std::vector<std::filesystem::path>& sources, const EnhancedCopyOptions& opts) {
    int64_t total = 0;
    for (const auto& src : sources) {
        std::error_code ec;
        if (std::filesystem::is_directory(src, ec)) {
            if (opts.recursive) {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(src, ec)) {
                    if (entry.is_regular_file(ec)) total += entry.file_size(ec);
                }
            }
        } else if (std::filesystem::is_regular_file(src, ec)) {
            total += std::filesystem::file_size(src, ec);
        }
    }
    return total;
}

// Stubs for save/load queue
void EnhancedCopyEngine::save_queue(const std::filesystem::path&) const {}
void EnhancedCopyEngine::load_queue(const std::filesystem::path&) {}
void EnhancedCopyEngine::write_job_log(const TransferJob&) {}

} // namespace fo::core
