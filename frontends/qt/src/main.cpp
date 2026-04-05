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
#include <fo/core/registry.hpp>
#include <fo/core/version.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
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

class DashboardTab final : public QWidget {
public:
    explicit DashboardTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        auto *layout = new QVBoxLayout(this);
        auto *title = new QLabel(QStringLiteral("bobfilez Qt GUI Demo"), this);
        auto *body = new QLabel(
            QStringLiteral("This Qt Widgets frontend now exercises the same core scan, duplicate, statistics, and hashing workflows used by the JUCE and React lanes."),
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

    window.setCentralWidget(tabs);
    window.show();

    return app.exec();
}
