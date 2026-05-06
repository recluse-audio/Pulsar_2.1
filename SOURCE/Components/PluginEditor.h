/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PulsaretVisualizer.h"
#include "PresetManagerView.h"
#include "CreatePresetView.h"
#include "../Processor/PluginProcessor.h"
#include "../LookNFeel/NameFeel.h"
#include "../LookNFeel/MixFeel.h"
#include "../LookNFeel/PulsarUIFeel.h"
#include "PulsarControlsComponent.h"
#include "PulsarSlider.h"

//==============================================================================
class PulsarAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Timer
{
public:
    PulsarAudioProcessorEditor (PulsarAudioProcessor&);
    ~PulsarAudioProcessorEditor() override;
    void init();
    void resetSliders();

    void paint (juce::Graphics&) override;
    void resized() override;

    void repaintPulsaret();
    void timerCallback() override;

    void setTestColor(juce::Colour c){ color = c; }

private:
    PulsarUIFeel pulsarFeel;
    MixFeel mixFeel;

    std::unique_ptr<PulsarSlider> fundamentalSlider;
    std::unique_ptr<PulsarSlider> formantSlider;
    std::unique_ptr<PulsarSlider> formantSlider2;
    std::unique_ptr<PulsarSlider> waveSlider;
    std::unique_ptr<PulsarSlider> ampSlider;
    std::unique_ptr<PulsarSlider> panSlider;

    std::unique_ptr<PulsarControlsComponent> controlComponent;

    std::unique_ptr<juce::TextButton>   presetButton;
    std::unique_ptr<juce::TextButton>   savePresetButton;
    std::unique_ptr<PresetManagerView>  presetManagerView;
    std::unique_ptr<CreatePresetView>   createPresetView;

    PulsaretVisualizer pulsaretVisualizer;

    juce::Rectangle<int> frame;
    juce::Component frameComponent;

    NameFeel nameFeel;
    std::unique_ptr<juce::Label> nameLabel;
    std::unique_ptr<juce::Label> versionLabel;

    PulsarAudioProcessor& audioProcessor;

    juce::Colour color{ 0, 0, 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PulsarAudioProcessorEditor)
};
