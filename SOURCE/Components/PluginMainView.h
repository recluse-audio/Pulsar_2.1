#pragma once

#include <JuceHeader.h>
#include "../Processor/PluginProcessor.h"
#include "../LookNFeel/NameFeel.h"
#include "PulsaretVisualizer.h"
#include "PulsarControlsComponent.h"
#include "PulsarSlider.h"

class PluginMainView : public juce::Component
{
public:
    static constexpr int kDesignWidth  = 600;
    static constexpr int kDesignHeight = 450;

    PluginMainView(PulsarAudioProcessor& p);
    ~PluginMainView() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void repaintPulsaret(bool flashing, float flashCoef);

    juce::TextButton& getPresetButton()     { return presetButton; }
    juce::TextButton& getSaveButton()       { return savePresetButton; }

private:
    void initLabels();
    void buildSliders();

    PulsarAudioProcessor& audioProcessor;

    juce::Rectangle<int> frame;

    PulsaretVisualizer pulsaretVisualizer;

    NameFeel nameFeel;
    juce::Label nameLabel;
    juce::Label versionLabel;

    juce::TextButton presetButton     { "PRESETS" };
    juce::TextButton savePresetButton { "SAVE AS PRESET" };

    std::unique_ptr<PulsarControlsComponent> controlComponent;

    std::unique_ptr<PulsarSlider> fundamentalSlider;
    std::unique_ptr<PulsarSlider> formantSlider;
    std::unique_ptr<PulsarSlider> formantSlider2;
    std::unique_ptr<PulsarSlider> waveSlider;
    std::unique_ptr<PulsarSlider> ampSlider;
    std::unique_ptr<PulsarSlider> panSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginMainView)
};
