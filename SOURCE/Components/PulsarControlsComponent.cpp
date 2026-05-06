/*
  ==============================================================================

    PulsarControlsComponent.cpp
    Created: 23 Aug 2021 8:01:33pm
    Author:  ryand

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PulsarControlsComponent.h"
#include "../Processor/PluginProcessor.h"

//==============================================================================
PulsarControlsComponent::PulsarControlsComponent(PulsarAudioProcessor& p) : audioProcessor(p)
{
    
    setLookAndFeel(&pulsarFeel);

    triggerButton = std::make_unique<TextButton>("TRIGGER");
    triggerButton->addListener(this);
    triggerButton->setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack.withAlpha(0.f));
    triggerButton->setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack.withAlpha(0.f));
    addAndMakeVisible(triggerButton.get());

    runButton = std::make_unique<TextButton>("RUN");
    runButton->setClickingTogglesState(true);
    runButton->addListener(this);
    runButton->setColour(juce::TextButton::buttonColourId, juce::Colours::darkorange.withAlpha(0.2f));
    runButton->setColour(juce::TextButton::buttonOnColourId, juce::Colours::darkorange.withAlpha(0.7f));
    addAndMakeVisible(runButton.get());


    
   createSliders();
   drawLabels();
   attachToState();

}

PulsarControlsComponent::~PulsarControlsComponent()
{
}

void PulsarControlsComponent::paint ([[maybe_unused]] juce::Graphics& g)
{

   // g.fillAll(juce::Colours::transparentBlack.withAlpha(0.f));
}

void PulsarControlsComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    runButton->setBoundsRelative(0.05f, 0.035f, 0.1f, 0.05f);
    triggerButton->setBoundsRelative(0.175f, 0.035f, 0.1f, 0.05f);

    attackSlider->setBoundsRelative(0.32f, 0.825f, 0.06f, 0.125f);
    decaySlider->setBoundsRelative(0.42f, 0.825f, 0.06f, 0.125f);
    sustainSlider->setBoundsRelative(0.52f, 0.825f, 0.06f, 0.125f);
    releaseSlider->setBoundsRelative(0.62f, 0.825f, 0.06f, 0.125f);

    glideSlider->setBoundsRelative(0.05f, 0.85f, 0.1f, 0.1f);
    widthSlider->setBoundsRelative(0.85f, 0.85f, 0.1f, 0.1f);

    interSlider->setBoundsRelative(0.8f, 0.2f, 0.035f, 0.35f);
    triggerOnSlider->setBoundsRelative(0.85f, 0.2f, 0.035f, 0.35f);
    triggerOffSlider->setBoundsRelative(0.9f, 0.2f, 0.035f, 0.35f);

    //glideSlider->setBounds(455, getHeight() - 77, 40, 55);
    //interSlider->setBounds(496, 80, 10, 160);
    //widthSlider->setBounds(35, getHeight() - 77, 40, 55);
    //triggerOnSlider->setBounds(447, 80, 22, 160);
    //triggerOffSlider->setBounds(470, 80, 22, 160);
}

void PulsarControlsComponent::buttonClicked([[maybe_unused]] Button* b)
{

}

void PulsarControlsComponent::buttonStateChanged(Button* b)
{
    if (b == triggerButton.get())
    {
        if (b->isDown())
        {
            audioProcessor.triggerPulsarTrain();
        }
        else
        {
            audioProcessor.releasePulsarTrain();
        }
    }

    if (b == runButton.get())
    {
        if (b->getToggleState())
        {
            audioProcessor.triggerPulsarTrain();
        }
        else
        {
            audioProcessor.releasePulsarTrain();
        }
    }
}

void PulsarControlsComponent::createSliders()
{

    attackSlider = std::make_unique<Slider>(Slider::SliderStyle::LinearBarVertical, Slider::TextBoxBelow);
    addAndMakeVisible(attackSlider.get());

    decaySlider = std::make_unique<Slider>(Slider::SliderStyle::LinearBarVertical, Slider::TextBoxBelow);
    addAndMakeVisible(decaySlider.get());

    sustainSlider = std::make_unique<Slider>(Slider::SliderStyle::LinearBarVertical, Slider::TextBoxBelow);
    addAndMakeVisible(sustainSlider.get());

    releaseSlider = std::make_unique<Slider>(Slider::SliderStyle::LinearBarVertical, Slider::TextBoxBelow);
    addAndMakeVisible(releaseSlider.get());

    glideSlider = std::make_unique<Slider>(Slider::SliderStyle::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow);
    addAndMakeVisible(glideSlider.get());

    widthSlider = std::make_unique<Slider>(Slider::SliderStyle::RotaryHorizontalVerticalDrag, Slider::TextBoxBelow);
    addAndMakeVisible(widthSlider.get());

    interSlider = std::make_unique<Slider>(Slider::SliderStyle::LinearVertical, Slider::NoTextBox);
    addAndMakeVisible(interSlider.get());

    triggerOnSlider = std::make_unique<Slider>(Slider::SliderStyle::LinearBarVertical, Slider::TextBoxBelow);
    addAndMakeVisible(triggerOnSlider.get());

    triggerOffSlider = std::make_unique<Slider>(Slider::SliderStyle::LinearBarVertical, Slider::TextBoxBelow);
    addAndMakeVisible(triggerOffSlider.get());

}

void PulsarControlsComponent::drawLabels()
{
    interLabel = std::make_unique<Label>("", "Int");
    addAndMakeVisible(interLabel.get());
    interLabel->attachToComponent(interSlider.get(), false);
    interLabel->setJustificationType(Justification::centred);
    interLabel->setLookAndFeel(&pulsarFeel);

    widthLabel = std::make_unique<Label>("", "Width");
    addAndMakeVisible(widthLabel.get());
    widthLabel->attachToComponent(widthSlider.get(), false);
    widthLabel->setJustificationType(Justification::centred);
    widthLabel->setLookAndFeel(&pulsarFeel);

    glideLabel = std::make_unique<Label>("", "Glide");
    addAndMakeVisible(glideLabel.get());
    glideLabel->attachToComponent(glideSlider.get(), false);
    glideLabel->setJustificationType(Justification::centred);
    glideLabel->setLookAndFeel(&pulsarFeel);

    attackLabel = std::make_unique<Label>("", "A");
    addAndMakeVisible(attackLabel.get());
    attackLabel->attachToComponent(attackSlider.get(), false);
    attackLabel->setJustificationType(Justification::centred);
    attackLabel->setLookAndFeel(&pulsarFeel);

    decayLabel = std::make_unique<Label>("", "D");
    decayLabel->attachToComponent(decaySlider.get(), false);
    decayLabel->setJustificationType(Justification::centred);
    decayLabel->setLookAndFeel(&pulsarFeel);

    sustainLabel = std::make_unique<Label>("", "S");
    sustainLabel->attachToComponent(sustainSlider.get(), false);
    sustainLabel->setJustificationType(Justification::centred);
    sustainLabel->setLookAndFeel(&pulsarFeel);

    releaseLabel = std::make_unique<Label>("", "R");
    releaseLabel->attachToComponent(releaseSlider.get(), false);
    releaseLabel->setJustificationType(Justification::centred);
    releaseLabel->setLookAndFeel(&pulsarFeel);

    onLabel = std::make_unique<Label>("", "On");
    onLabel->attachToComponent(triggerOnSlider.get(), false);
    onLabel->setJustificationType(Justification::centred);
    onLabel->setLookAndFeel(&pulsarFeel);

    offLabel = std::make_unique<Label>("", "Off");
    offLabel->attachToComponent(triggerOffSlider.get(), false);
    offLabel->setJustificationType(Justification::centred);
    offLabel->setLookAndFeel(&pulsarFeel);

}

/*
    TODO:  If not for the 'audioProcessor.triggerPulsarTrain()' function I would not pass
    a ref to the audio processor in the constructor.  The 'trigger' and 'run' buttons are not o
    on the value tree.  I imagine they should be.
*/
void PulsarControlsComponent::attachToState()
{
    using namespace Pulsar;

    triggerAttachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, kTriggerID, *triggerButton);
    runAttachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, kTriggerID, *runButton);

    interAttachment     = std::make_unique<Attachment>(audioProcessor.apvts, kIntermittanceID, *interSlider);
    triggerOnAttachment = std::make_unique<Attachment>(audioProcessor.apvts, kTriggerOnID,     *triggerOnSlider);
    triggerOffAttachment= std::make_unique<Attachment>(audioProcessor.apvts, kTriggerOffID,    *triggerOffSlider);

    widthAttachment   = std::make_unique<Attachment>(audioProcessor.apvts, kWidthID,       *widthSlider);

    attackAttachment  = std::make_unique<Attachment>(audioProcessor.apvts, kAttackID,      *attackSlider);
    decayAttachment   = std::make_unique<Attachment>(audioProcessor.apvts, kDecayID,       *decaySlider);
    sustainAttachment = std::make_unique<Attachment>(audioProcessor.apvts, kSustainLevelID,*sustainSlider);
    releaseAttachment = std::make_unique<Attachment>(audioProcessor.apvts, kReleaseID,     *releaseSlider);

    glideAttachment   = std::make_unique<Attachment>(audioProcessor.apvts, kGlideTimeID,   *glideSlider);
}
