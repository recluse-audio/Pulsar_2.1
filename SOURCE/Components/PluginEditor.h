/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginMainView.h"
#include "PresetManagerView.h"
#include "CreatePresetView.h"
#include "../Processor/PluginProcessor.h"
#include "../LookNFeel/MixFeel.h"
#include "../LookNFeel/PulsarUIFeel.h"

class PulsarAudioProcessorEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
    PulsarAudioProcessorEditor(PulsarAudioProcessor&);
    ~PulsarAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    PulsarUIFeel pulsarFeel;
    MixFeel mixFeel;

    std::unique_ptr<PluginMainView>     mainView;
    std::unique_ptr<PresetManagerView>  presetManagerView;
    std::unique_ptr<CreatePresetView>   createPresetView;

    PulsarAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PulsarAudioProcessorEditor)
};
