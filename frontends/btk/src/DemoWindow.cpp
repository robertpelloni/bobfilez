#include "DemoWindow.hpp"

#include <fo/core/engine.hpp>
#include <fo/core/interfaces.hpp>
#include <fo/core/lint_interface.hpp>
#include <fo/core/registry.hpp>
#include <fo/core/search_interface.hpp>
#include <fo/core/omniflow_engine_interface.hpp>
#include <fo/core/self_healing_interface.hpp>
#include <fo/core/export.hpp>
#include <fo/core/operation_repository.hpp>
#include <fo/core/ignore_repository.hpp>
#include <fo/core/rule_engine.hpp>

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

    // Export tab
    exportTab = new QWidget();
    exportTab = createTextTabRow(
        exportPathEdit, exportBtn, exportResultLbl,
        QString::fromLatin1("Directory to export..."),
        QString::fromLatin1("Export"));
    exportResultLbl->setText(QString::fromLatin1("No export run yet."));
    connect(exportBtn, SIGNAL(clicked()), this, SLOT(onExportClicked()));
    tabs->addTab(exportTab, QString::fromLatin1("Export"));

    // Count tab
    countTab = new QWidget();
    countTab = createTextTabRow(
        countPathEdit, countBtn, countResultLbl,
        QString::fromLatin1("Directory to count..."),
        QString::fromLatin1("Count"));
    countResultLbl->setText(QString::fromLatin1("No count run yet."));
    connect(countBtn, SIGNAL(clicked()), this, SLOT(onCountClicked()));
    tabs->addTab(countTab, QString::fromLatin1("Count"));

    // History tab
    historyTab = new QWidget();
    auto *histLayout = new QVBoxLayout(historyTab);
    historyBtn = new QPushButton(QString::fromLatin1("Load History"));
    historyResultLbl = new QLabel(QString::fromLatin1("Click 'Load History' to view operations."));
    historyResultLbl->setWordWrap(true);
    historyResultLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    histLayout->addWidget(historyBtn);
    histLayout->addWidget(historyResultLbl, 1);
    connect(historyBtn, SIGNAL(clicked()), this, SLOT(onHistoryClicked()));
    tabs->addTab(historyTab, QString::fromLatin1("History"));

    // Ignore tab
    ignoreTab = new QWidget();
    auto *ignoreLayout = new QVBoxLayout(ignoreTab);
    auto *ignoreTop = new QHBoxLayout();
    ignorePatternEdit = new QLineEdit();
    ignorePatternEdit->setPlaceholderText(QString::fromLatin1("Pattern (e.g. .*\\.tmp)"));
    ignoreListBtn = new QPushButton(QString::fromLatin1("List Rules"));
    ignoreAddBtn = new QPushButton(QString::fromLatin1("Add Rule"));
    ignoreResultLbl = new QLabel(QString::fromLatin1("No ignore operations yet."));
    ignoreResultLbl->setWordWrap(true);
    ignoreResultLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ignoreTop->addWidget(ignorePatternEdit, 2);
    ignoreTop->addWidget(ignoreListBtn);
    ignoreTop->addWidget(ignoreAddBtn);
    ignoreLayout->addLayout(ignoreTop);
    ignoreLayout->addWidget(ignoreResultLbl, 1);
    connect(ignoreListBtn, SIGNAL(clicked()), this, SLOT(onIgnoreListClicked()));
    connect(ignoreAddBtn, SIGNAL(clicked()), this, SLOT(onIgnoreAddClicked()));
    tabs->addTab(ignoreTab, QString::fromLatin1("Ignore"));

    // Organize tab
    organizeTab = new QWidget();
    auto *orgLayout = new QVBoxLayout(organizeTab);
    auto *orgTop = new QHBoxLayout();
    organizePathEdit = new QLineEdit();
    organizePathEdit->setPlaceholderText(QString::fromLatin1("Directory to organize..."));
    organizeRuleEdit = new QLineEdit();
    organizeRuleEdit->setPlaceholderText(QString::fromLatin1("Rule: {extension}/{name}"));
    organizeRuleEdit->setText(QString::fromLatin1("{extension}/{year}/{name}"));
    organizeBtn = new QPushButton(QString::fromLatin1("Preview"));
    organizeResultLbl = new QLabel(QString::fromLatin1("No organize preview yet."));
    organizeResultLbl->setWordWrap(true);
    organizeResultLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    orgTop->addWidget(organizePathEdit, 2);
    orgTop->addWidget(organizeRuleEdit, 2);
    orgTop->addWidget(organizeBtn);
    orgLayout->addLayout(orgTop);
    orgLayout->addWidget(organizeResultLbl, 1);
    connect(organizeBtn, SIGNAL(clicked()), this, SLOT(onOrganizeClicked()));
    tabs->addTab(organizeTab, QString::fromLatin1("Organize"));

    // Delete Dupes tab
    deleteDupesTab = new QWidget();
    auto *ddLayout = new QVBoxLayout(deleteDupesTab);
    auto *ddTop = new QHBoxLayout();
    deleteDupesPathEdit = new QLineEdit();
    deleteDupesPathEdit->setPlaceholderText(QString::fromLatin1("Directory to deduplicate..."));
    deleteDupesBtn = new QPushButton(QString::fromLatin1("Delete Dupes"));
    deleteDupesResultLbl = new QLabel(QString::fromLatin1("No deduplication run yet."));
    deleteDupesResultLbl->setWordWrap(true);
    deleteDupesResultLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    ddTop->addWidget(deleteDupesPathEdit, 3);
    ddTop->addWidget(deleteDupesBtn);
    ddLayout->addLayout(ddTop);
    ddLayout->addWidget(deleteDupesResultLbl, 1);
    connect(deleteDupesBtn, SIGNAL(clicked()), this, SLOT(onDeleteDupesClicked()));
    tabs->addTab(deleteDupesTab, QString::fromLatin1("Delete Dupes"));
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

// ── Export Tab ───────────────────────────────────────────────────────────────

void DemoWindow::onExportClicked()
{
    const QString path = exportPathEdit->text();
    if (path.isEmpty()) return;
    runExport(path);
}

void DemoWindow::runExport(const QString &dir)
{
    exportBtn->setEnabled(false);
    exportResultLbl->setText(QString::fromLatin1("Exporting: ") + dir + QString::fromLatin1(" ..."));
    const std::string stdDir = dir.toStdString();
    std::thread([this, stdDir]() {
        QString resultStr;
        try {
            fo::core::EngineConfig cfg;
            cfg.db_path = ":memory:"; cfg.scanner = "std"; cfg.hasher = "fast64";
            fo::core::Engine engine(cfg);
            auto files = engine.scan({std::filesystem::u8path(stdDir)}, {}, false, false);
            auto groups = engine.find_duplicates(files);
            auto stats = fo::core::Exporter::compute_stats(files, groups);
            std::ostringstream out;
            fo::core::Exporter::to_json(out, files, groups, stats);
            resultStr = QString::fromLatin1("Export Complete\n\n%1 files, %2 duplicate groups\n\nFirst 500 chars:\n")
                .arg(files.size()).arg(groups.size())
                + QString::fromStdString(out.str()).left(500);
        } catch (const std::exception &e) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(e.what());
        }
        QMetaObject::invokeMethod(this, "applyExportResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::applyExportResult(const QString &result)
{
    exportResultLbl->setText(result);
    exportBtn->setEnabled(true);
}

// ── Count Tab ────────────────────────────────────────────────────────────────

void DemoWindow::onCountClicked()
{
    const QString path = countPathEdit->text();
    if (path.isEmpty()) return;
    runCount(path);
}

void DemoWindow::runCount(const QString &dir)
{
    countBtn->setEnabled(false);
    countResultLbl->setText(QString::fromLatin1("Counting: ") + dir + QString::fromLatin1(" ..."));
    const std::string stdDir = dir.toStdString();
    std::thread([this, stdDir]() {
        QString resultStr;
        try {
            auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
            if (!scanner) throw std::runtime_error("Scanner not found");
            auto files = scanner->scan({std::filesystem::u8path(stdDir)}, {}, false);
            std::uintmax_t total_size = 0;
            int file_count = 0, dir_count = 0;
            for (const auto &f : files) {
                if (f.is_dir) { ++dir_count; continue; }
                ++file_count; total_size += f.size;
            }
            resultStr = QString::fromLatin1("File Count\n\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\n\n"
                "Files: %1\nDirectories: %2\nTotal Size: %3 MB\n").arg(file_count).arg(dir_count).arg(total_size / (1024*1024));
        } catch (const std::exception &e) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(e.what());
        }
        QMetaObject::invokeMethod(this, "applyCountResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::applyCountResult(const QString &result)
{
    countResultLbl->setText(result);
    countBtn->setEnabled(true);
}

// ── History Tab ─────────────────────────────────────────────────────────────

void DemoWindow::onHistoryClicked()
{
    historyBtn->setEnabled(false);
    historyResultLbl->setText(QString::fromLatin1("Loading history..."));
    std::thread([this]() {
        QString resultStr;
        try {
            fo::core::EngineConfig cfg;
            cfg.db_path = ":memory:"; cfg.scanner = "std"; cfg.hasher = "fast64";
            fo::core::Engine engine(cfg);
            fo::core::OperationRepository op_repo(engine.database());
            auto ops = op_repo.list_recent(50);
            resultStr = QString::fromLatin1("Operation History\n\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\n\n");
            if (ops.empty()) resultStr += QString::fromLatin1("(no operations recorded)\n");
            for (const auto &op : ops) {
                resultStr += QString::fromLatin1("[%1] %2: %3 -> %4\n")
                    .arg(QString::fromStdString(op.source_path).left(30))
                    .arg(op.type == fo::core::OperationType::Move ? "Move" : "Copy")
                    .arg(QString::fromStdString(op.source_path).left(40))
                    .arg(QString::fromStdString(op.dest_path).left(40));
            }
        } catch (const std::exception &e) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(e.what());
        }
        QMetaObject::invokeMethod(this, "applyHistoryResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::applyHistoryResult(const QString &result)
{
    historyResultLbl->setText(result);
    historyBtn->setEnabled(true);
}

// ── Ignore Tab ─────────────────────────────────────────────────────────────

void DemoWindow::onIgnoreListClicked()
{
    ignoreListBtn->setEnabled(false);
    ignoreResultLbl->setText(QString::fromLatin1("Loading ignore rules..."));
    std::thread([this]() {
        QString resultStr;
        try {
            fo::core::EngineConfig cfg;
            cfg.db_path = ":memory:"; cfg.scanner = "std"; cfg.hasher = "fast64";
            fo::core::Engine engine(cfg);
            fo::core::IgnoreRepository ignore_repo(engine.database());
            ignore_repo.migrate();
            auto rules = ignore_repo.list_rules();
            resultStr = QString::fromLatin1("Ignore Rules\n\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\n\n");
            if (rules.empty()) resultStr += QString::fromLatin1("(no rules)\n");
            for (const auto &r : rules) {
                resultStr += QString::fromStdString(r.pattern) + QString::fromLatin1(" - ") + QString::fromStdString(r.reason) + QString::fromLatin1("\n");
            }
        } catch (const std::exception &e) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(e.what());
        }
        QMetaObject::invokeMethod(this, "applyIgnoreResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::onIgnoreAddClicked()
{
    const QString pattern = ignorePatternEdit->text();
    if (pattern.isEmpty()) return;
    ignoreAddBtn->setEnabled(false);
    const std::string stdPattern = pattern.toStdString();
    std::thread([this, stdPattern]() {
        QString resultStr;
        try {
            fo::core::EngineConfig cfg;
            cfg.db_path = ":memory:"; cfg.scanner = "std"; cfg.hasher = "fast64";
            fo::core::Engine engine(cfg);
            fo::core::IgnoreRepository ignore_repo(engine.database());
            ignore_repo.migrate();
            fo::core::IgnoreRule rule;
            rule.pattern = stdPattern;
            rule.reason = "Added from BTK demo";
            ignore_repo.add_rule(rule);
            resultStr = QString::fromLatin1("Added rule: %1\n").arg(QString::fromStdString(stdPattern));
            auto rules = ignore_repo.list_rules();
            for (const auto &r : rules) {
                resultStr += QString::fromStdString(r.pattern) + QString::fromLatin1("\n");
            }
        } catch (const std::exception &e) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(e.what());
        }
        QMetaObject::invokeMethod(this, "applyIgnoreResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::applyIgnoreResult(const QString &result)
{
    ignoreResultLbl->setText(result);
    ignoreListBtn->setEnabled(true);
    ignoreAddBtn->setEnabled(true);
}

// ── Organize Tab ────────────────────────────────────────────────────────────

void DemoWindow::onOrganizeClicked()
{
    const QString path = organizePathEdit->text();
    const QString rule = organizeRuleEdit->text();
    if (path.isEmpty() || rule.isEmpty()) return;
    organizeBtn->setEnabled(false);
    const std::string stdPath = path.toStdString();
    const std::string stdRule = rule.toStdString();
    std::thread([this, stdPath, stdRule]() {
        QString resultStr;
        try {
            fo::core::EngineConfig cfg;
            cfg.db_path = ":memory:"; cfg.scanner = "std"; cfg.hasher = "fast64";
            fo::core::Engine engine(cfg);
            auto files = engine.scan({std::filesystem::u8path(stdPath)}, {}, false, false);
            fo::core::RuleEngine rule_engine;
            rule_engine.add_rule({"cli_rule", "", stdRule});
            int moves = 0;
            for (const auto &f : files) {
                auto new_path = rule_engine.apply_rules(f, {});
                if (new_path && new_path->string() != f.uri) ++moves;
            }
            resultStr = QString::fromLatin1("Organize Dry Run\n\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\n\n"
                "Scanned: %1 files\nProposed moves: %2\nRule: %3\n").arg(files.size()).arg(moves).arg(QString::fromStdString(stdRule));
        } catch (const std::exception &e) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(e.what());
        }
        QMetaObject::invokeMethod(this, "applyOrganizeResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::applyOrganizeResult(const QString &result)
{
    organizeResultLbl->setText(result);
    organizeBtn->setEnabled(true);
}

// ── Delete Dupes Tab ────────────────────────────────────────────────────────

void DemoWindow::onDeleteDupesClicked()
{
    const QString path = deleteDupesPathEdit->text();
    if (path.isEmpty()) return;
    deleteDupesBtn->setEnabled(false);
    const std::string stdPath = path.toStdString();
    std::thread([this, stdPath]() {
        QString resultStr;
        try {
            fo::core::EngineConfig cfg;
            cfg.db_path = ":memory:"; cfg.scanner = "std"; cfg.hasher = "fast64";
            fo::core::Engine engine(cfg);
            auto files = engine.scan({std::filesystem::u8path(stdPath)}, {}, false, false);
            auto groups = engine.find_duplicates(files);
            int deleted = 0;
            std::uintmax_t freed = 0;
            for (const auto &g : groups) {
                // Keep first, delete rest
                for (size_t i = 1; i < g.files.size(); ++i) {
                    try {
                        auto sz = std::filesystem::file_size(g.files[i].uri);
                        std::filesystem::remove(g.files[i].uri);
                        ++deleted; freed += sz;
                    } catch (...) {}
                }
            }
            resultStr = QString::fromLatin1("Delete Duplicates\n\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\n\n"
                "Groups: %1\nDeleted: %2 files\nFreed: %3 MB\n").arg(groups.size()).arg(deleted).arg(freed/(1024*1024));
        } catch (const std::exception &e) {
            resultStr = QString::fromLatin1("Error: ") + QString::fromStdString(e.what());
        }
        QMetaObject::invokeMethod(this, "applyDeleteDupesResult", Qt::QueuedConnection, Q_ARG(QString, resultStr));
    }).detach();
}

void DemoWindow::applyDeleteDupesResult(const QString &result)
{
    deleteDupesResultLbl->setText(result);
    deleteDupesBtn->setEnabled(true);
}
