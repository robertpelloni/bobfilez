#include <JuceHeader.h>

#include <fo/core/version.hpp>
#include <fo/core/interfaces.hpp>
#include <fo/core/lint_interface.hpp>
#include <fo/core/registry.hpp>

#include <fo/core/engine.hpp>

#include <algorithm>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace {

class DashboardTab final : public juce::Component
{
public:
    DashboardTab()
    {
        title.setText("bobfilez JUCE Demo", juce::dontSendNotification);
        title.setJustificationType(juce::Justification::centred);
        title.setFont(juce::Font(28.0f, juce::Font::bold));
        addAndMakeVisible(title);

        body.setText(
            "This minimal JUCE frontend proves bobfilez can host a native JUCE shell alongside the Qt, BobUI, BTK, BobGUI, and web lanes.\n\n"
            "Use the tabs above to invoke the native C++ fo_core engine functions, including metadata inspection and lint analysis.",
            juce::dontSendNotification);
        body.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(body);

        version.setText("Core version: " + juce::String(fo::core::FO_VERSION.data()), juce::dontSendNotification);
        version.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(version);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff10151d));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(40);
        title.setBounds(area.removeFromTop(60));
        body.setBounds(area.removeFromTop(80));
        version.setBounds(area.removeFromTop(40));
    }

private:
    juce::Label title;
    juce::Label body;
    juce::Label version;
};

class ScannerTab final : public juce::Component
{
public:
    ScannerTab()
    {
        scanButton.setButtonText("Select Directory to Scan...");
        addAndMakeVisible(scanButton);

        resultsLabel.setText("No directory scanned yet.", juce::dontSendNotification);
        resultsLabel.setJustificationType(juce::Justification::centredTop);
        addAndMakeVisible(resultsLabel);

        scanButton.onClick = [this]() {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Select a directory to scan",
                juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                "*"
            );

            auto folderFlags = juce::FileBrowserComponent::canSelectDirectories |
                               juce::FileBrowserComponent::openMode;

            fileChooser->launchAsync(folderFlags, [this](const juce::FileChooser& fc) {
                if (fc.getResult().exists()) {
                    auto path = fc.getResult().getFullPathName().toStdString();
                    runScan(path);
                }
            });
        };
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff10151d));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(40);
        auto buttonArea = area.removeFromTop(40);
        scanButton.setBounds(buttonArea.withSizeKeepingCentre(240, 40));
        area.removeFromTop(20);
        resultsLabel.setBounds(area);
    }

private:
    void runScan(const std::string& directoryPath)
    {
        resultsLabel.setText("Scanning: " + juce::String(directoryPath) + " ...", juce::dontSendNotification);

        juce::Thread::launch([this, directoryPath]() {
            auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
            if (!scanner) {
                juce::MessageManager::callAsync([this]() {
                    resultsLabel.setText("Error: 'std' scanner provider not found in fo_core registry.", juce::dontSendNotification);
                });
                return;
            }

            std::vector<std::filesystem::path> roots = { std::filesystem::u8path(directoryPath) };
            std::vector<std::string> exts;
            bool follow_symlinks = false;

            auto files = scanner->scan(roots, exts, follow_symlinks);
            auto count = files.size();

            std::uintmax_t total_bytes = 0;
            for (const auto& f : files) {
                total_bytes += f.size;
            }

            juce::String resultText = "Scan Complete!\nFound " + juce::String(count) + " files.\n" +
                                      "Total size: " + juce::String(total_bytes / (1024 * 1024)) + " MB.\n\n" +
                                      "First few files:\n";

            for (size_t i = 0; i < std::min<size_t>(files.size(), 10); ++i) {
                resultText << "- " << juce::String(files[i].uri) << "\n";
            }

            juce::MessageManager::callAsync([this, resultText]() {
                resultsLabel.setText(resultText, juce::dontSendNotification);
            });
        });
    }

    juce::TextButton scanButton;
    juce::Label resultsLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
};

class DuplicatesTab final : public juce::Component
{
public:
    DuplicatesTab()
    {
        dupeButton.setButtonText("Find Duplicates in Directory...");
        addAndMakeVisible(dupeButton);

        resultsLabel.setText("No duplicates search run yet.", juce::dontSendNotification);
        resultsLabel.setJustificationType(juce::Justification::centredTop);
        addAndMakeVisible(resultsLabel);

        dupeButton.onClick = [this]() {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Select a directory to find duplicates",
                juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                "*"
            );

            auto folderFlags = juce::FileBrowserComponent::canSelectDirectories |
                               juce::FileBrowserComponent::openMode;

            fileChooser->launchAsync(folderFlags, [this](const juce::FileChooser& fc) {
                if (fc.getResult().exists()) {
                    auto path = fc.getResult().getFullPathName().toStdString();
                    runDupes(path);
                }
            });
        };
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff10151d));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(40);
        auto buttonArea = area.removeFromTop(40);
        dupeButton.setBounds(buttonArea.withSizeKeepingCentre(240, 40));
        area.removeFromTop(20);
        resultsLabel.setBounds(area);
    }

private:
    void runDupes(const std::string& directoryPath)
    {
        resultsLabel.setText("Finding duplicates in: " + juce::String(directoryPath) + "\nThis may take a moment...", juce::dontSendNotification);

        juce::Thread::launch([this, directoryPath]() {
            try {
                fo::core::EngineConfig cfg;
                cfg.db_path = ":memory:"; // In-memory DB for quick demo
                cfg.scanner = "std";
                cfg.hasher = "fast64";
                fo::core::Engine engine(cfg);

                std::vector<std::filesystem::path> roots = { std::filesystem::u8path(directoryPath) };
                std::vector<std::string> exts;
                auto files = engine.scan(roots, exts, false, false);
                auto dupes = engine.find_duplicates(files);

                size_t duplicateFiles = 0;
                for (const auto& group : dupes) {
                    duplicateFiles += group.files.size();
                }

                juce::String resultText = "Search Complete!\nFound " + juce::String(dupes.size()) + " duplicate groups containing " + juce::String(duplicateFiles) + " files.\n\n";

                size_t displayCount = 0;
                for (const auto& group : dupes) {
                    if (displayCount++ >= 5) {
                        resultText << "...and more groups.\n";
                        break;
                    }
                    resultText << "Group Hash (" << juce::String(group.fast64) << "), Size: " << juce::String(group.size) << " bytes\n";
                    for (const auto& f : group.files) {
                        resultText << "  - " << juce::String(f.uri) << "\n";
                    }
                    resultText << "\n";
                }

                juce::MessageManager::callAsync([this, resultText]() {
                    resultsLabel.setText(resultText, juce::dontSendNotification);
                });
            } catch (const std::exception& e) {
                juce::String err = "Exception: " + juce::String(e.what());
                juce::MessageManager::callAsync([this, err]() {
                    resultsLabel.setText(err, juce::dontSendNotification);
                });
            }
        });
    }

    juce::TextButton dupeButton;
    juce::Label resultsLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
};

class StatsTab final : public juce::Component
{
public:
    StatsTab()
    {
        statsButton.setButtonText("Get Directory Statistics...");
        addAndMakeVisible(statsButton);

        resultsLabel.setText("No statistics gathered yet.", juce::dontSendNotification);
        resultsLabel.setJustificationType(juce::Justification::centredTop);
        addAndMakeVisible(resultsLabel);

        statsButton.onClick = [this]() {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Select a directory to analyze",
                juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                "*"
            );

            auto folderFlags = juce::FileBrowserComponent::canSelectDirectories |
                               juce::FileBrowserComponent::openMode;

            fileChooser->launchAsync(folderFlags, [this](const juce::FileChooser& fc) {
                if (fc.getResult().exists()) {
                    auto path = fc.getResult().getFullPathName().toStdString();
                    runStats(path);
                }
            });
        };
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff10151d));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(40);
        auto buttonArea = area.removeFromTop(40);
        statsButton.setBounds(buttonArea.withSizeKeepingCentre(240, 40));
        area.removeFromTop(20);
        resultsLabel.setBounds(area);
    }

private:
    void runStats(const std::string& directoryPath)
    {
        resultsLabel.setText("Analyzing: " + juce::String(directoryPath) + "\nThis may take a moment...", juce::dontSendNotification);

        juce::Thread::launch([this, directoryPath]() {
            try {
                auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
                if (!scanner) throw std::runtime_error("Scanner provider 'std' not found.");

                std::vector<std::filesystem::path> roots = { std::filesystem::u8path(directoryPath) };
                std::vector<std::string> exts;
                auto files = scanner->scan(roots, exts, false);

                std::uintmax_t total_size = 0;
                int dir_count = 0, file_count = 0;
                std::map<std::string, int> ext_counts;

                for (const auto& f : files) {
                    if (f.is_dir) { dir_count++; continue; }
                    file_count++;
                    total_size += f.size;

                    auto dot = f.uri.find_last_of('.');
                    auto slash = f.uri.find_last_of("/\\");
                    std::string ext = (dot != std::string::npos && (slash == std::string::npos || dot > slash))
                        ? f.uri.substr(dot) : "(none)";
                    for (auto& c : ext) c = std::tolower(c);
                    ext_counts[ext]++;
                }

                // Sort extensions by count
                std::vector<std::pair<std::string, int>> ext_list(ext_counts.begin(), ext_counts.end());
                std::sort(ext_list.begin(), ext_list.end(), [](const auto& a, const auto& b) {
                    return a.second > b.second;
                });

                juce::String resultText = "Statistics for: " + juce::String(directoryPath) + "\n\n" +
                                          "Total Files: " + juce::String(file_count) + "\n" +
                                          "Total Directories: " + juce::String(dir_count) + "\n" +
                                          "Total Size: " + juce::String(total_size / (1024 * 1024)) + " MB\n\n" +
                                          "Top Extensions:\n";

                int ext_shown = 0;
                for (const auto& [ext, count] : ext_list) {
                    if (ext_shown++ >= 10) break;
                    resultText << "  " << juce::String(ext) << ": " << juce::String(count) << " files\n";
                }

                juce::MessageManager::callAsync([this, resultText]() {
                    resultsLabel.setText(resultText, juce::dontSendNotification);
                });
            } catch (const std::exception& e) {
                juce::String err = "Exception: " + juce::String(e.what());
                juce::MessageManager::callAsync([this, err]() {
                    resultsLabel.setText(err, juce::dontSendNotification);
                });
            }
        });
    }

    juce::TextButton statsButton;
    juce::Label resultsLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
};

class HashTab final : public juce::Component
{
public:
    HashTab()
    {
        hashButton.setButtonText("Hash a File...");
        addAndMakeVisible(hashButton);

        resultsLabel.setText("No file hashed yet.", juce::dontSendNotification);
        resultsLabel.setJustificationType(juce::Justification::centredTop);
        addAndMakeVisible(resultsLabel);

        hashButton.onClick = [this]() {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Select a file to hash",
                juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                "*"
            );

            auto fileFlags = juce::FileBrowserComponent::canSelectFiles |
                             juce::FileBrowserComponent::openMode;

            fileChooser->launchAsync(fileFlags, [this](const juce::FileChooser& fc) {
                if (fc.getResult().existsAsFile()) {
                    auto path = fc.getResult().getFullPathName().toStdString();
                    runHash(path);
                }
            });
        };
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff10151d));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(40);
        auto buttonArea = area.removeFromTop(40);
        hashButton.setBounds(buttonArea.withSizeKeepingCentre(240, 40));
        area.removeFromTop(20);
        resultsLabel.setBounds(area);
    }

private:
    void runHash(const std::string& filePath)
    {
        resultsLabel.setText("Hashing: " + juce::String(filePath) + "\n...", juce::dontSendNotification);

        juce::Thread::launch([this, filePath]() {
            try {
                auto hasher = fo::core::Registry<fo::core::IHasher>::instance().create("fast64");
                if (!hasher) throw std::runtime_error("Hasher provider 'fast64' not found.");

                auto sha256 = fo::core::Registry<fo::core::IHasher>::instance().create("sha256");

                std::filesystem::path p = std::filesystem::u8path(filePath);

                juce::String resultText = "File: " + juce::String(filePath) + "\n\n";

                resultText << "Fast64 Hash: " << juce::String(hasher->fast64(p)) << "\n";
                if (sha256) {
                    resultText << "SHA-256 Hash: " << juce::String(sha256->fast64(p)) << "\n"; // actually sha256 implements fast64 returning sha256, or strong()
                }

                juce::MessageManager::callAsync([this, resultText]() {
                    resultsLabel.setText(resultText, juce::dontSendNotification);
                });
            } catch (const std::exception& e) {
                juce::String err = "Exception: " + juce::String(e.what());
                juce::MessageManager::callAsync([this, err]() {
                    resultsLabel.setText(err, juce::dontSendNotification);
                });
            }
        });
    }

    juce::TextButton hashButton;
    juce::Label resultsLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
};

class MetadataTab final : public juce::Component
{
public:
    MetadataTab()
    {
        metadataButton.setButtonText("Load Image Metadata...");
        addAndMakeVisible(metadataButton);

        resultsLabel.setText("No metadata loaded yet.", juce::dontSendNotification);
        resultsLabel.setJustificationType(juce::Justification::centredTop);
        addAndMakeVisible(resultsLabel);

        metadataButton.onClick = [this]() {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Select a directory to inspect for metadata",
                juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                "*"
            );

            auto folderFlags = juce::FileBrowserComponent::canSelectDirectories |
                               juce::FileBrowserComponent::openMode;

            fileChooser->launchAsync(folderFlags, [this](const juce::FileChooser& fc) {
                if (fc.getResult().exists()) {
                    auto path = fc.getResult().getFullPathName().toStdString();
                    runMetadata(path);
                }
            });
        };
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff10151d));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(40);
        auto buttonArea = area.removeFromTop(40);
        metadataButton.setBounds(buttonArea.withSizeKeepingCentre(240, 40));
        area.removeFromTop(20);
        resultsLabel.setBounds(area);
    }

private:
    static juce::String formatTakenTimestamp(const fo::core::ImageMetadata& metadata)
    {
        if (!metadata.date.has_taken) {
            return "n/a";
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
        return juce::String(stream.str());
    }

    void runMetadata(const std::string& directoryPath)
    {
        resultsLabel.setText("Reading metadata from: " + juce::String(directoryPath) + "\nThis may take a moment...", juce::dontSendNotification);

        juce::Thread::launch([this, directoryPath]() {
            try {
                auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
                if (!scanner) throw std::runtime_error("Scanner provider 'std' not found.");

                auto provider = fo::core::Registry<fo::core::IMetadataProvider>::instance().create("tinyexif");
                if (!provider) throw std::runtime_error("Metadata provider 'tinyexif' not found.");

                std::vector<std::filesystem::path> roots = { std::filesystem::u8path(directoryPath) };
                std::vector<std::string> exts;
                auto files = scanner->scan(roots, exts, false);

                juce::String resultText = "Metadata for: " + juce::String(directoryPath) + "\n\n";
                int recordCount = 0;

                for (const auto& file : files) {
                    if (file.is_dir) {
                        continue;
                    }

                    fo::core::ImageMetadata metadata;
                    if (!provider->read(std::filesystem::path(file.uri), metadata)) {
                        continue;
                    }

                    ++recordCount;
                    resultText << juce::String(file.uri) << "\n";
                    resultText << "  Taken: " << formatTakenTimestamp(metadata) << "\n";
                    if (metadata.has_gps) {
                        resultText << "  GPS: " << juce::String(metadata.gps_lat, 6) << ", " << juce::String(metadata.gps_lon, 6) << "\n";
                    } else {
                        resultText << "  GPS: n/a\n";
                    }
                    resultText << "\n";

                    if (recordCount >= 20) {
                        break;
                    }
                }

                if (recordCount == 0) {
                    resultText << "No readable metadata records were found.\n";
                }

                juce::MessageManager::callAsync([this, resultText]() {
                    resultsLabel.setText(resultText, juce::dontSendNotification);
                });
            } catch (const std::exception& e) {
                juce::String err = "Exception: " + juce::String(e.what());
                juce::MessageManager::callAsync([this, err]() {
                    resultsLabel.setText(err, juce::dontSendNotification);
                });
            }
        });
    }

    juce::TextButton metadataButton;
    juce::Label resultsLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
};

class LintTab final : public juce::Component
{
public:
    LintTab()
    {
        lintButton.setButtonText("Run Directory Lint...");
        addAndMakeVisible(lintButton);

        resultsLabel.setText("No lint run yet.", juce::dontSendNotification);
        resultsLabel.setJustificationType(juce::Justification::centredTop);
        addAndMakeVisible(resultsLabel);

        lintButton.onClick = [this]() {
            fileChooser = std::make_unique<juce::FileChooser>(
                "Select a directory to lint",
                juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                "*"
            );

            auto folderFlags = juce::FileBrowserComponent::canSelectDirectories |
                               juce::FileBrowserComponent::openMode;

            fileChooser->launchAsync(folderFlags, [this](const juce::FileChooser& fc) {
                if (fc.getResult().exists()) {
                    auto path = fc.getResult().getFullPathName().toStdString();
                    runLint(path);
                }
            });
        };
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff10151d));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(40);
        auto buttonArea = area.removeFromTop(40);
        lintButton.setBounds(buttonArea.withSizeKeepingCentre(240, 40));
        area.removeFromTop(20);
        resultsLabel.setBounds(area);
    }

private:
    static juce::String lintTypeName(fo::core::LintType type)
    {
        switch (type) {
            case fo::core::LintType::EmptyFile:
                return "EmptyFile";
            case fo::core::LintType::EmptyDirectory:
                return "EmptyDirectory";
            case fo::core::LintType::BrokenSymlink:
                return "BrokenSymlink";
            case fo::core::LintType::TemporaryFile:
                return "TemporaryFile";
        }

        return "Unknown";
    }

    void runLint(const std::string& directoryPath)
    {
        resultsLabel.setText("Linting: " + juce::String(directoryPath) + "\nThis may take a moment...", juce::dontSendNotification);

        juce::Thread::launch([this, directoryPath]() {
            try {
                auto linter = fo::core::Registry<fo::core::ILinter>::instance().create("std");
                if (!linter) throw std::runtime_error("Linter provider 'std' not found.");

                std::vector<std::filesystem::path> roots = { std::filesystem::u8path(directoryPath) };
                auto results = linter->lint(roots);

                std::map<juce::String, int> counts;
                for (const auto& result : results) {
                    counts[lintTypeName(result.type)]++;
                }

                juce::String resultText = "Lint for: " + juce::String(directoryPath) + "\n\nIssue counts:\n";
                for (const auto& [typeName, count] : counts) {
                    resultText << "  " << typeName << ": " << juce::String(count) << "\n";
                }

                if (results.empty()) {
                    resultText << "\nNo lint issues were found.\n";
                } else {
                    resultText << "\nFirst issues:\n";
                    size_t limit = std::min<size_t>(results.size(), 20);
                    for (size_t i = 0; i < limit; ++i) {
                        resultText << "- [" << lintTypeName(results[i].type) << "] "
                                   << juce::String(results[i].path.string()) << " — "
                                   << juce::String(results[i].details) << "\n";
                    }
                }

                juce::MessageManager::callAsync([this, resultText]() {
                    resultsLabel.setText(resultText, juce::dontSendNotification);
                });
            } catch (const std::exception& e) {
                juce::String err = "Exception: " + juce::String(e.what());
                juce::MessageManager::callAsync([this, err]() {
                    resultsLabel.setText(err, juce::dontSendNotification);
                });
            }
        });
    }

    juce::TextButton lintButton;
    juce::Label resultsLabel;
    std::unique_ptr<juce::FileChooser> fileChooser;
};

class MainComponent final : public juce::Component

{
public:
    MainComponent()
    {
        addAndMakeVisible(tabs);
        tabs.addTab("Dashboard", juce::Colours::transparentBlack, &dashboardTab, false);
        tabs.addTab("Scanner", juce::Colours::transparentBlack, &scannerTab, false);
        tabs.addTab("Duplicates", juce::Colours::transparentBlack, &duplicatesTab, false);
        tabs.addTab("Statistics", juce::Colours::transparentBlack, &statsTab, false);
        tabs.addTab("Hasher", juce::Colours::transparentBlack, &hashTab, false);
        tabs.addTab("Metadata", juce::Colours::transparentBlack, &metadataTab, false);
        tabs.addTab("Lint", juce::Colours::transparentBlack, &lintTab, false);
        tabs.setTabBarDepth(40);

        setSize(900, 600);
    }

    void resized() override
    {
        tabs.setBounds(getLocalBounds());
    }

private:
    juce::TabbedComponent tabs{ juce::TabbedButtonBar::Orientation::TabsAtTop };
    DashboardTab dashboardTab;
    ScannerTab scannerTab;
    DuplicatesTab duplicatesTab;
    StatsTab statsTab;
    HashTab hashTab;
    MetadataTab metadataTab;
    LintTab lintTab;
};


class DemoApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "bobfilez JUCE Demo"; }
    const juce::String getApplicationVersion() override { return juce::String(fo::core::FO_VERSION.data()); }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override
    {
        main_window = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        main_window.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    class MainWindow final : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(const juce::String& name)
            : juce::DocumentWindow(name,
                                   juce::Colours::black,
                                   juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
    };

    std::unique_ptr<MainWindow> main_window;
};

} // namespace

START_JUCE_APPLICATION(DemoApplication)
