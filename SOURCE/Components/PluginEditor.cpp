/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "../Processor/PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PulsarAudioProcessorEditor::PulsarAudioProcessorEditor (PulsarAudioProcessor& p)
: AudioProcessorEditor (&p), audioProcessor (p), pulsaretVisualizer(p.getTable())
{
    setSize(600, 450);
    frame = juce::Rectangle<int>(15, 50, getWidth() - 30, getHeight() - 65);

    setLookAndFeel(&pulsarFeel);

    pulsaretVisualizer.setBounds(frame);
    addAndMakeVisible(pulsaretVisualizer);

    resetSliders();
    init();

    menuBar.reset(new MenuBarComponent(this));
    addAndMakeVisible(menuBar.get());

    setApplicationCommandManagerToWatch(&commandManager);
    commandManager.registerAllCommandsForTarget(this);

    addKeyListener(commandManager.getKeyMappings());

    addChildComponent(menuHeader);

    commandRunner = std::make_unique<CommandRunnerComponent>(audioProcessor);
    commandRunner->setBounds(frame);

    addChildComponent(commandRunner.get());
    addAndMakeVisible(burgerMenu);
    addAndMakeVisible(sidePanel);
    
    burgerMenu.setModel(this);
    setBurgerMenuPosition();

    setWantsKeyboardFocus(true);

    startTimerHz(120);

}

PulsarAudioProcessorEditor::~PulsarAudioProcessorEditor()
{
}

void PulsarAudioProcessorEditor::init()
{
    nameLabel = std::make_unique<Label>("", "P U L S A R");
    addAndMakeVisible(nameLabel.get());
    nameLabel->setBoundsRelative(0.4f, 0.f, 0.3f, 0.1f);
    nameLabel->setFont(Font("Consolas", "Regular", 20.f));
    nameLabel->setLookAndFeel(&nameFeel);
    nameLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    
}

void PulsarAudioProcessorEditor::resetSliders()
{
    controlComponent = std::make_unique<PulsarControlsComponent>(audioProcessor);
    addAndMakeVisible(controlComponent.get());
    controlComponent->setBoundsRelative(0.f, 0.f, 1.f, 1.f);

    fundamentalSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Fundamental, "Fund", true, "Grid", false);
    addAndMakeVisible(fundamentalSlider.get());
    fundamentalSlider->setBoundsRelative(0.05, 0.15, 0.15, 0.45);
    fundamentalSlider->attachToState(audioProcessor.apvts,
        "Fundamental Freq", "Fundamental Spread",
        "Fundamental Random", "Rhythmic Grid Mode");

    formantSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Formant, "Form", true, "Key", true);
    addAndMakeVisible(formantSlider.get());
    formantSlider->setBoundsRelative(0.2, 0.15, 0.15, 0.45);
    formantSlider->attachToState(audioProcessor.apvts,
        "Formant Freq", "Formant Spread",
        "Formant Random", "Formant Keylock Mode",
        "Form Key", "Form Scale");

    formantSlider2 = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Formant, "Form2", true, "Key", true);
    addAndMakeVisible(formantSlider2.get());
    formantSlider2->setBoundsRelative(0.35, 0.15, 0.15, 0.45);
    formantSlider2->attachToState(audioProcessor.apvts,
        "Formant Freq2", "Formant Spread2",
        "Formant Random2", "Formant2 Keylock Mode",
        "Form Key2", "Form Scale2");

    waveSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Standard, "Wave",
        false, "", false);
    addAndMakeVisible(waveSlider.get());
    waveSlider->setBoundsRelative(0.5, 0.15, 0.15, 0.45);
    waveSlider->attachToState(audioProcessor.apvts,
        "Wave Type", "Wave Spread", "Wave Random");

    ampSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Standard, "Amp",
        false, "", false); // don't know how to set defaults correctly
    addAndMakeVisible(ampSlider.get());
    ampSlider->setBoundsRelative(0.65, 0.15, 0.15, 0.45);
    ampSlider->attachToState(audioProcessor.apvts,
        "Amp", "Amp Spread", "Amp Random");

    panSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Horizontal, "Pan",
        false, "", false);
    addAndMakeVisible(panSlider.get());
    panSlider->setBoundsRelative(0.25f, 0.625f, 0.55f, 0.13f);
    panSlider->attachToState(audioProcessor.apvts,
        "Pan", "Pan Spread", "Pan Random");



}

//==============================================================================
void PulsarAudioProcessorEditor::paint (juce::Graphics& g)
{
    //g.setColour(juce::Colours::black);
    g.setColour(color);
    g.fillAll();
    
    
    g.setColour(juce::Colours::yellow);
    //juce::Rectangle<int> frame(15, 50, getWidth() - 30, getHeight() - 65);  // 25px from sides, 30 from top, 5 below nameLabel, 5 above bottom
    g.drawRoundedRectangle(frame.toFloat(), 5.f, 2.f);
    
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(getLocalBounds().reduced(5).toFloat(), 5.f, 2.f);  
}

void PulsarAudioProcessorEditor::resized()
{
    menuHeader.setBounds(540, 12, 30, 30);
}



void PulsarAudioProcessorEditor::repaintPulsaret()
{

    if(audioProcessor.isFlashing() && audioProcessor.isTrainRunning())
    {
        pulsaretVisualizer.setNewWaveColour(juce::Colours::mediumvioletred.withHue(0.5f));
        pulsaretVisualizer.setAmp(audioProcessor.getFlashCoef());
        pulsaretVisualizer.repaint();
    }
    else
    {
        pulsaretVisualizer.setNewWaveColour(juce::Colours::black);
        pulsaretVisualizer.repaint(); 
    }

}

void PulsarAudioProcessorEditor::timerCallback()
{
    repaint();
    repaintPulsaret();
}


PopupMenu PulsarAudioProcessorEditor::getMenuForIndex(int menuIndex, const String& /*menuName*/)
{
    // pop up menus have 'Options' which allow you to call 'drawPopupMenuWithOptions()' as a lnf class.  
    // With too few items, this menu will expose the defaul coloring of the region below it.  look into this
    // All this to say that menus will only draw as tall as necessary unless you force them to be the correct height.  
    PopupMenu menu; 
    
    menu.setLookAndFeel(&pulsarFeel);

    if (menuIndex == 0)
    {
        menu.addCommandItem(&commandManager, CommandIDs::freshStart);
        menu.addCommandItem(&commandManager, CommandIDs::loadPreset);
        menu.addCommandItem(&commandManager, CommandIDs::loadUserPreset);
    }

    if (menuIndex == 1)
    {
        menu.addCommandItem(&commandManager, CommandIDs::explainPulsarSynthesis);
        menu.addCommandItem(&commandManager, CommandIDs::parametersExplained);
        //menu.addCommandItem(&commandManager, CommandIDs::testBasicOperation);
    }

    //if (menuIndex == 2)
    //{
    //    menu.addCommandItem(&commandManager, CommandIDs::loadAboutPage);
    //    menu.addCommandItem(&commandManager, CommandIDs::loadFoundersAppreciation);
    //}
    

    return menu;
}

ApplicationCommandTarget* PulsarAudioProcessorEditor::getNextCommandTarget()
{
    return nullptr; // might be a weird spot to check out
}

void PulsarAudioProcessorEditor::getAllCommands(Array<CommandID>& c)
{
    Array<CommandID> commands
    { 
        CommandIDs::freshStart,
        CommandIDs::loadPreset,
        CommandIDs::loadUserPreset,
        CommandIDs::explainPulsarSynthesis,
        CommandIDs::parametersExplained,
        //CommandIDs::testBasicOperation,
        //CommandIDs::loadAboutPage,
        //CommandIDs::loadFoundersAppreciation
    };
    c.addArray(commands);
}

void PulsarAudioProcessorEditor::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
    case CommandIDs::freshStart:
        result.setInfo("Start Fresh", "Mode designed for basic Pulsar operation.", "Menu", 0);
        result.setTicked(false);
        // result.addDefaultKeypress('w', ModifierKeys::shiftModifier);
        break;
    case CommandIDs::loadPreset:
        result.setInfo("Signature Presets", "A unique software tutorial reminiscent of video games of yore.", "Menu", 0);
        result.setTicked(false);
        //result.addDefaultKeypress('g', ModifierKeys::shiftModifier);
        break;
    case CommandIDs::loadUserPreset:
        result.setInfo("User Preset", "A unique software tutorial reminiscent of video games of yore.", "Menu", 0);
        result.setTicked(false);
        //result.addDefaultKeypress('g', ModifierKeys::shiftModifier);
        break;
    case CommandIDs::explainPulsarSynthesis:
        result.setInfo("Pulsar Synthesis Overview", "A unique software tutorial reminiscent of video games of yore.", "Menu", 0);
        result.setTicked(false);
        //result.addDefaultKeypress('g', ModifierKeys::shiftModifier);
        break;
    case CommandIDs::parametersExplained:
        result.setInfo("Parameter Dictionary", "A unique software tutorial reminiscent of video games of yore.", "Menu", 0);
        result.setTicked(false);
        //result.addDefaultKeypress('g', ModifierKeys::shiftModifier);
        break;
    //case CommandIDs::testBasicOperation:
    //    result.setInfo("Test your abilities", "A unique software tutorial reminiscent of video games of yore.", "Menu", 0);
    //    result.setTicked(false);
    //    //result.addDefaultKeypress('g', ModifierKeys::shiftModifier);
    //    break;
    //case CommandIDs::loadAboutPage:
    //    result.setInfo("About Page", "A unique software tutorial reminiscent of video games of yore.", "Menu", 0);
    //    result.setTicked(false);
    //    //result.addDefaultKeypress('g', ModifierKeys::shiftModifier);
    //    break;
    //case CommandIDs::loadFoundersAppreciation:
    //    result.setInfo("Founders Appreciation", "A unique software tutorial reminiscent of video games of yore.", "Menu", 0);
    //    result.setTicked(false);
    //    //result.addDefaultKeypress('g', ModifierKeys::shiftModifier);
    //    break;

    default:
        break;
    }
}

bool PulsarAudioProcessorEditor::perform(const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::freshStart:
        commandRunner->setVisible(false);
        audioProcessor.loadPreset(String("C:/ProgramData/Recluse-Audio/Pulsar/Presets/Default"));
        break;
    case CommandIDs::loadPreset:
        commandRunner->setVisible(true); 
        sidePanel.showOrHide(false);
        commandRunner->startPresetMenu();
        break;
    case CommandIDs::loadUserPreset:
        commandRunner->setVisible(true);
        sidePanel.showOrHide(false);
        commandRunner->startUserPresetMenu();
        break;
    case CommandIDs::explainPulsarSynthesis:
        commandRunner->setVisible(true);
        sidePanel.showOrHide(false);
        commandRunner->startTutorial();
        break;
    case CommandIDs::parametersExplained:
        commandRunner->setVisible(true);
        sidePanel.showOrHide(false);
        commandRunner->startParameterDemo();
        break;
    //case CommandIDs::testBasicOperation:
    //    DBG("operation");
    //    break;
    //case CommandIDs::loadAboutPage:
    //    DBG("about");
    //    break;
    //case CommandIDs::loadFoundersAppreciation:
    //    DBG("founders");
    //    break;
    default:
        return false;
    }

    return true;
}

/*
  This is a deviation from the juce example in which this is set 
  by a selection made by the user, and then called from perform()
*/
void PulsarAudioProcessorEditor::setBurgerMenuPosition()
{
    burgerMenu.setModel(this);
    menuHeader.setVisible(true);
    burgerMenu.setSize(sidePanel.getWidth(), sidePanel.getHeight());
    sidePanel.setContent(&burgerMenu, false);
    sidePanel.setColour(juce::SidePanel::backgroundColour, juce::Colours::black);
    menuItemsChanged();
    resized();
}

