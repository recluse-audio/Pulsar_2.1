/*
  ==============================================================================

    PulsarSlider.h
    Created: 19 Aug 2021 11:51:05am
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
enum class SliderType
{
    Standard,
    Fundamental,
    Formant,
    Horizontal
};
class PulsarSlider  : public juce::Slider,
juce::Button::Listener, juce::Slider::Listener
{
public:
    PulsarSlider(PulsarAudioProcessor& p, 
        SliderType type, const String& labelText, 
        bool hasButton, const String& buttonText, 
        bool hasIncDecSlider);
    ~PulsarSlider() override;

    void addButton(const String& buttonText);
    void addIncDecSlider();

    /* Default for slider with a value, spread, and random factor*/
    void attachToState(AudioProcessorValueTreeState& stateToUse,
        const String& sliderParamID, const String& spreadParamID,
        const String& randParamID);

    /* Intended for Use with fundamental with a 'grid' button */
    void attachToState(AudioProcessorValueTreeState& stateToUse,
        const String& sliderParamID, const String& spreadParamID,
        const String& randParamID, const String& buttonParamID);

    /* Intended for use with formant slider with a 'key' button 
        as well as 
    */
    void attachToState(AudioProcessorValueTreeState& stateToUse, 
        const String& sliderParamID, const String& spreadParamID, 
        const String& randParamID, const String& buttonParamID,
        const String& keyParamID, const String& scaleParamID);

    void setParamIDs(const juce::String& slider,
        const juce::String& spread, const juce::String& rand);

    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked(Button* b) override;
    void buttonStateChanged(Button* b) override;
    void sliderValueChanged(Slider* s) override;



private:
    PulsarUIFeel pulsarFeel;
    SliderType sliderType;

    std::unique_ptr<juce::Slider>   slider;
    std::unique_ptr<juce::Slider>   spreadSlider;
    std::unique_ptr<juce::Slider>   randSlider;
    std::unique_ptr<juce::Slider>   multiSlider;
    std::unique_ptr<Label>          sliderLabel;

    juce::String sliderParam;
    juce::String spreadParam;
    juce::String randParam;

    std::unique_ptr<juce::Button>   button;
    bool containsButton = { false };

    std::unique_ptr<juce::Slider>   keySlider;
    std::unique_ptr<Label>          keyLabel;
    std::unique_ptr<juce::Slider>   scaleSlider;
    bool containsScale = { false };



    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using bAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<Attachment>     attachment;
    std::unique_ptr<Attachment>     spreadAttachment;
    std::unique_ptr<Attachment>     randAttachment;
    std::unique_ptr<Attachment>     keyAttachment;
    std::unique_ptr<Attachment>     scaleAttachment;


    std::unique_ptr<bAttachment>     buttonAttachment;

    PulsarAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PulsarSlider)
};
