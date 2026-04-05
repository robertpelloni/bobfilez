#include <JuceHeader.h>

#include <fo/core/version.hpp>

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
            "This minimal JUCE frontend proves bobfilez can host a native JUCE shell alongside the Qt, BobUI, BTK, BobGUI, and web lanes.",
            juce::dontSendNotification);
        body.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(body);

        version.setText("Core version: " + juce::String(fo::core::FO_VERSION.data()), juce::dontSendNotification);
        version.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(version);

        setSize(900, 600);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff10151d));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(40);
        title.setBounds(area.removeFromTop(80));
        body.setBounds(area.removeFromTop(160));
        version.setBounds(area.removeFromTop(40));
    }

private:
    juce::Label title;
    juce::Label body;
    juce::Label version;
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
