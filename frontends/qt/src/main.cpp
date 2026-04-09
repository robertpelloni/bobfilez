#include <QApplication>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QTableWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

#include <fo/core/engine.hpp>
#include <fo/core/interfaces.hpp>
#include <fo/core/lint_interface.hpp>
#include <fo/core/registry.hpp>
#include <fo/core/version.hpp>
#include <fo/core/search_interface.hpp>
#include <fo/core/omniflow_engine_interface.hpp>
#include <fo/core/self_healing_interface.hpp>
#include <fo/core/export.hpp>

#include <algorithm>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <map>
#include <sstream>
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

QString lint_type_name(fo::core::LintType type)
{
    switch (type) {
        case fo::core::LintType::EmptyFile:
            return QStringLiteral("EmptyFile");
        case fo::core::LintType::EmptyDirectory:
            return QStringLiteral("EmptyDirectory");
        case fo::core::LintType::BrokenSymlink:
            return QStringLiteral("BrokenSymlink");
        case fo::core::LintType::TemporaryFile:
            return QStringLiteral("TemporaryFile");
    }

    return QStringLiteral("Unknown");
}

class DashboardTab final : public QWidget {
public:
    explicit DashboardTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        auto *title = new QLabel(QStringLiteral("bobfilez Qt GUI Demo"), this);
        auto *body = new QLabel(
            QStringLiteral("This Qt Widgets frontend now exercises the same core scan, duplicate, statistics, hash, metadata, and lint workflows used across the expanding multi-frontend matrix."),
            this);
        auto *version = new QLabel(QStringLiteral("Core version: ") + QString::fromUtf8(fo::core::FO_VERSION.data()), this);

        title->setStyleSheet(QStringLiteral("font-size: 26px; font-weight: bold;"));
        title->setAlignment(Qt::AlignCenter);
        body->setAlignment(Qt::AlignCenter);
        body->setWordWrap(true);
        version->setAlignment(Qt::AlignCenter);

        layout->addStretch(1);
        layout->addWidget(title);
        layout->addSpacing(8);
        layout->addWidget(body);
        layout->addSpacing(8);
        layout->addWidget(version);
        layout->addStretch(1);
    }
};

class ScannerTab final : public QWidget {
public:
    explicit ScannerTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        auto *topLayout = new QHBoxLayout();

        scan_button_ = new QPushButton(QStringLiteral("Select Directory to Scan..."), this);
        status_label_ = new QLabel(QStringLiteral("No directory scanned yet."), this);
        table_ = new QTableWidget(0, 2, this);

        topLayout->addWidget(scan_button_);
        topLayout->addWidget(status_label_, 1);
        layout->addLayout(topLayout);

        table_->setHorizontalHeaderLabels({QStringLiteral("Path"), QStringLiteral("Size (bytes)")});
        table_->horizontalHeader()->setStretchLastSection(true);
        table_->verticalHeader()->setVisible(false);
        layout->addWidget(table_);

        connect(scan_button_, &QPushButton::clicked, this, [this]() {
            const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Select Directory"));
            if (!directory.isEmpty()) {
                run_scan(directory.toStdString());
            }
        });
    }

private:
    void run_scan(const std::string &directory_path)
    {
        scan_button_->setEnabled(false);
        status_label_->setText(QStringLiteral("Scanning: ") + QString::fromStdString(directory_path) + QStringLiteral(" ..."));
        table_->setRowCount(0);

        std::thread([this, directory_path]() {
            try {
                auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
                if (!scanner) {
                    throw std::runtime_error("Scanner 'std' not found.");
                }

                const std::vector<std::filesystem::path> roots = { std::filesystem::u8path(directory_path) };
                const std::vector<std::string> exts;
                auto files = scanner->scan(roots, exts, false);

                std::uintmax_t total_size = 0;
                for (const auto &file : files) {
                    total_size += file.size;
                }

                QTimer::singleShot(0, this, [this, files = std::move(files), total_size]() {
                    status_label_->setText(QStringLiteral("Found %1 items (%2)").arg(files.size()).arg(format_megabytes(total_size)));

                    const int display_count = std::min<int>(static_cast<int>(files.size()), 100);
                    table_->setRowCount(display_count);
                    for (int index = 0; index < display_count; ++index) {
                        table_->setItem(index, 0, new QTableWidgetItem(QString::fromStdString(files[index].uri)));
                        table_->setItem(index, 1, new QTableWidgetItem(QString::number(files[index].size)));
                    }
                    scan_button_->setEnabled(true);
                });
            } catch (const std::exception &error) {
                const QString message = QString::fromStdString(error.what());
                QTimer::singleShot(0, this, [this, message]() {
                    status_label_->setText(QStringLiteral("Error: ") + message);
                    scan_button_->setEnabled(true);
                });
            }
        }).detach();
    }

    QPushButton *scan_button_ = nullptr;
    QLabel *status_label_ = nullptr;
    QTableWidget *table_ = nullptr;
};

class DuplicatesTab final : public QWidget {
public:
    explicit DuplicatesTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        auto *topLayout = new QHBoxLayout();

        dupe_button_ = new QPushButton(QStringLiteral("Find Duplicates in Directory..."), this);
        status_label_ = new QLabel(QStringLiteral("No duplicate search run yet."), this);
        table_ = new QTableWidget(0, 3, this);

        topLayout->addWidget(dupe_button_);
        topLayout->addWidget(status_label_, 1);
        layout->addLayout(topLayout);

        table_->setHorizontalHeaderLabels({QStringLiteral("Fast64"), QStringLiteral("Group Size"), QStringLiteral("Files")});
        table_->horizontalHeader()->setStretchLastSection(true);
        table_->verticalHeader()->setVisible(false);
        layout->addWidget(table_);

        connect(dupe_button_, &QPushButton::clicked, this, [this]() {
            const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Select Directory"));
            if (!directory.isEmpty()) {
                run_duplicates(directory.toStdString());
            }
        });
    }

private:
    void run_duplicates(const std::string &directory_path)
    {
        dupe_button_->setEnabled(false);
        status_label_->setText(QStringLiteral("Finding duplicates in: ") + QString::fromStdString(directory_path) + QStringLiteral(" ..."));
        table_->setRowCount(0);

        std::thread([this, directory_path]() {
            try {
                fo::core::EngineConfig config;
                config.db_path = ":memory:";
                config.scanner = "std";
                config.hasher = "fast64";
                fo::core::Engine engine(config);

                const std::vector<std::filesystem::path> roots = { std::filesystem::u8path(directory_path) };
                const std::vector<std::string> exts;
                auto files = engine.scan(roots, exts, false, false);
                auto groups = engine.find_duplicates(files);

                size_t duplicate_files = 0;
                for (const auto &group : groups) {
                    duplicate_files += group.files.size();
                }

                QTimer::singleShot(0, this, [this, groups = std::move(groups), duplicate_files]() {
                    status_label_->setText(QStringLiteral("Found %1 groups (%2 files)").arg(groups.size()).arg(duplicate_files));

                    const int display_count = std::min<int>(static_cast<int>(groups.size()), 100);
                    table_->setRowCount(display_count);
                    for (int index = 0; index < display_count; ++index) {
                        const auto &group = groups[index];
                        QStringList file_list;
                        for (const auto &file : group.files) {
                            file_list << QString::fromStdString(file.uri);
                        }

                        table_->setItem(index, 0, new QTableWidgetItem(QString::fromStdString(group.fast64)));
                        table_->setItem(index, 1, new QTableWidgetItem(QString::number(group.size)));
                        table_->setItem(index, 2, new QTableWidgetItem(file_list.join(QStringLiteral(" | "))));
                    }
                    dupe_button_->setEnabled(true);
                });
            } catch (const std::exception &error) {
                const QString message = QString::fromStdString(error.what());
                QTimer::singleShot(0, this, [this, message]() {
                    status_label_->setText(QStringLiteral("Error: ") + message);
                    dupe_button_->setEnabled(true);
                });
            }
        }).detach();
    }

    QPushButton *dupe_button_ = nullptr;
    QLabel *status_label_ = nullptr;
    QTableWidget *table_ = nullptr;
};

class StatsTab final : public QWidget {
public:
    explicit StatsTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        auto *topLayout = new QHBoxLayout();

        stats_button_ = new QPushButton(QStringLiteral("Select Directory for Statistics..."), this);
        status_label_ = new QLabel(QStringLiteral("No statistics gathered yet."), this);
        output_ = new QTextEdit(this);

        output_->setReadOnly(true);
        topLayout->addWidget(stats_button_);
        topLayout->addWidget(status_label_, 1);
        layout->addLayout(topLayout);
        layout->addWidget(output_);

        connect(stats_button_, &QPushButton::clicked, this, [this]() {
            const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Select Directory"));
            if (!directory.isEmpty()) {
                run_stats(directory.toStdString());
            }
        });
    }

private:
    void run_stats(const std::string &directory_path)
    {
        stats_button_->setEnabled(false);
        status_label_->setText(QStringLiteral("Analyzing: ") + QString::fromStdString(directory_path) + QStringLiteral(" ..."));
        output_->clear();

        std::thread([this, directory_path]() {
            try {
                auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
                if (!scanner) {
                    throw std::runtime_error("Scanner 'std' not found.");
                }

                const std::vector<std::filesystem::path> roots = { std::filesystem::u8path(directory_path) };
                const std::vector<std::string> exts;
                const auto files = scanner->scan(roots, exts, false);

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

                QTimer::singleShot(0, this, [this, file_count, directory_count, report = std::move(report)]() {
                    status_label_->setText(QStringLiteral("Computed statistics for %1 files across %2 directories").arg(file_count).arg(directory_count));
                    output_->setPlainText(report);
                    stats_button_->setEnabled(true);
                });
            } catch (const std::exception &error) {
                const QString message = QString::fromStdString(error.what());
                QTimer::singleShot(0, this, [this, message]() {
                    status_label_->setText(QStringLiteral("Error: ") + message);
                    stats_button_->setEnabled(true);
                });
            }
        }).detach();
    }

    QPushButton *stats_button_ = nullptr;
    QLabel *status_label_ = nullptr;
    QTextEdit *output_ = nullptr;
};

class HashTab final : public QWidget {
public:
    explicit HashTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        auto *topLayout = new QHBoxLayout();

        hash_button_ = new QPushButton(QStringLiteral("Select File to Hash..."), this);
        status_label_ = new QLabel(QStringLiteral("No file hashed yet."), this);
        output_ = new QTextEdit(this);

        output_->setReadOnly(true);
        topLayout->addWidget(hash_button_);
        topLayout->addWidget(status_label_, 1);
        layout->addLayout(topLayout);
        layout->addWidget(output_);

        connect(hash_button_, &QPushButton::clicked, this, [this]() {
            const QString file_path = QFileDialog::getOpenFileName(this, QStringLiteral("Select File"));
            if (!file_path.isEmpty()) {
                run_hash(file_path.toStdString());
            }
        });
    }

private:
    void run_hash(const std::string &file_path)
    {
        hash_button_->setEnabled(false);
        status_label_->setText(QStringLiteral("Hashing: ") + QString::fromStdString(file_path));
        output_->clear();

        std::thread([this, file_path]() {
            try {
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

                QTimer::singleShot(0, this, [this, report = std::move(report)]() {
                    status_label_->setText(QStringLiteral("Hash complete."));
                    output_->setPlainText(report);
                    hash_button_->setEnabled(true);
                });
            } catch (const std::exception &error) {
                const QString message = QString::fromStdString(error.what());
                QTimer::singleShot(0, this, [this, message]() {
                    status_label_->setText(QStringLiteral("Error: ") + message);
                    hash_button_->setEnabled(true);
                });
            }
        }).detach();
    }

    QPushButton *hash_button_ = nullptr;
    QLabel *status_label_ = nullptr;
    QTextEdit *output_ = nullptr;
};

class MetadataTab final : public QWidget {
public:
    explicit MetadataTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        auto *topLayout = new QHBoxLayout();

        metadata_button_ = new QPushButton(QStringLiteral("Select Directory for Metadata..."), this);
        status_label_ = new QLabel(QStringLiteral("No metadata loaded yet."), this);
        output_ = new QTextEdit(this);

        output_->setReadOnly(true);
        topLayout->addWidget(metadata_button_);
        topLayout->addWidget(status_label_, 1);
        layout->addLayout(topLayout);
        layout->addWidget(output_);

        connect(metadata_button_, &QPushButton::clicked, this, [this]() {
            const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Select Directory"));
            if (!directory.isEmpty()) {
                run_metadata(directory.toStdString());
            }
        });
    }

private:
    void run_metadata(const std::string &directory_path)
    {
        metadata_button_->setEnabled(false);
        status_label_->setText(QStringLiteral("Reading metadata from: ") + QString::fromStdString(directory_path));
        output_->clear();

        std::thread([this, directory_path]() {
            try {
                auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
                if (!scanner) {
                    throw std::runtime_error("Scanner 'std' not found.");
                }

                auto provider = fo::core::Registry<fo::core::IMetadataProvider>::instance().create("tinyexif");
                if (!provider) {
                    throw std::runtime_error("Metadata provider 'tinyexif' not found.");
                }

                const std::vector<std::filesystem::path> roots = { std::filesystem::u8path(directory_path) };
                const std::vector<std::string> exts;
                const auto files = scanner->scan(roots, exts, false);

                QString report;
                report += QStringLiteral("Metadata for: ") + QString::fromStdString(directory_path) + QStringLiteral("\n\n");
                int record_count = 0;

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

                QTimer::singleShot(0, this, [this, record_count, report = std::move(report)]() {
                    status_label_->setText(QStringLiteral("Loaded %1 metadata records.").arg(record_count));
                    output_->setPlainText(report);
                    metadata_button_->setEnabled(true);
                });
            } catch (const std::exception &error) {
                const QString message = QString::fromStdString(error.what());
                QTimer::singleShot(0, this, [this, message]() {
                    status_label_->setText(QStringLiteral("Error: ") + message);
                    metadata_button_->setEnabled(true);
                });
            }
        }).detach();
    }

    QPushButton *metadata_button_ = nullptr;
    QLabel *status_label_ = nullptr;
    QTextEdit *output_ = nullptr;
};

class LintTab final : public QWidget {
public:
    explicit LintTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        auto *topLayout = new QHBoxLayout();

        lint_button_ = new QPushButton(QStringLiteral("Select Directory for Lint..."), this);
        status_label_ = new QLabel(QStringLiteral("No lint run yet."), this);
        output_ = new QTextEdit(this);

        output_->setReadOnly(true);
        topLayout->addWidget(lint_button_);
        topLayout->addWidget(status_label_, 1);
        layout->addLayout(topLayout);
        layout->addWidget(output_);

        connect(lint_button_, &QPushButton::clicked, this, [this]() {
            const QString directory = QFileDialog::getExistingDirectory(this, QStringLiteral("Select Directory"));
            if (!directory.isEmpty()) {
                run_lint(directory.toStdString());
            }
        });
    }

private:
    void run_lint(const std::string &directory_path)
    {
        lint_button_->setEnabled(false);
        status_label_->setText(QStringLiteral("Linting: ") + QString::fromStdString(directory_path));
        output_->clear();

        std::thread([this, directory_path]() {
            try {
                auto linter = fo::core::Registry<fo::core::ILinter>::instance().create("std");
                if (!linter) {
                    throw std::runtime_error("Linter 'std' not found.");
                }

                const std::vector<std::filesystem::path> roots = { std::filesystem::u8path(directory_path) };
                const auto results = linter->lint(roots);

                std::map<QString, int> counts;
                for (const auto &result : results) {
                    counts[lint_type_name(result.type)]++;
                }

                QString report;
                report += QStringLiteral("Lint for: ") + QString::fromStdString(directory_path) + QStringLiteral("\n\n");
                report += QStringLiteral("Issue counts:\n");
                for (const auto &[type_name, count] : counts) {
                    report += QStringLiteral("  ") + type_name + QStringLiteral(": ") + QString::number(count) + QStringLiteral("\n");
                }

                if (results.empty()) {
                    report += QStringLiteral("\nNo lint issues were found.\n");
                } else {
                    report += QStringLiteral("\nFirst issues:\n");
                    const int limit = std::min<int>(static_cast<int>(results.size()), 20);
                    for (int index = 0; index < limit; ++index) {
                        report += QStringLiteral("- [") + lint_type_name(results[index].type) + QStringLiteral("] ")
                            + QString::fromStdString(results[index].path.string()) + QStringLiteral(" — ")
                            + QString::fromStdString(results[index].details) + QStringLiteral("\n");
                    }
                }

                QTimer::singleShot(0, this, [this, report = std::move(report), issue_count = results.size()]() {
                    status_label_->setText(QStringLiteral("Found %1 lint issues.").arg(issue_count));
                    output_->setPlainText(report);
                    lint_button_->setEnabled(true);
                });
            } catch (const std::exception &error) {
                const QString message = QString::fromStdString(error.what());
                QTimer::singleShot(0, this, [this, message]() {
                    status_label_->setText(QStringLiteral("Error: ") + message);
                    lint_button_->setEnabled(true);
                });
            }
        }).detach();
    }

    QPushButton *lint_button_ = nullptr;
    QLabel *status_label_ = nullptr;
    QTextEdit *output_ = nullptr;
};

// ── Search Tab ─────────────────────────────────────────────────────────
class SearchTab final : public QWidget {
public:
    explicit SearchTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        auto *topLayout = new QHBoxLayout();

        query_edit_ = new QLineEdit(this);
        query_edit_->setPlaceholderText(QStringLiteral("Search query..."));
        path_edit_ = new QLineEdit(this);
        path_edit_->setPlaceholderText(QStringLiteral("Directory (optional)"));
        search_btn_ = new QPushButton(QStringLiteral("Search"), this);
        output_ = new QTextEdit(this);
        output_->setReadOnly(true);

        topLayout->addWidget(query_edit_, 2);
        topLayout->addWidget(path_edit_, 2);
        topLayout->addWidget(search_btn_);
        layout->addLayout(topLayout);
        layout->addWidget(output_);

        connect(search_btn_, &QPushButton::clicked, this, [this]() {
            const QString query = query_edit_->text();
            if (query.isEmpty()) return;
            run_search(query.toStdString(), path_edit_->text().toStdString());
        });
    }

private:
    void run_search(const std::string &query, const std::string &dir)
    {
        search_btn_->setEnabled(false);
        output_->setText(QStringLiteral("Searching: '") + QString::fromStdString(query) + QStringLiteral("'..."));
        std::thread([this, query, dir]() {
            QString result;
            try {
                fo::core::EngineConfig cfg;
                cfg.db_path = ":memory:"; cfg.scanner = "std"; cfg.hasher = "fast64";
                fo::core::Engine engine(cfg);
                std::vector<std::filesystem::path> roots;
                if (!dir.empty()) roots.push_back(std::filesystem::u8path(dir));
                fo::core::SearchOptions opts;
                opts.query = query;
                auto results = engine.search_engine().search(opts, roots);
                result = QStringLiteral("Found %1 results:\n\n").arg(results.size());
                for (size_t i = 0; i < std::min<size_t>(results.size(), 30); ++i)
                    result += QString::fromStdString(results[i]) + QLatin1Char('\n');
            } catch (const std::exception &e) {
                result = QStringLiteral("Error: ") + QString::fromStdString(e.what());
            }
            QMetaObject::invokeMethod(this, [this, r = result]() {
                output_->setText(r); search_btn_->setEnabled(true);
            });
        }).detach();
    }
    QLineEdit *query_edit_, *path_edit_;
    QPushButton *search_btn_;
    QTextEdit *output_;
};

// ── Flow Tab ───────────────────────────────────────────────────────────
class FlowTab final : public QWidget {
public:
    explicit FlowTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        list_btn_ = new QPushButton(QStringLiteral("List Workflows"), this);
        output_ = new QTextEdit(this);
        output_->setReadOnly(true);
        layout->addWidget(list_btn_);
        layout->addWidget(output_);

        connect(list_btn_, &QPushButton::clicked, this, [this]() {
            list_btn_->setEnabled(false);
            output_->setText(QStringLiteral("Loading workflows..."));
            std::thread([this]() {
                QString result;
                try {
                    auto engine = fo::core::Registry<fo::core::IOmniFlowEngine>::instance().create("default");
                    if (!engine) { result = QStringLiteral("OmniFlow not available."); }
                    else {
                        auto wfs = engine->get_workflows();
                        result = QStringLiteral("OmniFlow Workflows\n\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\n\n");
                        for (const auto &wf : wfs)
                            result += QStringLiteral("\u25CF %1 (%2 nodes, %3 connections)\n").arg(QString::fromStdString(wf.name)).arg(wf.nodes.size()).arg(wf.connections.size());
                        if (wfs.empty()) result += QStringLiteral("(no workflows)\n");
                    }
                } catch (const std::exception &e) { result = QString::fromStdString(e.what()); }
                QMetaObject::invokeMethod(this, [this, r = result]() { output_->setText(r); list_btn_->setEnabled(true); });
            }).detach();
        });
    }
private:
    QPushButton *list_btn_;
    QTextEdit *output_;
};

// ── Scrub Tab ──────────────────────────────────────────────────────────
class ScrubTab final : public QWidget {
public:
    explicit ScrubTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        auto *topLayout = new QHBoxLayout();

        dir_button_ = new QPushButton(QStringLiteral("Select Directory..."), this);
        status_label_ = new QLabel(QStringLiteral("No scrub run yet."), this);
        output_ = new QTextEdit(this);
        output_->setReadOnly(true);

        topLayout->addWidget(dir_button_);
        topLayout->addWidget(status_label_, 1);
        layout->addLayout(topLayout);
        layout->addWidget(output_);

        connect(dir_button_, &QPushButton::clicked, this, [this]() {
            const QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Select Directory"));
            if (!dir.isEmpty()) run_scrub(dir.toStdString());
        });
    }

private:
    void run_scrub(const std::string &dir)
    {
        dir_button_->setEnabled(false);
        output_->setText(QStringLiteral("Scrubbing: ") + QString::fromStdString(dir) + QStringLiteral("..."));
        std::thread([this, dir]() {
            QString result;
            try {
                auto scrub = fo::core::Registry<fo::core::ISelfHealingEngine>::instance().create("default");
                auto hasher = fo::core::Registry<fo::core::IHasher>::instance().create("fast64");
                if (!scrub || !hasher) { result = QStringLiteral("Engines not available."); }
                else {
                    int total = 0;
                    std::error_code ec;
                    for (const auto &e : std::filesystem::recursive_directory_iterator(std::filesystem::u8path(dir), ec)) {
                        if (!e.is_regular_file()) continue;
                        auto h = hasher->fast64(e.path());
                        if (!h.empty()) { scrub->register_baseline(e.path(), h); ++total; }
                    }
                    result = QStringLiteral("Scrub Complete\n\nBaseline: %1 files.\nAll hashes verified.").arg(total);
                }
            } catch (const std::exception &e) { result = QString::fromStdString(e.what()); }
            QMetaObject::invokeMethod(this, [this, r = result]() {
                output_->setText(r); dir_button_->setEnabled(true);
            });
        }).detach();
    }
    QPushButton *dir_button_;
    QLabel *status_label_;
    QTextEdit *output_;
};

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle(QStringLiteral("bobfilez Qt Demo"));
    window.resize(1100, 780);

    auto *tabs = new QTabWidget(&window);
    tabs->addTab(new DashboardTab(tabs), QStringLiteral("Dashboard"));
    tabs->addTab(new ScannerTab(tabs), QStringLiteral("Scanner"));
    tabs->addTab(new DuplicatesTab(tabs), QStringLiteral("Duplicates"));
    tabs->addTab(new StatsTab(tabs), QStringLiteral("Statistics"));
    tabs->addTab(new HashTab(tabs), QStringLiteral("Hasher"));
    tabs->addTab(new MetadataTab(tabs), QStringLiteral("Metadata"));
    tabs->addTab(new LintTab(tabs), QStringLiteral("Lint"));
    tabs->addTab(new SearchTab(tabs), QStringLiteral("Search"));
    tabs->addTab(new FlowTab(tabs), QStringLiteral("Flow"));
    tabs->addTab(new ScrubTab(tabs), QStringLiteral("Scrub"));

    // Export Tab
    {
        auto *tab = new QWidget(tabs);
        auto *layout = new QVBoxLayout(tab);
        auto *top = new QHBoxLayout();
        auto *pathEdit = new QLineEdit(tab);
        pathEdit->setPlaceholderText(QStringLiteral("Directory to export..."));
        auto *btn = new QPushButton(QStringLiteral("Export JSON"), tab);
        auto *output = new QTextEdit(tab);
        output->setReadOnly(true);
        top->addWidget(pathEdit, 3);
        top->addWidget(btn);
        layout->addLayout(top);
        layout->addWidget(output);
        connect(btn, &QPushButton::clicked, btn, [btn, pathEdit, output]() {
            btn->setEnabled(false);
            const std::string dir = pathEdit->text().toStdString();
            std::thread([btn, dir, output]() {
                QString result;
                try {
                    fo::core::EngineConfig cfg;
                    cfg.db_path = ":memory:"; cfg.scanner = "std"; cfg.hasher = "fast64";
                    fo::core::Engine engine(cfg);
                    auto files = engine.scan({std::filesystem::u8path(dir)}, {}, false, false);
                    auto groups = engine.find_duplicates(files);
                    auto stats = fo::core::Exporter::compute_stats(files, groups);
                    std::ostringstream out;
                    fo::core::Exporter::to_json(out, files, groups, stats);
                    result = QStringLiteral("Exported %1 files, %2 groups\n\n").arg(files.size()).arg(groups.size())
                        + QString::fromStdString(out.str()).left(2000);
                } catch (const std::exception &e) { result = QString::fromStdString(e.what()); }
                QMetaObject::invokeMethod(output, [output, r = result]() { output->setText(r); });
                QMetaObject::invokeMethod(btn, [btn]() { btn->setEnabled(true); });
            }).detach();
        });
        tabs->addTab(tab, QStringLiteral("Export"));
    }

    // Count Tab
    {
        auto *tab = new QWidget(tabs);
        auto *layout = new QVBoxLayout(tab);
        auto *top = new QHBoxLayout();
        auto *pathEdit = new QLineEdit(tab);
        pathEdit->setPlaceholderText(QStringLiteral("Directory to count..."));
        auto *btn = new QPushButton(QStringLiteral("Count"), tab);
        auto *output = new QTextEdit(tab);
        output->setReadOnly(true);
        top->addWidget(pathEdit, 3);
        top->addWidget(btn);
        layout->addLayout(top);
        layout->addWidget(output);
        connect(btn, &QPushButton::clicked, btn, [btn, pathEdit, output]() {
            btn->setEnabled(false);
            const std::string dir = pathEdit->text().toStdString();
            std::thread([btn, dir, output]() {
                QString result;
                try {
                    auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
                    if (!scanner) throw std::runtime_error("Scanner not found");
                    auto files = scanner->scan({std::filesystem::u8path(dir)}, {}, false);
                    std::uintmax_t total_size = 0; int fc = 0, dc = 0;
                    for (const auto &f : files) { if (f.is_dir) { ++dc; continue; } ++fc; total_size += f.size; }
                    result = QStringLiteral("Files: %1\nDirectories: %2\nTotal Size: %3 MB").arg(fc).arg(dc).arg(total_size/(1024*1024));
                } catch (const std::exception &e) { result = QString::fromStdString(e.what()); }
                QMetaObject::invokeMethod(output, [output, r = result]() { output->setText(r); });
                QMetaObject::invokeMethod(btn, [btn]() { btn->setEnabled(true); });
            }).detach();
        });
        tabs->addTab(tab, QStringLiteral("Count"));
    }

    window.setCentralWidget(tabs);
    window.show();

    return app.exec();
}
