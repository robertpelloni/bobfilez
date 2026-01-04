#include "mainwindow.h"
#include "fo/core/provider_registration.hpp"
#include "fo/core/export.hpp"
#include "fo/core/version.hpp"
#include <QWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QTime>
#include <QDir>
#include <chrono>
#include <ctime>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    fo::core::register_all_providers();

    setWindowTitle(QString("filez v%1").arg(fo::core::FO_VERSION));
    resize(1024, 768);

    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Top bar: Path input and Browse button
    QHBoxLayout *topLayout = new QHBoxLayout();
    pathInput = new QLineEdit(this);
    pathInput->setPlaceholderText("Select directory to scan...");
    browseBtn = new QPushButton("Browse", this);
    connect(browseBtn, &QPushButton::clicked, this, &MainWindow::selectDirectory);

    topLayout->addWidget(new QLabel("Path:", this));
    topLayout->addWidget(pathInput, 1);
    topLayout->addWidget(browseBtn);
    mainLayout->addLayout(topLayout);

    // Options bar
    QHBoxLayout *optionsLayout = new QHBoxLayout();
    optionsLayout->addWidget(new QLabel("Scanner:", this));
    scannerCombo = new QComboBox(this);
    scannerCombo->addItems({"std", "win32", "dirent"});
    optionsLayout->addWidget(scannerCombo);

    optionsLayout->addWidget(new QLabel("Hasher:", this));
    hasherCombo = new QComboBox(this);
    hasherCombo->addItems({"fast64", "sha256", "xxhash"});
    optionsLayout->addWidget(hasherCombo);

    followSymlinks = new QCheckBox("Follow Symlinks", this);
    optionsLayout->addWidget(followSymlinks);
    optionsLayout->addStretch();
    mainLayout->addLayout(optionsLayout);

    // Action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    scanBtn = new QPushButton("Scan", this);
    scanBtn->setIcon(QIcon::fromTheme("folder-open"));
    connect(scanBtn, &QPushButton::clicked, this, &MainWindow::startScan);
    buttonLayout->addWidget(scanBtn);

    dupBtn = new QPushButton("Find Duplicates", this);
    dupBtn->setEnabled(false);
    connect(dupBtn, &QPushButton::clicked, this, &MainWindow::findDuplicates);
    buttonLayout->addWidget(dupBtn);

    exportBtn = new QPushButton("Export", this);
    exportBtn->setEnabled(false);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::exportResults);
    buttonLayout->addWidget(exportBtn);

    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // Progress bar
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    mainLayout->addWidget(progressBar);

    // Tab widget for results
    tabWidget = new QTabWidget(this);

    // Files tab
    fileTable = new QTableWidget(this);
    fileTable->setColumnCount(3);
    fileTable->setHorizontalHeaderLabels({"Path", "Size", "Modified"});
    fileTable->horizontalHeader()->setStretchLastSection(true);
    fileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabWidget->addTab(fileTable, "Files");

    // Duplicates tab
    dupTable = new QTableWidget(this);
    dupTable->setColumnCount(4);
    dupTable->setHorizontalHeaderLabels({"Group", "Size", "Hash", "Path"});
    dupTable->horizontalHeader()->setStretchLastSection(true);
    dupTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabWidget->addTab(dupTable, "Duplicates");

    // Log tab
    logArea = new QTextEdit(this);
    logArea->setReadOnly(true);
    tabWidget->addTab(logArea, "Log");

    mainLayout->addWidget(tabWidget, 1);

    // Initialize engine
    fo::core::EngineConfig cfg;
    cfg.db_path = "fo_gui.db";
    engine = std::make_unique<fo::core::Engine>(cfg);

    log("filez GUI initialized.");
}

MainWindow::~MainWindow() {
}

void MainWindow::setupMenuBar() {
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Open Directory...", this, &MainWindow::selectDirectory, QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", qApp, &QApplication::quit, QKeySequence::Quit);

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About", [this]() {
        QMessageBox::about(this, "About filez",
            QString("filez v%1\n\nA powerful file organization tool.").arg(fo::core::FO_VERSION));
    });
}

void MainWindow::setupToolBar() {
    QToolBar *toolbar = addToolBar("Main");
    toolbar->addAction("Scan", this, &MainWindow::startScan);
    toolbar->addAction("Duplicates", this, &MainWindow::findDuplicates);
    toolbar->addAction("Export", this, &MainWindow::exportResults);
}

void MainWindow::setupStatusBar() {
    statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(statusLabel);
}

void MainWindow::log(const QString& message) {
    logArea->append(QString("[%1] %2").arg(QTime::currentTime().toString("HH:mm:ss"), message));
}

void MainWindow::selectDirectory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory",
                                                    QDir::homePath(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        pathInput->setText(dir);
    }
}


void MainWindow::startScan() {
    QString dir = pathInput->text();
    if (dir.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a directory first.");
        return;
    }

    log("Starting scan: " + dir);
    statusLabel->setText("Scanning...");
    scanBtn->setEnabled(false);
    progressBar->setVisible(true);
    progressBar->setRange(0, 0); // Indeterminate

    QApplication::processEvents();

    try {
        std::vector<std::filesystem::path> roots = { dir.toStdString() };
        scannedFiles = engine->scan(roots, {}, followSymlinks->isChecked(), false);

        // Populate file table
        fileTable->setRowCount(static_cast<int>(scannedFiles.size()));
        for (size_t i = 0; i < scannedFiles.size(); ++i) {
            const auto& f = scannedFiles[i];
            fileTable->setItem(static_cast<int>(i), 0, new QTableWidgetItem(QString::fromStdString(f.path.string())));
            fileTable->setItem(static_cast<int>(i), 1, new QTableWidgetItem(QString::fromStdString(fo::core::Exporter::format_size(f.size))));

            auto sys_tp = std::chrono::clock_cast<std::chrono::system_clock>(f.mtime);
            auto t = std::chrono::system_clock::to_time_t(sys_tp);
            fileTable->setItem(static_cast<int>(i), 2, new QTableWidgetItem(QString::fromStdString(std::ctime(&t)).trimmed()));
        }

        log(QString("Scan complete. Found %1 files.").arg(scannedFiles.size()));
        statusLabel->setText(QString("Found %1 files").arg(scannedFiles.size()));
        dupBtn->setEnabled(true);
        exportBtn->setEnabled(true);
        tabWidget->setCurrentIndex(0); // Switch to Files tab

    } catch (const std::exception& e) {
        log(QString("Error: %1").arg(e.what()));
        QMessageBox::critical(this, "Error", QString("Scan failed: %1").arg(e.what()));
    }

    scanBtn->setEnabled(true);
    progressBar->setVisible(false);
}

void MainWindow::findDuplicates() {
    if (scannedFiles.empty()) {
        QMessageBox::warning(this, "Warning", "Please scan a directory first.");
        return;
    }

    log("Finding duplicates...");
    statusLabel->setText("Finding duplicates...");
    progressBar->setVisible(true);
    progressBar->setRange(0, 0);

    QApplication::processEvents();

    try {
        duplicateGroups = engine->find_duplicates(scannedFiles);

        // Count total duplicate files
        int totalDups = 0;
        qint64 wastedSpace = 0;
        for (const auto& g : duplicateGroups) {
            totalDups += static_cast<int>(g.files.size());
            wastedSpace += g.size * (g.files.size() - 1);
        }

        // Populate duplicates table
        dupTable->setRowCount(totalDups);
        int row = 0;
        int groupNum = 1;
        for (const auto& g : duplicateGroups) {
            for (const auto& f : g.files) {
                dupTable->setItem(row, 0, new QTableWidgetItem(QString::number(groupNum)));
                dupTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(fo::core::Exporter::format_size(g.size))));
                dupTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(g.fast64.substr(0, 16)) + "..."));
                dupTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(f.path.string())));
                ++row;
            }
            ++groupNum;
        }

        log(QString("Found %1 duplicate groups (%2 wasted space).").arg(duplicateGroups.size()).arg(QString::fromStdString(fo::core::Exporter::format_size(wastedSpace))));
        statusLabel->setText(QString("%1 duplicate groups").arg(duplicateGroups.size()));
        tabWidget->setCurrentIndex(1); // Switch to Duplicates tab

    } catch (const std::exception& e) {
        log(QString("Error: %1").arg(e.what()));
        QMessageBox::critical(this, "Error", QString("Duplicate detection failed: %1").arg(e.what()));
    }

    progressBar->setVisible(false);
}

void MainWindow::exportResults() {
    if (scannedFiles.empty()) {
        QMessageBox::warning(this, "Warning", "No scan results to export.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Export Results",
        QDir::homePath() + "/scan_report.html",
        "HTML Files (*.html);;JSON Files (*.json);;CSV Files (*.csv)");

    if (fileName.isEmpty()) return;

    try {
        auto stats = fo::core::Exporter::compute_stats(scannedFiles, duplicateGroups);

        fo::core::ExportFormat format = fo::core::ExportFormat::HTML;
        if (fileName.endsWith(".json")) format = fo::core::ExportFormat::JSON;
        else if (fileName.endsWith(".csv")) format = fo::core::ExportFormat::CSV;

        if (fo::core::Exporter::export_to_file(fileName.toStdString(), scannedFiles, duplicateGroups, stats, format)) {
            log("Exported to: " + fileName);
            QMessageBox::information(this, "Export Complete", "Results exported to:\n" + fileName);
        } else {
            throw std::runtime_error("Export failed");
        }
    } catch (const std::exception& e) {
        log(QString("Export error: %1").arg(e.what()));
        QMessageBox::critical(this, "Error", QString("Export failed: %1").arg(e.what()));
    }
}

void MainWindow::onScanProgress(int current, int total) {
    progressBar->setRange(0, total);
    progressBar->setValue(current);
}

void MainWindow::onScanComplete(int fileCount) {
    log(QString("Scan complete. Found %1 files.").arg(fileCount));
    statusLabel->setText(QString("Found %1 files").arg(fileCount));
}

void MainWindow::onFileFound(const QString& path, qint64 size) {
    Q_UNUSED(path);
    Q_UNUSED(size);
}

void MainWindow::onDuplicatesFound(int groupCount, qint64 wastedSpace) {
    log(QString("Found %1 duplicate groups (%2 wasted).").arg(groupCount).arg(wastedSpace));
}
