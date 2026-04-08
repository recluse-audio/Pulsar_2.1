/*
  ==============================================================================

    PulsarControlsComponent.h
    Created: 23 Aug 2021 8:01:33pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Processor/PluginProcessor.h" // PulsarSlider file location needs to change file is not in the right spot
#include "../LookNFeel/PulsarUIFeel.h"

//==============================================================================
/*
*/
class PulsarControlsComponent  : public juce::Component,
    juce::Button::Listener
{
public:
    PulsarControlsComponent(PulsarAudioProcessor& p);
    ~PulsarControlsComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked(Button* b) override;
    void buttonStateChanged(Button* b) override;

    void createSliders();
    void drawLabels();
    void attachToState();

private:
    PulsarUIFeel pulsarFeel;
    std::unique_ptr<juce::TextButton> triggerButton;
    std::unique_ptr<juce::TextButton> runButton;



    std::unique_ptr<Slider>         interSlider;
    std::unique_ptr<Label>          interLabel;
    std::unique_ptr<Slider>         triggerOnSlider;
    std::unique_ptr<Slider>         triggerOffSlider;
    std::unique_ptr<Label>          onLabel;
    std::unique_ptr<Label>          offLabel;

    std::unique_ptr<Slider>         widthSlider;
    std::unique_ptr<Label>          widthLabel;

    std::unique_ptr<Slider>         attackSlider;
    std::unique_ptr<Slider>         decaySlider;
    std::unique_ptr<Slider>         sustainSlider;
    std::unique_ptr<Slider>         releaseSlider;

    std::unique_ptr<Label>          attackLabel;
    std::unique_ptr<Label>          decayLabel;
    std::unique_ptr<Label>          sustainLabel;
    std::unique_ptr<Label>          releaseLabel;


    std::unique_ptr<Slider>         glideSlider;
    std::unique_ptr<Label>          glideLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;


    std::unique_ptr<Attachment>     interAttachment;
    std::unique_ptr<Attachment>     triggerOnAttachment;
    std::unique_ptr<Attachment>     triggerOffAttachment;

    std::unique_ptr<Attachment>     widthAttachment;
    std::unique_ptr<Attachment>     attackAttachment;
    std::unique_ptr<Attachment>     decayAttachment;
    std::unique_ptr<Attachment>     sustainAttachment;
    std::unique_ptr<Attachment>     releaseAttachment;

    std::unique_ptr<Attachment>     glideAttachment;

    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment>     triggerAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment>     runAttachment;


    PulsarAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PulsarControlsComponent)
};
