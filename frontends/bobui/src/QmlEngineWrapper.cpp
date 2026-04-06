#include "QmlEngineWrapper.hpp"

#include <fo/core/engine.hpp>
#include <fo/core/interfaces.hpp>
#include <fo/core/registry.hpp>

#include <QMetaObject>
#include <QVariantMap>
#include <QStringList>

#include <algorithm>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

QString format_megabytes(std::uintmax_t bytes)
{
    const double megabytes = static_cast<double>(bytes) / (1024.0 * 1024.0);
    return QString::number(megabytes, 'f', 2) + QStringLiteral(" MB");
}

QVariantList build_scan_results(const std::vector<fo::core::FileInfo> &files)
{
    QVariantList results;
    const int limit = std::min<int>(static_cast<int>(files.size()), 100);

    for (int index = 0; index < limit; ++index) {
        QVariantMap row;
        row[QStringLiteral("path")] = QString::fromStdString(files[index].uri);
        row[QStringLiteral("size")] = QString::number(files[index].size);
        results.append(row);
    }

    return results;
}

QVariantList build_duplicate_results(const std::vector<fo::core::DuplicateGroup> &groups)
{
    QVariantList results;
    const int limit = std::min<int>(static_cast<int>(groups.size()), 100);

    for (int index = 0; index < limit; ++index) {
        const auto &group = groups[index];
        QVariantMap row;
        QStringList file_list;
        for (const auto &file : group.files) {
            file_list << QString::fromStdString(file.uri);
        }

        row[QStringLiteral("hash")] = QString::fromStdString(group.fast64);
        row[QStringLiteral("size")] = QString::number(group.size);
        row[QStringLiteral("files")] = file_list.join(QStringLiteral(" | "));
        results.append(row);
    }

    return results;
}

QString build_stats_report(const std::string &directory_path, const std::vector<fo::core::FileInfo> &files)
{
    std::uintmax_t total_size = 0;
    int directory_count = 0;
    int file_count = 0;
    std::map<std::string, int> extension_counts;

    for (const auto &file : files) {
        if (file.is_dir) {
            ++directory_count;
            continue;
        }

        ++file_count;
        total_size += file.size;

        const auto dot = file.uri.find_last_of('.');
        const auto slash = file.uri.find_last_of("/\\");
        std::string extension = (dot != std::string::npos && (slash == std::string::npos || dot > slash))
            ? file.uri.substr(dot)
            : "(none)";
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        ++extension_counts[extension];
    }

    std::vector<std::pair<std::string, int>> sorted_extensions(extension_counts.begin(), extension_counts.end());
    std::sort(sorted_extensions.begin(), sorted_extensions.end(), [](const auto &lhs, const auto &rhs) {
        return lhs.second > rhs.second;
    });

    QString report;
    report += QStringLiteral("Statistics for: ") + QString::fromStdString(directory_path) + QStringLiteral("\n\n");
    report += QStringLiteral("Total files: ") + QString::number(file_count) + QStringLiteral("\n");
    report += QStringLiteral("Total directories: ") + QString::number(directory_count) + QStringLiteral("\n");
    report += QStringLiteral("Total size: ") + format_megabytes(total_size) + QStringLiteral("\n\n");
    report += QStringLiteral("Top extensions:\n");

    int shown = 0;
    for (const auto &[extension, count] : sorted_extensions) {
        if (shown++ >= 10) {
            break;
        }
        report += QStringLiteral("  ") + QString::fromStdString(extension) + QStringLiteral(": ") + QString::number(count) + QStringLiteral(" files\n");
    }

    return report;
}

QString build_hash_report(const std::string &file_path)
{
    auto fast64_hasher = fo::core::Registry<fo::core::IHasher>::instance().create("fast64");
    if (!fast64_hasher) {
        throw std::runtime_error("Hasher 'fast64' not found.");
    }

    auto sha256_hasher = fo::core::Registry<fo::core::IHasher>::instance().create("sha256");
    const auto path = std::filesystem::u8path(file_path);

    QString report;
    report += QStringLiteral("File: ") + QString::fromStdString(file_path) + QStringLiteral("\n\n");
    report += QStringLiteral("Fast64: ") + QString::fromStdString(fast64_hasher->fast64(path)) + QStringLiteral("\n");

    if (sha256_hasher) {
        report += QStringLiteral("SHA-256: ") + QString::fromStdString(sha256_hasher->fast64(path)) + QStringLiteral("\n");
    }

    return report;
}

QString format_taken_timestamp(const fo::core::ImageMetadata &metadata)
{
    if (!metadata.date.has_taken) {
        return QStringLiteral("n/a");
    }

    const auto time = std::chrono::system_clock::to_time_t(metadata.date.taken);
    std::tm tm_buffer;
#ifdef _WIN32
    localtime_s(&tm_buffer, &time);
#else
    localtime_r(&time, &tm_buffer);
#endif
    std::ostringstream timestamp;
    timestamp << std::put_time(&tm_buffer, "%Y-%m-%d %H:%M:%S");
    return QString::fromStdString(timestamp.str());
}

QString build_metadata_report(const std::string &directory_path, const std::vector<fo::core::FileInfo> &files, int &record_count)
{
    auto provider = fo::core::Registry<fo::core::IMetadataProvider>::instance().create("tinyexif");
    if (!provider) {
        throw std::runtime_error("Metadata provider 'tinyexif' not found.");
    }

    QString report;
    report += QStringLiteral("Metadata for: ") + QString::fromStdString(directory_path) + QStringLiteral("\n\n");

    record_count = 0;
    for (const auto &file : files) {
        if (file.is_dir) {
            continue;
        }

        fo::core::ImageMetadata metadata;
        if (!provider->read(std::filesystem::path(file.uri), metadata)) {
            continue;
        }

        ++record_count;
        report += QString::fromStdString(file.uri) + QStringLiteral("\n");
        report += QStringLiteral("  Taken: ") + format_taken_timestamp(metadata) + QStringLiteral("\n");
        if (metadata.has_gps) {
            report += QStringLiteral("  GPS: ") + QString::number(metadata.gps_lat, 'f', 6) + QStringLiteral(", ") + QString::number(metadata.gps_lon, 'f', 6) + QStringLiteral("\n");
        } else {
            report += QStringLiteral("  GPS: n/a\n");
        }
        report += QStringLiteral("\n");

        if (record_count >= 20) {
            break;
        }
    }

    if (record_count == 0) {
        report += QStringLiteral("No readable metadata records were found.\n");
    }

    return report;
}

} // namespace

QmlEngineWrapper::QmlEngineWrapper(QObject *parent) : QObject(parent) {}

void QmlEngineWrapper::runScan(const QString &directoryPath)
{
    const std::string path = directoryPath.toStdString();
    std::thread([this, path]() {
        try {
            auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
            if (!scanner) {
                throw std::runtime_error("Scanner 'std' not found.");
            }

            const std::vector<std::filesystem::path> roots = { std::filesystem::u8path(path) };
            const std::vector<std::string> exts;
            auto files = scanner->scan(roots, exts, false);

            std::uintmax_t total_size = 0;
            for (const auto &file : files) {
                total_size += file.size;
            }

            const QString stats = QStringLiteral("Found %1 items (%2)").arg(files.size()).arg(format_megabytes(total_size));
            const QVariantList results = build_scan_results(files);

            QMetaObject::invokeMethod(this, [this, results, stats]() {
                emit scanFinished(results, stats);
            }, Qt::QueuedConnection);
        } catch (const std::exception &error) {
            const QString message = QString::fromStdString(error.what());
            QMetaObject::invokeMethod(this, [this, message]() {
                emit errorOccurred(QStringLiteral("scan"), message);
            }, Qt::QueuedConnection);
        }
    }).detach();
}

void QmlEngineWrapper::runDuplicates(const QString &directoryPath)
{
    const std::string path = directoryPath.toStdString();
    std::thread([this, path]() {
        try {
            fo::core::EngineConfig config;
            config.db_path = ":memory:";
            config.scanner = "std";
            config.hasher = "fast64";
            fo::core::Engine engine(config);

            const std::vector<std::filesystem::path> roots = { std::filesystem::u8path(path) };
            const std::vector<std::string> exts;
            auto files = engine.scan(roots, exts, false, false);
            auto groups = engine.find_duplicates(files);

            size_t duplicate_files = 0;
            for (const auto &group : groups) {
                duplicate_files += group.files.size();
            }

            const QString stats = QStringLiteral("Found %1 groups (%2 files)").arg(groups.size()).arg(duplicate_files);
            const QVariantList results = build_duplicate_results(groups);

            QMetaObject::invokeMethod(this, [this, results, stats]() {
                emit duplicatesFinished(results, stats);
            }, Qt::QueuedConnection);
        } catch (const std::exception &error) {
            const QString message = QString::fromStdString(error.what());
            QMetaObject::invokeMethod(this, [this, message]() {
                emit errorOccurred(QStringLiteral("duplicates"), message);
            }, Qt::QueuedConnection);
        }
    }).detach();
}

void QmlEngineWrapper::runStats(const QString &directoryPath)
{
    const std::string path = directoryPath.toStdString();
    std::thread([this, path]() {
        try {
            auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
            if (!scanner) {
                throw std::runtime_error("Scanner 'std' not found.");
            }

            const std::vector<std::filesystem::path> roots = { std::filesystem::u8path(path) };
            const std::vector<std::string> exts;
            const auto files = scanner->scan(roots, exts, false);

            int file_count = 0;
            for (const auto &file : files) {
                if (!file.is_dir) {
                    ++file_count;
                }
            }

            const QString stats = QStringLiteral("Computed statistics for %1 files").arg(file_count);
            const QString report = build_stats_report(path, files);

            QMetaObject::invokeMethod(this, [this, report, stats]() {
                emit statsFinished(report, stats);
            }, Qt::QueuedConnection);
        } catch (const std::exception &error) {
            const QString message = QString::fromStdString(error.what());
            QMetaObject::invokeMethod(this, [this, message]() {
                emit errorOccurred(QStringLiteral("stats"), message);
            }, Qt::QueuedConnection);
        }
    }).detach();
}

void QmlEngineWrapper::runHash(const QString &filePath)
{
    const std::string path = filePath.toStdString();
    std::thread([this, path]() {
        try {
            const QString report = build_hash_report(path);
            const QString stats = QStringLiteral("Hash complete.");

            QMetaObject::invokeMethod(this, [this, report, stats]() {
                emit hashFinished(report, stats);
            }, Qt::QueuedConnection);
        } catch (const std::exception &error) {
            const QString message = QString::fromStdString(error.what());
            QMetaObject::invokeMethod(this, [this, message]() {
                emit errorOccurred(QStringLiteral("hash"), message);
            }, Qt::QueuedConnection);
        }
    }).detach();
}

void QmlEngineWrapper::runMetadata(const QString &directoryPath)
{
    const std::string path = directoryPath.toStdString();
    std::thread([this, path]() {
        try {
            auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
            if (!scanner) {
                throw std::runtime_error("Scanner 'std' not found.");
            }

            const std::vector<std::filesystem::path> roots = { std::filesystem::u8path(path) };
            const std::vector<std::string> exts;
            const auto files = scanner->scan(roots, exts, false);

            int record_count = 0;
            const QString report = build_metadata_report(path, files, record_count);
            const QString stats = QStringLiteral("Loaded %1 metadata records.").arg(record_count);

            QMetaObject::invokeMethod(this, [this, report, stats]() {
                emit metadataFinished(report, stats);
            }, Qt::QueuedConnection);
        } catch (const std::exception &error) {
            const QString message = QString::fromStdString(error.what());
            QMetaObject::invokeMethod(this, [this, message]() {
                emit errorOccurred(QStringLiteral("metadata"), message);
            }, Qt::QueuedConnection);
        }
    }).detach();
}
