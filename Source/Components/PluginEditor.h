/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PulsaretVisualizer.h"
#include "../Processor/PluginProcessor.h"
#include "../LookNFeel/NameFeel.h"
#include "../LookNFeel/MixFeel.h"
#include "../LookNFeel/PulsarUIFeel.h"
#include "BurgerMenuHeader.h"
#include "CommandRunnerComponent.h"
#include "PulsarControlsComponent.h"
#include "PulsarSlider.h"

//==============================================================================
/**
*/
class PulsarAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Timer, public juce::ApplicationCommandTarget,
 public juce::MenuBarModel
{
public:
    PulsarAudioProcessorEditor (PulsarAudioProcessor&);
    ~PulsarAudioProcessorEditor() override;
    void init();
    void resetSliders();
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void repaintPulsaret();
    
    void timerCallback() override;
   
    void setTestColor(juce::Colour c){ color = c; }

    /* Menu Bar Model Virtuals*/
    StringArray getMenuBarNames() override { return { "PRESETS", "TUTORIALS" }; }
    PopupMenu getMenuForIndex(int menuIndex, const String& /*menuName*/) override;
    void menuItemSelected(int /*menuItemID*/, int /*topLevelMenuIndex*/) override {}

    /*Application Command Target Virtuals*/
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;
    void setBurgerMenuPosition();

    enum CommandIDs
    {
        freshStart = 1,
        loadPreset,
        loadUserPreset,
        explainPulsarSynthesis,
        parametersExplained,
        testBasicOperation,
        loadAboutPage,
        loadFoundersAppreciation
    };

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

    ApplicationCommandManager commandManager;
    std::unique_ptr<TextButton> closeButton;
    std::unique_ptr <CommandRunnerComponent>   commandRunner;
    //CommandRunnerComponent commandRunner;

    std::unique_ptr<MenuBarComponent> menuBar;

    SidePanel sidePanel{ "", 300, false };

    BurgerMenuComponent burgerMenu;

    BurgerMenuHeader menuHeader{ sidePanel };


    PulsaretVisualizer  pulsaretVisualizer;

    bool flashState = false;
    float alphaSkew = 0.f;
    float low = 0.f;
    float high = 0.f;
    float center = 0.f;
    
    juce::Rectangle<int> frame;
    Component frameComponent;
    
    NameFeel nameFeel;
    std::unique_ptr<Label>          nameLabel;

    PulsarAudioProcessor& audioProcessor;

    juce::Colour color{ 0, 0, 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PulsarAudioProcessorEditor)
};
