/*
  ==============================================================================

    PulsarSlider.cpp
    Created: 19 Aug 2021 11:51:05am
    Author:  ryand

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PulsarSlider.h"

//==============================================================================
PulsarSlider::PulsarSlider(PulsarAudioProcessor& p,
    SliderType type, const String& labelText,
    bool hasButton = false, const String& buttonText = { "" },
    bool hasIncDecSlider = { false }) 
    : audioProcessor(p), sliderType(type)
{
    auto threeValue = SliderStyle::ThreeValueVertical;
    auto sliderBarStyle = SliderStyle::LinearBarVertical;
    auto randStyle = SliderStyle::LinearHorizontal;
    auto spreadStyle = SliderStyle::LinearVertical;

    if (type == SliderType::Horizontal)
    {
        threeValue = SliderStyle::ThreeValueHorizontal;
        sliderBarStyle = SliderStyle::LinearHorizontal;
        randStyle = SliderStyle::LinearVertical;
        spreadStyle = SliderStyle::LinearHorizontal;
    }

    multiSlider = std::make_unique<Slider>(threeValue, Slider::NoTextBox);
    multiSlider->addListener(this);
    multiSlider->setLookAndFeel(&pulsarFeel);
    addAndMakeVisible(multiSlider.get());

    slider = std::make_unique<Slider>(sliderBarStyle, Slider::TextBoxBelow);
    slider->addListener(this);
    slider->setLookAndFeel(&pulsarFeel);
    addAndMakeVisible(slider.get());

    randSlider = std::make_unique<Slider>(randStyle, Slider::NoTextBox);
    randSlider->addListener(this);
    randSlider->setLookAndFeel(&pulsarFeel);
    addAndMakeVisible(randSlider.get());

    spreadSlider = std::make_unique<Slider>(spreadStyle, Slider::NoTextBox);
    spreadSlider->addListener(this);
    spreadSlider->setLookAndFeel(&pulsarFeel);
    addAndMakeVisible(spreadSlider.get());

    sliderLabel = std::make_unique<Label>(labelText, labelText);
    addAndMakeVisible(sliderLabel.get());
    sliderLabel->attachToComponent(slider.get(), false);
    sliderLabel->setJustificationType(Justification::centred);
    sliderLabel->setLookAndFeel(&pulsarFeel);

    if (hasButton)
    {
        addButton(buttonText);
        containsButton = true;
    }

    if (hasIncDecSlider)
    {
        addIncDecSlider();
        containsScale = true;
    }


}

void PulsarSlider::addButton(const String& buttonText)
{
    button = std::make_unique<TextButton>(buttonText);
    button->addListener(this);
    button->setClickingTogglesState(true);
    button->setColour(juce::TextButton::buttonColourId, juce::Colours::orange.withAlpha(0.5f));
    button->setColour(juce::TextButton::buttonOnColourId, juce::Colours::orange.withAlpha(0.8f));
    addAndMakeVisible(button.get());

}

void PulsarSlider::addIncDecSlider()
{
    keySlider = std::make_unique<Slider>(Slider::SliderStyle::IncDecButtons, Slider::TextBoxBelow);
    keySlider->setRange(0, 11, 1);
    keySlider->addListener(this);
    keySlider->setLookAndFeel(&pulsarFeel);
    addChildComponent(keySlider.get());

    scaleSlider = std::make_unique<Slider>(Slider::SliderStyle::IncDecButtons, Slider::TextBoxLeft);
    scaleSlider->setRange(0, 5, 1);
    scaleSlider->addListener(this);
    scaleSlider->setLookAndFeel(&pulsarFeel);
    addChildComponent(scaleSlider.get());
}


PulsarSlider::~PulsarSlider()
{
}

void PulsarSlider::attachToState(AudioProcessorValueTreeState& stateToUse, 
    const String& sliderParamID, const String& spreadParamID,
    const String& randParamID)
{
    setParamIDs(sliderParamID, spreadParamID, randParamID);

    attachment = std::make_unique<Attachment>(audioProcessor.apvts, sliderParamID, *slider);
    spreadAttachment = std::make_unique<Attachment>(audioProcessor.apvts, spreadParamID, *spreadSlider);
    randAttachment = std::make_unique<Attachment>(audioProcessor.apvts, randParamID, *randSlider);
}

void PulsarSlider::attachToState(AudioProcessorValueTreeState& stateToUse, 
    const String& sliderParamID, const String& spreadParamID, 
    const String& randParamID, const String& buttonParamID = { "empty" })
{
    setParamIDs(sliderParamID, spreadParamID, randParamID);

    attachment = std::make_unique<Attachment>(audioProcessor.apvts, sliderParamID, *slider);
    spreadAttachment = std::make_unique<Attachment>(audioProcessor.apvts, spreadParamID, *spreadSlider);
    randAttachment = std::make_unique<Attachment>(audioProcessor.apvts, randParamID, *randSlider);

    buttonAttachment = std::make_unique<bAttachment>(audioProcessor.apvts, buttonParamID, *button);
}

void PulsarSlider::attachToState(AudioProcessorValueTreeState& stateToUse, 
    const String& sliderParamID, const String& spreadParamID, 
    const String& randParamID, const String& buttonParamID = { "empty" },
    const String& keyParamID = { "empty" }, const String& scaleParamID = { "empty" })
{
    setParamIDs(sliderParamID, spreadParamID, randParamID);

    attachment = std::make_unique<Attachment>(audioProcessor.apvts, sliderParamID, *slider);
    spreadAttachment = std::make_unique<Attachment>(audioProcessor.apvts, spreadParamID, *spreadSlider);
    randAttachment = std::make_unique<Attachment>(audioProcessor.apvts, randParamID, *randSlider);

    buttonAttachment = std::make_unique<bAttachment>(audioProcessor.apvts, buttonParamID, *button);
    keyAttachment = std::make_unique<Attachment>(audioProcessor.apvts, keyParamID, *keySlider);
    scaleAttachment = std::make_unique<Attachment>(audioProcessor.apvts, scaleParamID, *scaleSlider);
}

void PulsarSlider::setParamIDs(const juce::String& slider, const juce::String& spread, const juce::String& rand)
{
    sliderParam = slider; spreadParam = spread, randParam = rand;
}




void PulsarSlider::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    g.setColour(juce::Colours::transparentBlack.withAlpha(0.f));

    g.fillAll ();   // clear the background
}

void PulsarSlider::resized()
{
    slider->setBoundsRelative       (0.0f,   0.1f,  0.3f,    0.8f);
    multiSlider->setBoundsRelative  (0.0f,   0.1f,  0.3f,    0.8f);
    spreadSlider->setBoundsRelative (0.35f,   0.1f,  0.1f,    0.8f);
    randSlider->setBoundsRelative   (0.0f,   0.92f, 0.8f,    0.05f);

    if (sliderType == SliderType::Fundamental)
    {
        button->setBoundsRelative(0.65f, 0.1f, 0.3f, 0.075f);
    }

    if (sliderType == SliderType::Formant)
    {
        button->setBoundsRelative(0.65f, 0.1f, 0.3f, 0.075f);
        keySlider->setBoundsRelative(0.49f, 0.3f, 0.5f, 0.15f);
        scaleSlider->setBoundsRelative(0.49f, 0.5f, 0.5f, 0.15f);
    }
    
    if (sliderType == SliderType::Horizontal)
    {
        slider->setBoundsRelative(0.05f, 0.21f, 0.85f, 0.55f);
        multiSlider->setBoundsRelative(0.05f, 0.21f, 0.85f, 0.55f);
        spreadSlider->setBoundsRelative(0.05f, 0.85f, 0.85f, 0.15f);
        randSlider->setBoundsRelative(0.f, 0.f, 0.025f, 1.f);
    }
}

void PulsarSlider::buttonClicked(Button* b)
{
    
}

void PulsarSlider::buttonStateChanged(Button* b)
{
    if (b == button.get())
    {

        /* essentially a flag, this tells me it is a fund*/
        if (sliderType == SliderType::Fundamental)
        {
            if (b->getToggleState())
            {
                slider->setSkewFactor(1.0);
                slider->textFromValueFunction = [](float sliderValue)
                {
                    // DBG(sliderValue);
                    if (sliderValue == 1)
                        return "1/1";
                    if (sliderValue > 1 && sliderValue <= 10)
                        return "1/2";
                    if (sliderValue > 10 && sliderValue <= 20)
                        return "1/2T";
                    if (sliderValue > 20 && sliderValue <= 40)
                        return "1/4";
                    if (sliderValue > 40 && sliderValue <= 60)
                        return "1/4T";
                    if (sliderValue > 60 && sliderValue <= 80)
                        return "1/8";
                    if (sliderValue > 80 && sliderValue <= 100)
                        return "1/8T";
                    if (sliderValue > 100 && sliderValue <= 120)
                        return "1/16";
                    if (sliderValue > 120 && sliderValue <= 140)
                        return "1/16T";
                    if (sliderValue > 140 && sliderValue <= 160)
                        return "1/32";
                    if (sliderValue > 160)
                        return "1/32T";
                };
            }
            else
            {
                slider->setSkewFactor(0.5f);
                slider->textFromValueFunction = [](float sliderValue)
                {
                    return String(sliderValue);
                };
            }
            
        } //

        if (sliderType == SliderType::Formant)
        {
            scaleSlider->textFromValueFunction = [](double sliderValue)
            {
                switch ((int)sliderValue)
                {
                case 0:
                    return "Major";
                case 1:
                    return "Minor";
                case 2:
                    return "Pent(maj)";
                case 3:
                    return "Pent(min)";
                case 4:
                    return "Neg(small)";
                case 5:
                    return "Neg(big)";
                }
            };

            if (b->getToggleState())
            {
                keySlider->setVisible(true);
                scaleSlider->setVisible(true);
                //spreadSlider->setVisible(false);
            }
            else if (!b->getToggleState())
            {
                keySlider->setVisible(false);
                scaleSlider->setVisible(false);
                //spreadSlider->setVisible(true);

            }
        }



    }

}

void PulsarSlider::sliderValueChanged(Slider* s)
{
    if (s == keySlider.get())
    {
        keySlider->textFromValueFunction = [](double sliderValue)
        {

            switch ((int)sliderValue)
            {
            case 0:
                return "C";
            case 1:
                return "C#/Db";
            case 2:
                return "D";
            case 3:
                return "D#/Eb";
            case 4:
                return "E";
            case 5:
                return "F";
            case 6:
                return "F#/Gb";
            case 7:
                return "G";
            case 8:
                return "G#/Ab";
            case 9:
                return "A";
            case 10:
                return "A#/Bb";
            case 11:
                return "B";
            }
        };
    }

    if (s == scaleSlider.get())
    {
        scaleSlider->textFromValueFunction = [](double sliderValue)
        {
            switch ((int)sliderValue)
            {
            case 0:
                return "Major";
            case 1:
                return "Minor";
            case 2:
                return "Pent(maj)";
            case 3:
                return "Pent(min)";
            case 4:
                return "Neg(small)";
            case 5:
                return "Neg(big)";
            }
        };
    }

    if (s == spreadSlider.get() || s == slider.get() || s == randSlider.get()) // sorry this is different format than the rest, just experimenting with clearer writing
    {
        float spread = *audioProcessor.apvts.getRawParameterValue(spreadParam);         
        float param = *audioProcessor.apvts.getRawParameterValue(sliderParam);
        float rand = *audioProcessor.apvts.getRawParameterValue(randParam);

        auto range = audioProcessor.apvts.getParameterRange(sliderParam);
        auto spreadRange = audioProcessor.apvts.getParameterRange(spreadParam);
        float min = param, mid = param, max = param;

        multiSlider->setNormalisableRange(NormalisableRange<double>
            (0.0, (double) range.end, (double) range.interval, (double) range.skew ));

        multiSlider->setColour(Slider::backgroundColourId, juce::Colours::mediumvioletred.withAlpha(rand));

        if (range.skew < 1.f) // checking for fund or form sliders
        {
            
            if (spread > spreadRange.start && spread < spreadRange.end)
            {

                min = param / spread;
                mid = param;
                max = param * spread;
            }
            else if (spread == spreadRange.end)
            {
                min = range.start;
                max = range.end;
            }
            else if (spread == spreadRange.start)
            {
                multiSlider->setColour(Slider::backgroundColourId, Colours::transparentBlack.withAlpha(0.f));
            }
        }
        else
        {
            if (spread > spreadRange.start && spread < spreadRange.end)
            {
                min = param - (spread/2);
                mid = param;
                max = param + (spread/2);
            }
            else if (spread == spreadRange.end)
            {
                min = range.start;
                max = range.end;
            }
            else if (spread == spreadRange.start)
            {
                multiSlider->setColour(Slider::backgroundColourId, Colours::transparentBlack.withAlpha(0.f));
            }
        }
        

        multiSlider->setMinValue(min);
        multiSlider->setValue(mid);
        multiSlider->setMaxValue(max);
    }
}
