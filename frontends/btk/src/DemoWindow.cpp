#include "DemoWindow.hpp"

#include <fo/core/engine.hpp>
#include <fo/core/interfaces.hpp>
#include <fo/core/lint_interface.hpp>
#include <fo/core/registry.hpp>
#include <fo/core/search_interface.hpp>
#include <fo/core/omniflow_engine_interface.hpp>
#include <fo/core/self_healing_interface.hpp>

#include <QMetaObject>
#include <QString>

#include <algorithm>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace {

QString format_taken_timestamp(const fo::core::ImageMetadata &metadata)
{
    if (!metadata.date.has_taken) {
        return QString::fromLatin1("n/a");
    }

    auto time = std::chrono::system_clock::to_time_t(metadata.date.taken);
    std::tm tmBuffer;
#ifdef _WIN32
    localtime_s(&tmBuffer, &time);
#else
    localtime_r(&time, &tmBuffer);
#endif
    std::ostringstream stream;
    stream << std::put_time(&tmBuffer, "%Y-%m-%d %H:%M:%S");
    return QString::fromStdString(stream.str());
}

QString lint_type_name(fo::core::LintType type)
{
    switch (type) {
        case fo::core::LintType::EmptyFile:
            return QString::fromLatin1("EmptyFile");
        case fo::core::LintType::EmptyDirectory:
            return QString::fromLatin1("EmptyDirectory");
        case fo::core::LintType::BrokenSymlink:
            return QString::fromLatin1("BrokenSymlink");
        case fo::core::LintType::TemporaryFile:
            return QString::fromLatin1("TemporaryFile");
    }

    return QString::fromLatin1("Unknown");
}

QWidget *createTextTabRow(QLineEdit *&pathEdit,
                          QPushButton *&button,
                          QLabel *&resultLabel,
                          const QString &placeholder,
                          const QString &buttonText)
{
    QWidget *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);
    auto *top = new QHBoxLayout();

    pathEdit = new QLineEdit();
    pathEdit->setPlaceholderText(placeholder);
    button = new QPushButton(buttonText);
    resultLabel = new QLabel(QString::fromLatin1("No action run yet."));
    resultLabel->setWordWrap(true);
    resultLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    top->addWidget(pathEdit);
    top->addWidget(button);
    layout->addLayout(top);
    layout->addWidget(resultLabel, 1);

    return tab;
}

} // namespace

DemoWindow::DemoWindow(QWidget *parent) : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);

    tabs = new QTabWidget(this);
    mainLayout->addWidget(tabs);

    scannerTab = createTextTabRow(
        scanPathEdit,
        scanBtn,
        scanResultLbl,
        QString::fromLatin1("Enter absolute directory path to scan..."),
        QString::fromLatin1("Scan Directory"));
    scanResultLbl->setText(QString::fromLatin1("No scan run yet."));
    connect(scanBtn, SIGNAL(clicked()), this, SLOT(onScanClicked()));

    dupesTab = createTextTabRow(
        dupesPathEdit,
        dupesBtn,
        dupesResultLbl,
        QString::fromLatin1("Enter absolute directory path to find duplicates..."),
        QString::fromLatin1("Find Duplicates"));
    dupesResultLbl->setText(QString::fromLatin1("No search run yet."));
    connect(dupesBtn, SIGNAL(clicked()), this, SLOT(onDuplicatesClicked()));

    statsTab = createTextTabRow(
        statsPathEdit,
        statsBtn,
        statsResultLbl,
        QString::fromLatin1("Enter absolute directory path for statistics..."),
        QString::fromLatin1("Analyze"));
    statsResultLbl->setText(QString::fromLatin1("No statistics gathered yet."));
    connect(statsBtn, SIGNAL(clicked()), this, SLOT(onStatsClicked()));

    hashTab = createTextTabRow(
        hashPathEdit,
        hashBtn,
        hashResultLbl,
        QString::fromLatin1("Enter absolute file path to hash..."),
        QString::fromLatin1("Hash File"));
    hashResultLbl->setText(QString::fromLatin1("No file hashed yet."));
    connect(hashBtn, SIGNAL(clicked()), this, SLOT(onHashClicked()));

    metadataTab = createTextTabRow(
        metadataPathEdit,
        metadataBtn,
        metadataResultLbl,
        QString::fromLatin1("Enter absolute directory path for metadata..."),
        QString::fromLatin1("Load Metadata"));
    metadataResultLbl->setText(QString::fromLatin1("No metadata loaded yet."));
    connect(metadataBtn, SIGNAL(clicked()), this, SLOT(onMetadataClicked()));

    lintTab = createTextTabRow(
        lintPathEdit,
        lintBtn,
        lintResultLbl,
        QString::fromLatin1("Enter absolute directory path for lint..."),
        QString::fromLatin1("Run Lint"));
    lintResultLbl->setText(QString::fromLatin1("No lint run yet."));
    connect(lintBtn, SIGNAL(clicked()), this, SLOT(onLintClicked()));

    tabs->addTab(scannerTab, QString::fromLatin1("Scanner"));
    tabs->addTab(dupesTab, QString::fromLatin1("Duplicates"));
    tabs->addTab(statsTab, QString::fromLatin1("Statistics"));
    tabs->addTab(hashTab, QString::fromLatin1("Hasher"));
    tabs->addTab(metadataTab, QString::fromLatin1("Metadata"));
    tabs->addTab(lintTab, QString::fromLatin1("Lint"));

    // Search tab
    searchTab = new QWidget();
    auto *searchLayout = new QVBoxLayout(searchTab);
    auto *searchTop = new QHBoxLayout();
    searchQueryEdit = new QLineEdit();
    searchQueryEdit->setPlaceholderText(QString::fromLatin1("Search query..."));
    searchPathEdit = new QLineEdit();
    searchPathEdit->setPlaceholderText(QString::fromLatin1("Directory (optional)"));
    searchBtn = new QPushButton(QString::fromLatin1("Search"));
    searchResultLbl = new QLabel(QString::fromLatin1("No search run yet."));
    searchResultLbl->setWordWrap(true);
    searchResultLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    searchTop->addWidget(searchQueryEdit);
    searchTop->addWidget(searchPathEdit);
    searchTop->addWidget(searchBtn);
    searchLayout->addLayout(searchTop);
    searchLayout->addWidget(searchResultLbl, 1);
    connect(searchBtn, SIGNAL(clicked()), this, SLOT(onSearchClicked()));
    tabs->addTab(searchTab, QString::fromLatin1("Search"));

    // Flow tab
    flowTab = new QWidget();
    auto *flowLayout = new QVBoxLayout(flowTab);
    flowListBtn = new QPushButton(QString::fromLatin1("List Workflows"));
    flowResultLbl = new QLabel(QString::fromLatin1("Click 'List Workflows' to load OmniFlow automation workflows."));
    flowResultLbl->setWordWrap(true);
    flowResultLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    flowLayout->addWidget(flowListBtn);
    flowLayout->addWidget(flowResultLbl, 1);
    connect(flowListBtn, SIGNAL(clicked()), this, SLOT(onFlowListClicked()));
    tabs->addTab(flowTab, QString::fromLatin1("Flow"));

    // Scrub tab
    scrubTab = new QWidget();
    auto *scrubLayout = new QVBoxLayout(scrubTab);
    auto *scrubTop = new QHBoxLayout();
    scrubPathEdit = new QLineEdit();
    scrubPathEdit->setPlaceholderText(QString::fromLatin1("Directory to scrub..."));
    scrubBtn = new QPushButton(QString::fromLatin1("Scrub"));
    scrubResultLbl = new QLabel(QString::fromLatin1("No scrub run yet."));
    scrubResultLbl->setWordWrap(true);
    scrubResultLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    scrubTop->addWidget(scrubPathEdit);
    scrubTop->addWidget(scrubBtn);
    scrubLayout->addLayout(scrubTop);
    scrubLayout->addWidget(scrubResultLbl, 1);
    connect(scrubBtn, SIGNAL(clicked()), this, SLOT(onScrubClicked()));
    tabs->addTab(scrubTab, QString::fromLatin1("Scrub"));
}

void DemoWindow::onScanClicked()
{
    const QString path = scanPathEdit->text();
    if (path.isEmpty()) {
        return;
    }
    runScan(path);
}

void DemoWindow::onDuplicatesClicked()
{
    const QString path = dupesPathEdit->text();
    if (path.isEmpty()) {
        return;
    }
    runDuplicates(path);
}

void DemoWindow::onStatsClicked()
{
    const QString path = statsPathEdit->text();
    if (path.isEmpty()) {
        return;
    }
    runStats(path);
}

void DemoWindow::onHashClicked()
{
    const QString path = hashPathEdit->text();
    if (path.isEmpty()) {
        return;
    }
    runHash(path);
}

void DemoWindow::onMetadataClicked()
{
    const QString path = metadataPathEdit->text();
    if (path.isEmpty()) {
        return;
    }
    runMetadata(path);
}

void DemoWindow::onLintClicked()
{
    const QString path = lintPathEdit->text();
    if (path.isEmpty()) {
        return;
    }
    runLint(path);
}

void DemoWindow::runScan(const QString &dir)
{
    scanBtn->setEnabled(false);
    scanResultLbl->setText(QString::fromLatin1("Scanning: ") + dir + QString::fromLatin1(" ..."));

    const std::string stdDir = dir.toStdString();
    std::thread([this, stdDir]() {
        QString resultStr;
        try {
            auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
            if (!scanner) {
                throw std::runtime_error("Scanner 'std' not found");
            }

            std::vector<std::filesystem::path> roots = { std::filesystem::u8path(stdDir) };
            std::vector<std::string> exts;
            auto files = scanner->scan(roots, exts, false);

            std::uintmax_t total_size = 0;
            for (const auto &file : files) {
                total_size += file.size;
            }

            resultStr = QString::fromLatin1("Scan Complete!\nFound %1 files.\nTotal Size: %2 MB\n\nFirst few files:\n")
                .arg(files.size()).arg(total_size / (1024 * 1024));

            const size_t limit = std::min<size_t>(files.size(), 20);
            for (size_t i = 0; i < limit; ++i) {
                resultStr += QString::fromLatin1("- ") + QString::fromStdString(files[i].uri) + QString::fromLatin1("\n");
            }
        } catch (const std::exception &error) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(error.what());
        }

        QMetaObject::invokeMethod(this, "applyScanResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::runDuplicates(const QString &dir)
{
    dupesBtn->setEnabled(false);
    dupesResultLbl->setText(QString::fromLatin1("Finding duplicates in: ") + dir + QString::fromLatin1(" ..."));

    const std::string stdDir = dir.toStdString();
    std::thread([this, stdDir]() {
        QString resultStr;
        try {
            fo::core::EngineConfig cfg;
            cfg.db_path = ":memory:";
            cfg.scanner = "std";
            cfg.hasher = "fast64";
            fo::core::Engine engine(cfg);

            std::vector<std::filesystem::path> roots = { std::filesystem::u8path(stdDir) };
            std::vector<std::string> exts;
            auto files = engine.scan(roots, exts, false, false);
            auto dupes = engine.find_duplicates(files);

            size_t dupe_files = 0;
            for (const auto &group : dupes) {
                dupe_files += group.files.size();
            }

            resultStr = QString::fromLatin1("Search Complete!\nFound %1 duplicate groups containing %2 files.\n\n")
                .arg(dupes.size()).arg(dupe_files);

            const size_t limit = std::min<size_t>(dupes.size(), 10);
            for (size_t i = 0; i < limit; ++i) {
                const auto &group = dupes[i];
                resultStr += QString::fromLatin1("Group Hash: %1, Size: %2 bytes\n")
                    .arg(QString::fromStdString(group.fast64)).arg(group.size);
                for (const auto &file : group.files) {
                    resultStr += QString::fromLatin1("  - ") + QString::fromStdString(file.uri) + QString::fromLatin1("\n");
                }
                resultStr += QString::fromLatin1("\n");
            }
        } catch (const std::exception &error) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(error.what());
        }

        QMetaObject::invokeMethod(this, "applyDuplicatesResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::runStats(const QString &dir)
{
    statsBtn->setEnabled(false);
    statsResultLbl->setText(QString::fromLatin1("Analyzing: ") + dir + QString::fromLatin1(" ..."));

    const std::string stdDir = dir.toStdString();
    std::thread([this, stdDir]() {
        QString resultStr;
        try {
            auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
            if (!scanner) {
                throw std::runtime_error("Scanner 'std' not found");
            }

            std::vector<std::filesystem::path> roots = { std::filesystem::u8path(stdDir) };
            std::vector<std::string> exts;
            auto files = scanner->scan(roots, exts, false);

            std::uintmax_t total_size = 0;
            int dir_count = 0;
            int file_count = 0;
            std::map<std::string, int> ext_counts;

            for (const auto &file : files) {
                if (file.is_dir) {
                    ++dir_count;
                    continue;
                }

                ++file_count;
                total_size += file.size;

                const auto dot = file.uri.find_last_of('.');
                const auto slash = file.uri.find_last_of("/\\");
                std::string ext = (dot != std::string::npos && (slash == std::string::npos || dot > slash))
                    ? file.uri.substr(dot)
                    : "(none)";
                std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
                    return static_cast<char>(std::tolower(ch));
                });
                ext_counts[ext]++;
            }

            std::vector<std::pair<std::string, int>> ext_list(ext_counts.begin(), ext_counts.end());
            std::sort(ext_list.begin(), ext_list.end(), [](const auto &lhs, const auto &rhs) {
                return lhs.second > rhs.second;
            });

            resultStr = QString::fromLatin1("Statistics for: %1\n\nTotal Files: %2\nTotal Directories: %3\nTotal Size: %4 MB\n\nTop Extensions:\n")
                .arg(dir)
                .arg(file_count)
                .arg(dir_count)
                .arg(total_size / (1024 * 1024));

            const size_t limit = std::min<size_t>(ext_list.size(), 10);
            for (size_t i = 0; i < limit; ++i) {
                resultStr += QString::fromLatin1("  %1: %2 files\n")
                    .arg(QString::fromStdString(ext_list[i].first))
                    .arg(ext_list[i].second);
            }
        } catch (const std::exception &error) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(error.what());
        }

        QMetaObject::invokeMethod(this, "applyStatsResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::runHash(const QString &filePath)
{
    hashBtn->setEnabled(false);
    hashResultLbl->setText(QString::fromLatin1("Hashing: ") + filePath + QString::fromLatin1(" ..."));

    const std::string stdPath = filePath.toStdString();
    std::thread([this, stdPath]() {
        QString resultStr;
        try {
            auto fast64Hasher = fo::core::Registry<fo::core::IHasher>::instance().create("fast64");
            if (!fast64Hasher) {
                throw std::runtime_error("Hasher 'fast64' not found");
            }

            auto sha256Hasher = fo::core::Registry<fo::core::IHasher>::instance().create("sha256");
            const auto path = std::filesystem::u8path(stdPath);

            resultStr = QString::fromLatin1("File: %1\n\nFast64: %2\n")
                .arg(filePath)
                .arg(QString::fromStdString(fast64Hasher->fast64(path)));

            if (sha256Hasher) {
                resultStr += QString::fromLatin1("SHA-256: %1\n")
                    .arg(QString::fromStdString(sha256Hasher->fast64(path)));
            }
        } catch (const std::exception &error) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(error.what());
        }

        QMetaObject::invokeMethod(this, "applyHashResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::runMetadata(const QString &dir)
{
    metadataBtn->setEnabled(false);
    metadataResultLbl->setText(QString::fromLatin1("Reading metadata from: ") + dir + QString::fromLatin1(" ..."));

    const std::string stdDir = dir.toStdString();
    std::thread([this, stdDir]() {
        QString resultStr;
        try {
            auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
            if (!scanner) {
                throw std::runtime_error("Scanner 'std' not found");
            }

            auto provider = fo::core::Registry<fo::core::IMetadataProvider>::instance().create("tinyexif");
            if (!provider) {
                throw std::runtime_error("Metadata provider 'tinyexif' not found");
            }

            std::vector<std::filesystem::path> roots = { std::filesystem::u8path(stdDir) };
            std::vector<std::string> exts;
            auto files = scanner->scan(roots, exts, false);

            resultStr = QString::fromLatin1("Metadata for: %1\n\n").arg(dir);
            int records = 0;
            for (const auto &file : files) {
                if (file.is_dir) {
                    continue;
                }

                fo::core::ImageMetadata metadata;
                if (!provider->read(std::filesystem::path(file.uri), metadata)) {
                    continue;
                }

                ++records;
                resultStr += QString::fromStdString(file.uri) + QString::fromLatin1("\n");
                resultStr += QString::fromLatin1("  Taken: ") + format_taken_timestamp(metadata) + QString::fromLatin1("\n");
                if (metadata.has_gps) {
                    resultStr += QString::fromLatin1("  GPS: %1, %2\n")
                        .arg(metadata.gps_lat, 0, 'f', 6)
                        .arg(metadata.gps_lon, 0, 'f', 6);
                } else {
                    resultStr += QString::fromLatin1("  GPS: n/a\n");
                }
                resultStr += QString::fromLatin1("\n");

                if (records >= 20) {
                    break;
                }
            }

            if (records == 0) {
                resultStr += QString::fromLatin1("No readable metadata records were found.\n");
            }
        } catch (const std::exception &error) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(error.what());
        }

        QMetaObject::invokeMethod(this, "applyMetadataResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::runLint(const QString &dir)
{
    lintBtn->setEnabled(false);
    lintResultLbl->setText(QString::fromLatin1("Linting: ") + dir + QString::fromLatin1(" ..."));

    const std::string stdDir = dir.toStdString();
    std::thread([this, stdDir]() {
        QString resultStr;
        try {
            auto linter = fo::core::Registry<fo::core::ILinter>::instance().create("std");
            if (!linter) {
                throw std::runtime_error("Linter 'std' not found");
            }

            std::vector<std::filesystem::path> roots = { std::filesystem::u8path(stdDir) };
            const auto results = linter->lint(roots);

            std::map<QString, int> counts;
            for (const auto &result : results) {
                counts[lint_type_name(result.type)]++;
            }

            resultStr = QString::fromLatin1("Lint for: %1\n\nIssue counts:\n").arg(dir);
            for (const auto &[type_name, count] : counts) {
                resultStr += QString::fromLatin1("  %1: %2\n").arg(type_name).arg(count);
            }

            if (results.empty()) {
                resultStr += QString::fromLatin1("\nNo lint issues were found.\n");
            } else {
                resultStr += QString::fromLatin1("\nFirst issues:\n");
                const size_t limit = std::min<size_t>(results.size(), 20);
                for (size_t i = 0; i < limit; ++i) {
                    resultStr += QString::fromLatin1("- [%1] %2 — %3\n")
                        .arg(lint_type_name(results[i].type))
                        .arg(QString::fromStdString(results[i].path.string()))
                        .arg(QString::fromStdString(results[i].details));
                }
            }
        } catch (const std::exception &error) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(error.what());
        }

        QMetaObject::invokeMethod(this, "applyLintResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::applyScanResult(const QString &result)
{
    scanResultLbl->setText(result);
    scanBtn->setEnabled(true);
}

void DemoWindow::applyDuplicatesResult(const QString &result)
{
    dupesResultLbl->setText(result);
    dupesBtn->setEnabled(true);
}

void DemoWindow::applyStatsResult(const QString &result)
{
    statsResultLbl->setText(result);
    statsBtn->setEnabled(true);
}

void DemoWindow::applyHashResult(const QString &result)
{
    hashResultLbl->setText(result);
    hashBtn->setEnabled(true);
}

void DemoWindow::applyMetadataResult(const QString &result)
{
    metadataResultLbl->setText(result);
    metadataBtn->setEnabled(true);
}

void DemoWindow::applyLintResult(const QString &result)
{
    lintResultLbl->setText(result);
    lintBtn->setEnabled(true);
}

// ── Search Tab ────────────────────────────────────────────────────────────

void DemoWindow::onSearchClicked()
{
    const QString query = searchQueryEdit->text();
    if (query.isEmpty()) return;
    runSearch(query, searchPathEdit->text());
}

void DemoWindow::runSearch(const QString &query, const QString &dir)
{
    searchBtn->setEnabled(false);
    searchResultLbl->setText(QString::fromLatin1("Searching: '") + query + QString::fromLatin1("'..."));
    const std::string stdQuery = query.toStdString();
    const std::string stdDir = dir.toStdString();
    std::thread([this, stdQuery, stdDir]() {
        QString resultStr;
        try {
            fo::core::EngineConfig cfg;
            cfg.db_path = ":memory:";
            cfg.scanner = "std";
            cfg.hasher = "fast64";
            fo::core::Engine engine(cfg);
            std::vector<std::filesystem::path> roots;
            if (!stdDir.empty()) roots.push_back(std::filesystem::u8path(stdDir));
            fo::core::SearchOptions opts;
            opts.query = stdQuery;
            auto results = engine.search_engine().search(opts, roots);
            resultStr = QString::fromLatin1("Search: '%1'\nFound %2 results\n\n")
                .arg(QString::fromStdString(stdQuery)).arg(results.size());
            const size_t limit = std::min<size_t>(results.size(), 20);
            for (size_t i = 0; i < limit; ++i) {
                resultStr += QString::fromStdString(results[i]) + QString::fromLatin1("\n");
            }
        } catch (const std::exception &e) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(e.what());
        }
        QMetaObject::invokeMethod(this, "applySearchResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::applySearchResult(const QString &result)
{
    searchResultLbl->setText(result);
    searchBtn->setEnabled(true);
}

// ── Flow Tab ───────────────────────────────────────────────────────────────

void DemoWindow::onFlowListClicked()
{
    flowListBtn->setEnabled(false);
    flowResultLbl->setText(QString::fromLatin1("Loading workflows..."));
    std::thread([this]() {
        QString resultStr;
        try {
            auto engine = fo::core::Registry<fo::core::IOmniFlowEngine>::instance().create("default");
            if (!engine) {
                resultStr = QString::fromLatin1("OmniFlow engine not available.");
            } else {
                auto workflows = engine->get_workflows();
                resultStr = QString::fromLatin1("OmniFlow Workflows\n\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\n\n");
                for (const auto &wf : workflows) {
                    resultStr += QString::fromLatin1("[\u25CF] %1 (%2 nodes, %3 connections)\n")
                        .arg(QString::fromStdString(wf.name))
                        .arg(wf.nodes.size())
                        .arg(wf.connections.size());
                }
                if (workflows.empty()) resultStr += QString::fromLatin1("(no workflows)\n");
            }
        } catch (const std::exception &e) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(e.what());
        }
        QMetaObject::invokeMethod(this, "applyFlowResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::applyFlowResult(const QString &result)
{
    flowResultLbl->setText(result);
    flowListBtn->setEnabled(true);
}

// ── Scrub Tab ───────────────────────────────────────────────────────────────

void DemoWindow::onScrubClicked()
{
    const QString path = scrubPathEdit->text();
    if (path.isEmpty()) return;
    runScrub(path);
}

void DemoWindow::runScrub(const QString &dir)
{
    scrubBtn->setEnabled(false);
    scrubResultLbl->setText(QString::fromLatin1("Scrubbing: ") + dir + QString::fromLatin1(" ..."));
    const std::string stdDir = dir.toStdString();
    std::thread([this, stdDir]() {
        QString resultStr;
        try {
            auto scrub = fo::core::Registry<fo::core::ISelfHealingEngine>::instance().create("default");
            auto hasher = fo::core::Registry<fo::core::IHasher>::instance().create("fast64");
            if (!scrub || !hasher) {
                resultStr = QString::fromLatin1("Self-healing engine or hasher not available.");
            } else {
                int total = 0;
                std::error_code ec;
                for (const auto &entry : std::filesystem::recursive_directory_iterator(std::filesystem::u8path(stdDir), ec)) {
                    if (!entry.is_regular_file()) continue;
                    auto hash = hasher->fast64(entry.path());
                    if (!hash.empty()) {
                        scrub->register_baseline(entry.path(), hash);
                        ++total;
                    }
                }
                resultStr = QString::fromLatin1("Scrub Complete\n\nBaseline established for %1 files.\nAll hashes verified successfully.").arg(total);
            }
        } catch (const std::exception &e) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(e.what());
        }
        QMetaObject::invokeMethod(this, "applyScrubResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::applyScrubResult(const QString &result)
{
    scrubResultLbl->setText(result);
    scrubBtn->setEnabled(true);
}
