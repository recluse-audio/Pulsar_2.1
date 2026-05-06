#pragma once

#include <JuceHeader.h>
#include "../Processor/PluginProcessor.h"
#include "../Util/PresetPaths.h"

class SmallButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return juce::Font(juce::FontOptions("Consolas", "Regular",
                                            juce::jmin(12.f, (float) buttonHeight * 0.5f)));
    }
};

class PresetManagerView : public juce::Component
{
public:
    PresetManagerView(PulsarAudioProcessor& p) : audioProcessor(p)
    {
        setInterceptsMouseClicks(true, true);
        setLookAndFeel(&smallLF);

        userButton.setClickingTogglesState(true);
        factoryButton.setClickingTogglesState(true);
        userButton.setRadioGroupId(1);
        factoryButton.setRadioGroupId(1);

        userButton.onClick    = [this] { setMode(Mode::User); };
        factoryButton.onClick = [this] { setMode(Mode::Factory); };
        closeButton.onClick   = [this] { setVisible(false); };

        for (auto* b : { &userButton, &factoryButton, &closeButton })
        {
            b->setColour(juce::TextButton::buttonColourId,   juce::Colours::black);
            b->setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkred);
            b->setColour(juce::TextButton::textColourOffId,  juce::Colours::white);
            b->setColour(juce::TextButton::textColourOnId,   juce::Colours::white);
            addAndMakeVisible(*b);
        }

        viewport.setViewedComponent(&presetList, false);
        viewport.setScrollBarsShown(true, false);
        addAndMakeVisible(viewport);

        factoryButton.setToggleState(true, juce::dontSendNotification);
        setMode(Mode::Factory);
    }

    ~PresetManagerView() override
    {
        setLookAndFeel(nullptr);
    }

    void refresh() { refreshList(); }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black.withAlpha(0.92f));

        const auto sidebar = getLocalBounds().removeFromLeft(getWidth() / 3);
        g.setColour(juce::Colours::ghostwhite.withAlpha(0.15f));
        g.drawVerticalLine(sidebar.getRight(), 0.f, (float) getHeight());

        g.setColour(juce::Colours::yellow);
        g.drawRoundedRectangle(getLocalBounds().reduced(4).toFloat(), 5.f, 2.f);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced(8);

        closeButton.setBounds(r.getRight() - 70, r.getY(), 70, 26);

        auto sidebar = r.removeFromLeft(r.getWidth() / 3);
        sidebar.removeFromTop(40);
        factoryButton.setBounds(sidebar.removeFromTop(36).reduced(4, 2));
        userButton   .setBounds(sidebar.removeFromTop(36).reduced(4, 2));

        r.removeFromTop(40);
        r.removeFromLeft(8);
        viewport.setBounds(r);
        relayoutPresetList();
    }

private:
    enum class Mode { Factory, User };

    void setMode(Mode m)
    {
        currentMode = m;
        refreshList();
    }

    juce::File currentDir() const
    {
        return currentMode == Mode::Factory ? Pulsar::factoryPresetDir()
                                            : Pulsar::userPresetDir();
    }

    void refreshList()
    {
        presetButtons.clear();
        currentFiles.clear();

        const auto files = currentDir().findChildFiles(juce::File::findFiles, false);
        for (const auto& f : files)
        {
            currentFiles.add(f);

            auto btn = std::make_unique<juce::TextButton>(f.getFileNameWithoutExtension());
            btn->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            btn->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            const int idx = (int) presetButtons.size();
            btn->onClick = [this, idx]
            {
                if (idx >= 0 && idx < currentFiles.size())
                    audioProcessor.loadPreset(currentFiles[idx].getFullPathName());
            };
            presetList.addAndMakeVisible(btn.get());
            presetButtons.push_back(std::move(btn));
        }

        relayoutPresetList();
    }

    void relayoutPresetList()
    {
        const int rowHeight = 28;
        const int width     = juce::jmax(0, viewport.getMaximumVisibleWidth());
        int y = 0;
        for (auto& b : presetButtons)
        {
            b->setBounds(0, y, width, rowHeight - 2);
            y += rowHeight;
        }
        presetList.setSize(width, juce::jmax(y, viewport.getHeight()));
    }

    PulsarAudioProcessor& audioProcessor;
    SmallButtonLookAndFeel smallLF;
    Mode currentMode = Mode::Factory;

    juce::TextButton userButton    { "User" };
    juce::TextButton factoryButton { "Factory" };
    juce::TextButton closeButton   { "CLOSE" };

    juce::Viewport  viewport;
    juce::Component presetList;
    std::vector<std::unique_ptr<juce::TextButton>> presetButtons;
    juce::Array<juce::File> currentFiles;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManagerView)
};
