/*
  ==============================================================================

    DemoComponent.h
    Created: 24 Aug 2021 12:32:12pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../LookNFeel/PulsarUIFeel.h"
#include "../Processor/PluginProcessor.h"


//==============================================================================
/*
*/
class DemoComponent  : public juce::Component, public Button::Listener, public juce::Timer
{
public:
    DemoComponent(PulsarAudioProcessor& p) : audioProcessor(p)
    {
        textLabel = std::make_unique<Label>();
        addAndMakeVisible(textLabel.get());

        playDemoButton = std::make_unique<TextButton>("Play");
        addAndMakeVisible(playDemoButton.get());
        playDemoButton->setAlwaysOnTop(true);
        playDemoButton->addListener(this);
        playDemoButton->setColour(juce::TextButton::buttonColourId, juce::Colours::orange);

        nextDemoButton = std::make_unique<TextButton>("Next");
        addAndMakeVisible(nextDemoButton.get());
        nextDemoButton->setAlwaysOnTop(true);
        nextDemoButton->addListener(this);
        nextDemoButton->setColour(juce::TextButton::buttonColourId, juce::Colours::orange);

        prevDemoButton = std::make_unique<TextButton>("Prev");
        addAndMakeVisible(prevDemoButton.get());
        prevDemoButton->setAlwaysOnTop(true);
        prevDemoButton->addListener(this);
        prevDemoButton->setColour(juce::TextButton::buttonColourId, juce::Colours::orange);

        initDemoText();
    }

    ~DemoComponent() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::transparentBlack.withAlpha(0.f));
    }

    void resized() override
    {
        textLabel->setBoundsRelative(0.1, 0.1, 0.1, 0.1);
        playDemoButton->setBoundsRelative(0.375, 0.9, 0.15, 0.05);
        nextDemoButton->setBoundsRelative(0.83, 0.9, 0.15, 0.05);
        prevDemoButton->setBoundsRelative(0.02, 0.9, 0.15, 0.05);
    }

    void buttonClicked(Button* b) override
    {
        if (b == nextDemoButton.get())
        {
            currentDemo = jlimit<int>(0, 4, currentDemo++);
        }
        if (b == prevDemoButton.get())
        {
            currentDemo = jlimit<int>(0, 4, currentDemo--);

        }
        if (b == playDemoButton.get())
        {
            startCurrentDemo();
        }
    }

    void startCurrentDemo()
    {
        switch (currentDemo)
        {
        case 0:
            param = audioProcessor.apvts.getParameter("Fundamental Freq");
            fundamentalDemo();
            break;
        case 1:
            param = audioProcessor.apvts.getParameter("Formant Freq");
            formantDemo();
            break;
        case 2:
            param = audioProcessor.apvts.getParameter("Wave Type");
            waveDemo();
            break;
        case 3:
            param = audioProcessor.apvts.getParameter("Pan");
            panDemo();
            break;
        }

        initParamDemo();
    }

    void initParamDemo()
    {
        textLabel->setText(param->getName(16), NotificationType::sendNotificationAsync);
        startTimer(50);
    }

    void calculateIncrement(float startVal, float endVal, int dur)
    {
        auto range = endVal - startVal;
        durationMS = dur;
        valIncrement = range / durationMS;
    }

    void timerCallback() override
    {

        param->beginChangeGesture();
        param->setValueNotifyingHost(demoVal);
        param->endChangeGesture();

        incrementVal();

        timerIncrement + 50;
        if (timerIncrement > durationMS)
        {
            timerIncrement = 0;
            stopTimer();
        }
    }

    void incrementVal()
    {
        auto val = demoVal + valIncrement;
        demoVal = jlimit<float>(1.f, param->getNormalisableRange().end, val);
    }

    void fundamentalDemo()
    {
        calculateIncrement(10.f, 120.f, 6000);
    }

    void formantDemo()
    {
        calculateIncrement(200.f, 2000.f, 6000);
    }

    void waveDemo()
    {
        calculateIncrement(0.f, 100.f, 6000);
    }

    void panDemo()
    {
        calculateIncrement(0.f, 100.f, 6000);
    }

private:
    PulsarUIFeel pulsarFeel;

    int currentDemo = 0;
    float demoVal = 0.f;
    float valIncrement = 0.f;
    int timerIncrement = 0;
    int durationMS = 0;

    std::unique_ptr<Label> textLabel;

    std::unique_ptr<TextButton> playDemoButton;
    std::unique_ptr<TextButton> nextDemoButton;
    std::unique_ptr<TextButton> prevDemoButton;

    PulsarAudioProcessor& audioProcessor;
    RangedAudioParameter* param;

    juce::StringArray demoTextArray;

    void initDemoText()
    {
        demoTextArray.set(0, "Fundamental");
        demoTextArray.set(1, "Formants");
        demoTextArray.set(2, "Wave");
        demoTextArray.set(3, "Formant Stochasticism");
        demoTextArray.set(4, "Pan Stochasticism");
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoComponent)
};
