/*
  ==============================================================================

    UserPresetMenu.h
    Created: 12 Aug 2021 7:30:57pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../LookNFeel/PulsarUIFeel.h"
#include "../Processor/PluginProcessor.h"

//==============================================================================
/*
        Currently working to load xml files or possible binary data,
        Right now I have three dummy .txt. files to practice with in a 'preset folder'
        have to look into how to write/read to binary data instead of xml
*/
class UserPresetMenu : public juce::Component
{
public:
    UserPresetMenu(PulsarAudioProcessor& p) : audioProcessor(p)
    {
        initFiles();
        prepareMenu();
        addChildComponent(textInput);
        textInput.setEditable(true);
        textInput.onTextChange = [this] { createNewPreset(textInput.getText()); };

        textInput.setBoundsRelative(0.3, 0.3, 0.4, 0.2);
        textInput.setColour(juce::Label::backgroundColourId, juce::Colours::black);
        textInput.setJustificationType(Justification::centred);
        textInput.setFont(Font("Consolas", "Regular", 24.f));
    }

    ~UserPresetMenu() override
    {
    }

    void paint(juce::Graphics& g) override
    {

    }

    void showMenu()
    {
        int selection = menu.showMenu(PopupMenu::Options()
            .withMinimumWidth(200)
            .withMaximumNumColumns(3)
            .withTargetScreenArea(getScreenBounds())
            .withPreferredPopupDirection(PopupMenu::Options::PopupDirection::upwards)
            );
        
        executeSelection(selection);
    }

    void executeSelection(int selection)
    {
        if (selection == 0)
        {
            menu.dismissAllActiveMenus();
        }
        if (selection == 1)
        {
            savePreset();
        }
        else if (selection > 1)
        {
            /*
                TODO: clean this up
                array of file paths starts at 0, but menu index starts at 1 (0 = close menu)
                I want save to be the first option so I gave it index 1
                Therefore the selection has an offset of 2 when retrieving filePaths from array
            */
            int index = selection - 2;
            auto filePath = getFilePathFromIndex(index);
            audioProcessor.loadPreset(filePath);
            showMenu();
        }
    }

    void savePreset()
    {
        textInput.setVisible(true);
        textInput.showEditor();
    }

    void resized() override
    {
        textInput.setBoundsRelative(0.3, 0.3, 0.4, 0.2);
        textInput.setColour(juce::Label::backgroundColourId, juce::Colours::black);
        textInput.setJustificationType(Justification::centred);
        textInput.setFont(Font("Consolas", "Regular", 16.f));
        sendLookAndFeelChange();
    }

    void prepareMenu()
    {
        menu.clear();
        menu.setLookAndFeel(&pulsarFeel);

        juce::ScopedPointer<juce::PopupMenu> loadMenu = new juce::PopupMenu();
        int loadIndex = 2; // can't be 0, 1 is reserved for 'save' command
        for (size_t i = 0; i < presetNames.size(); i++)
        {
            loadMenu->addItem(loadIndex, presetNames[i]);
            loadIndex++;
        }

        menu.addSectionHeader("User Presets");
        menu.addItem(1, "Store");
        menu.addSeparator();
        menu.addSubMenu("Load", *loadMenu);
    }

    void initFiles()
    {
        auto presetFolders = juce::File("C:/ProgramData/Recluse-Audio/Pulsar/User Presets/").findChildFiles(2, true);

        for (int i = 0; i < presetFolders.size(); i++)
        {
            filePaths.add(presetFolders[i].getFullPathName());
            presetNames.add(presetFolders[i].getFileNameWithoutExtension());
        }
    }

    juce::String& getFilePathFromIndex(int index) 
    { 
        return filePaths.getReference(index); 
    }

    void createNewPreset(juce::String& presetName)
    {
        juce::String filePath = "C:/ProgramData/Recluse-Audio/Pulsar/User Presets/";
        filePath.append(presetName, 64);
        audioProcessor.savePreset(filePath);
        textInput.hideEditor(true);
        textInput.setVisible(false);
        initFiles();
        prepareMenu();
    }
private:
    PulsarUIFeel pulsarFeel;
    PopupMenu menu;
    Label textInput;
    juce::StringArray filePaths;
    juce::StringArray presetNames;
   // ModalCallbackFunction
    PulsarAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UserPresetMenu)
};
