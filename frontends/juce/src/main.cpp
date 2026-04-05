#include <JuceHeader.h>

#include <fo/core/version.hpp>
#include <fo/core/interfaces.hpp>
#include <fo/core/registry.hpp>

#include <filesystem>
#include <vector>
#include <string>

namespace {

class MainComponent final : public juce::Component
{
public:
    MainComponent()
    {
        title.setText("bobfilez JUCE Demo", juce::dontSendNotification);
        title.setJustificationType(juce::Justification::centred);
        title.setFont(juce::Font(28.0f, juce::Font::bold));
        addAndMakeVisible(title);

        body.setText(
            "This minimal JUCE frontend proves bobfilez can host a native JUCE shell alongside the Qt, BobUI, BTK, BobGUI, and web lanes. Click 'Scan Directory' below to invoke the native C++ fo_core engine.",
            juce::dontSendNotification);
        body.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(body);

        version.setText("Core version: " + juce::String(fo::core::FO_VERSION.data()), juce::dontSendNotification);
        version.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(version);

        scanButton.setButtonText("Scan Directory...");
        addAndMakeVisible(scanButton);

        resultsLabel.setText("No directory scanned yet.", juce::dontSendNotification);
        resultsLabel.setJustificationType(juce::Justification::centred);
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

        setSize(900, 600);
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
        
        area.removeFromTop(40); // spacer
        
        auto buttonArea = area.removeFromTop(40);
        scanButton.setBounds(buttonArea.withSizeKeepingCentre(200, 40));

        area.removeFromTop(20); // spacer
        resultsLabel.setBounds(area.removeFromTop(80));
    }

private:
    void runScan(const std::string& directoryPath)
    {
        resultsLabel.setText("Scanning: " + juce::String(directoryPath) + " ...", juce::dontSendNotification);
        
        // Push the scan to a background thread to avoid blocking the JUCE message thread
        juce::Thread::launch([this, directoryPath]() {
            auto scanner = fo::core::Registry<fo::core::IFileScanner>::instance().create("std");
            if (!scanner) {
                juce::MessageManager::callAsync([this]() {
                    resultsLabel.setText("Error: 'std' scanner provider not found in fo_core registry.", juce::dontSendNotification);
                });
                return;
            }

            std::vector<std::filesystem::path> roots = { std::filesystem::u8path(directoryPath) };
            std::vector<std::string> exts; // all extensions
            bool follow_symlinks = false;

            auto files = scanner->scan(roots, exts, follow_symlinks);
            auto count = files.size();

            // Calculate total size
            std::uintmax_t total_bytes = 0;
            for (const auto& f : files) {
                total_bytes += f.size;
            }

            juce::String resultText = "Scan Complete!\nFound " + juce::String(count) + " files.\n" +
                                      "Total size: " + juce::String(total_bytes / (1024 * 1024)) + " MB.";

            juce::MessageManager::callAsync([this, resultText]() {
                resultsLabel.setText(resultText, juce::dontSendNotification);
            });
        });
    }

    juce::Label title;
    juce::Label body;
    juce::Label version;
    juce::TextButton scanButton;
    juce::Label resultsLabel;
    
    std::unique_ptr<juce::FileChooser> fileChooser;
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
