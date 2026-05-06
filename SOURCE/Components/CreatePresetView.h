#pragma once

#include <JuceHeader.h>
#include "../Processor/PluginProcessor.h"
#include "../Util/PresetPaths.h"
#include "PresetManagerView.h" // SmallButtonLookAndFeel

class CreatePresetView : public juce::Component
{
public:
    std::function<void()> onSaved;

    CreatePresetView(PulsarAudioProcessor& p) : audioProcessor(p)
    {
        setInterceptsMouseClicks(true, true);
        setLookAndFeel(&smallLF);

        title.setText("Save As Preset", juce::dontSendNotification);
        title.setJustificationType(juce::Justification::centred);
        title.setFont(juce::Font(juce::FontOptions("Consolas", "Regular", 18.f)));
        title.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(title);

        nameEditor.setMultiLine(false);
        nameEditor.setReturnKeyStartsNewLine(false);
        nameEditor.setTextToShowWhenEmpty("preset name", juce::Colours::grey);
        nameEditor.setFont(juce::Font(juce::FontOptions("Consolas", "Regular", 16.f)));
        nameEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
        nameEditor.setColour(juce::TextEditor::textColourId,       juce::Colours::white);
        nameEditor.setColour(juce::TextEditor::outlineColourId,    juce::Colours::ghostwhite.withAlpha(0.5f));
        nameEditor.onReturnKey = [this] { commitSave(); };
        addAndMakeVisible(nameEditor);

        for (auto* b : { &saveButton, &cancelButton })
        {
            b->setColour(juce::TextButton::buttonColourId,  juce::Colours::black);
            b->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            addAndMakeVisible(*b);
        }
        saveButton.onClick   = [this] { commitSave(); };
        cancelButton.onClick = [this] { close(); };

        status.setJustificationType(juce::Justification::centred);
        status.setFont(juce::Font(juce::FontOptions("Consolas", "Regular", 11.f)));
        status.setColour(juce::Label::textColourId, juce::Colours::orange);
        addAndMakeVisible(status);
    }

    ~CreatePresetView() override
    {
        setLookAndFeel(nullptr);
    }

    void visibilityChanged() override
    {
        if (isVisible())
        {
            nameEditor.setText({}, juce::dontSendNotification);
            status.setText({}, juce::dontSendNotification);
            nameEditor.grabKeyboardFocus();
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black.withAlpha(0.92f));

        g.setColour(juce::Colours::yellow);
        g.drawRoundedRectangle(getLocalBounds().reduced(4).toFloat(), 5.f, 2.f);

        const auto panel = panelBounds().toFloat();
        g.setColour(juce::Colours::ghostwhite.withAlpha(0.4f));
        g.drawRoundedRectangle(panel, 6.f, 1.f);
    }

    void resized() override
    {
        auto panel = panelBounds();

        title.setBounds(panel.removeFromTop(30));
        panel.removeFromTop(10);

        nameEditor.setBounds(panel.removeFromTop(32).reduced(20, 0));
        panel.removeFromTop(8);
        status.setBounds(panel.removeFromTop(18).reduced(20, 0));

        panel.removeFromTop(12);
        auto buttons = panel.removeFromTop(28).reduced(20, 0);
        const int half = buttons.getWidth() / 2;
        cancelButton.setBounds(buttons.removeFromLeft(half).reduced(4, 0));
        saveButton  .setBounds(buttons.reduced(4, 0));
    }

private:
    juce::Rectangle<int> panelBounds() const
    {
        auto r = getLocalBounds();
        return r.withSizeKeepingCentre(juce::jmin(360, r.getWidth()  - 40),
                                       juce::jmin(180, r.getHeight() - 40));
    }

    void close()
    {
        setVisible(false);
    }

    void commitSave()
    {
        const auto name = nameEditor.getText().trim();
        if (name.isEmpty())
        {
            status.setText("name required", juce::dontSendNotification);
            return;
        }

        const auto safe = juce::File::createLegalFileName(name);
        const auto file = Pulsar::userPresetDir().getChildFile(safe + ".xml");
        audioProcessor.savePreset(file.getFullPathName());

        if (onSaved != nullptr)
            onSaved();

        close();
    }

    PulsarAudioProcessor& audioProcessor;
    SmallButtonLookAndFeel smallLF;

    juce::Label       title;
    juce::TextEditor  nameEditor;
    juce::Label       status;
    juce::TextButton  saveButton   { "Save" };
    juce::TextButton  cancelButton { "Cancel" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CreatePresetView)
};
