/*
  ==============================================================================

    CommandRunnerComponent.h
    Created: 12 Aug 2021 3:17:29pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Processor/PluginProcessor.h"
#include "../Components/TutorialImageComponent.h"
#include "../Components/DemoComponent.h"
#include "../Components/PresetMenu.h"
#include "../Components/UserPresetMenu.h"
#include "../Components/ParameterDictionary.h"
#include "../LookNFeel/PulsarUIFeel.h"
//==============================================================================
/*
*/
class CommandRunnerComponent  : public juce::Component, public juce::Button::Listener
{
public:
    CommandRunnerComponent(PulsarAudioProcessor& p) : audioProcessor(p)
    {
        tutorialRunner = std::make_unique<TutorialImageComponent>();
        addChildComponent(tutorialRunner.get());

        paramDictionary = std::make_unique<ParameterDictionary>();
        addChildComponent(paramDictionary.get());

        demoRunner = std::make_unique<DemoComponent>(audioProcessor);
        addChildComponent(demoRunner.get());

        presetMenu = std::make_unique<PresetMenu>(audioProcessor);
        addChildComponent(presetMenu.get());

        userPresetMenu = std::make_unique<UserPresetMenu>(audioProcessor);
        addChildComponent(userPresetMenu.get());

        setLookAndFeel(&pulsarFeel);

        closeButton = std::make_unique<TextButton>("Close");
        addAndMakeVisible(closeButton.get());
        closeButton->setBoundsRelative(0.83, 0.05, 0.15, 0.05);
        closeButton->setAlwaysOnTop(true);
        closeButton->addListener(this);
        closeButton->setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    }

    ~CommandRunnerComponent() override
    {
        
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::transparentBlack.withAlpha(0.f));   // clear the background
    }

    void resized() override
    {
        tutorialRunner->setBounds(getLocalBounds().reduced(2.f));
        demoRunner->setBounds(getLocalBounds().reduced(2.f));
        paramDictionary->setBounds(getLocalBounds().reduced(2.f));

        presetMenu->setBoundsRelative(0.25, 0.1, 0.5f, 0.5f);
        userPresetMenu->setBoundsRelative(0.25, 0.1, 0.5f, 0.5f);

        closeButton->setBoundsRelative(0.83, 0.05, 0.15, 0.05);
    }

    void startTutorial()
    {
        reset();
        tutorialRunner->setVisible(true);
        tutorialRunner->start();
    }
   // image = juce::ImageFileFormat::loadFrom(juce::File("D:/Artie_Photos/tape_1.png"));
    void startPresetMenu()
    {
        reset();
        presetMenu->setVisible(true);
        presetMenu->showMenu();
    }

    void startDemo()
    {
        reset();
        demoRunner->setVisible(true);
    }

    void startParameterDemo()
    {
        reset();
        paramDictionary->setVisible(true);
        paramDictionary->start();
    }
    void startUserPresetMenu()
    {
        reset();
        userPresetMenu->setVisible(true);
        userPresetMenu->showMenu();
    }

    void buttonClicked(Button* b) override
    {
        if (b == closeButton.get())
        {
            setVisible(false);
        }
    }



    void reset()
    {
        tutorialRunner->setVisible(false);
        paramDictionary->setVisible(false);
        userPresetMenu->setVisible(false);
        presetMenu->setVisible(false);
        demoRunner->setVisible(false);
    }

private:
    PulsarUIFeel pulsarFeel;

    std::unique_ptr <TutorialImageComponent> tutorialRunner;
    std::unique_ptr <DemoComponent>          demoRunner;
    std::unique_ptr<ParameterDictionary>     paramDictionary;


    std::unique_ptr <PresetMenu> presetMenu;
    std::unique_ptr <UserPresetMenu> userPresetMenu;

    std::unique_ptr<TextButton> closeButton;


    PulsarAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CommandRunnerComponent)
};
