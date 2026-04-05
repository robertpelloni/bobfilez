#include "DemoWindow.hpp"

#include <fo/core/engine.hpp>
#include <fo/core/interfaces.hpp>
#include <fo/core/registry.hpp>

#include <QMetaObject>
#include <QString>

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <thread>

DemoWindow::DemoWindow(QWidget *parent) : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);

    tabs = new QTabWidget(this);
    mainLayout->addWidget(tabs);

    scannerTab = new QWidget();
    auto *scanLayout = new QVBoxLayout(scannerTab);
    auto *scanTop = new QHBoxLayout();

    scanPathEdit = new QLineEdit();
    scanPathEdit->setPlaceholderText(QString::fromLatin1("Enter absolute directory path to scan..."));
    scanBtn = new QPushButton(QString::fromLatin1("Scan Directory"));
    scanResultLbl = new QLabel(QString::fromLatin1("No scan run yet."));
    scanResultLbl->setWordWrap(true);
    scanResultLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    scanTop->addWidget(scanPathEdit);
    scanTop->addWidget(scanBtn);
    scanLayout->addLayout(scanTop);
    scanLayout->addWidget(scanResultLbl, 1);
    connect(scanBtn, SIGNAL(clicked()), this, SLOT(onScanClicked()));

    dupesTab = new QWidget();
    auto *dupesLayout = new QVBoxLayout(dupesTab);
    auto *dupesTop = new QHBoxLayout();

    dupesPathEdit = new QLineEdit();
    dupesPathEdit->setPlaceholderText(QString::fromLatin1("Enter absolute directory path to find duplicates..."));
    dupesBtn = new QPushButton(QString::fromLatin1("Find Duplicates"));
    dupesResultLbl = new QLabel(QString::fromLatin1("No search run yet."));
    dupesResultLbl->setWordWrap(true);
    dupesResultLbl->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    dupesTop->addWidget(dupesPathEdit);
    dupesTop->addWidget(dupesBtn);
    dupesLayout->addLayout(dupesTop);
    dupesLayout->addWidget(dupesResultLbl, 1);
    connect(dupesBtn, SIGNAL(clicked()), this, SLOT(onDuplicatesClicked()));

    tabs->addTab(scannerTab, QString::fromLatin1("Scanner"));
    tabs->addTab(dupesTab, QString::fromLatin1("Duplicates"));
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
