/*
  ==============================================================================

    PresetMenu.h
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
class PresetMenu  : public juce::Component
{
public:
    PresetMenu(PulsarAudioProcessor& p) : audioProcessor(p)
    {
        loadFiles();
        prepareMenu();

    }

    ~PresetMenu() override
    {
    }

    void paint (juce::Graphics& g) override
    {
 // draw an outline around the component
    }

    void showMenu()
    {
        int selection = menu.showMenu(PopupMenu::Options()
            .withMinimumWidth(200)
            .withMaximumNumColumns(3)
            .withTargetScreenArea(getScreenBounds())
            .withPreferredPopupDirection(PopupMenu::Options::PopupDirection::upwards)
        );;

        if (selection == 0)
        {
            menu.dismissAllActiveMenus();
        }
        else if (selection >= 1)
        {
            /*
                TODO: clean this up
                array of file paths starts at 0, but menu index starts at 1 (0 = close menu)
            */
            int index = selection - 1;
            auto filePath = getFilePathFromIndex(index);
            audioProcessor.loadPreset(filePath);
            showMenu();
        }
    }
    void resized() override
    {


    }
    void prepareMenu()
    {
        menu.setLookAndFeel(&pulsarFeel);

        juce::ScopedPointer<juce::PopupMenu> presetMenu = new juce::PopupMenu;

        int itemIndex = 1; // used to properly index artist romples without resetting in the artist loop

        for (size_t i = 0; i < presetNames.size(); i++)
        {
            presetMenu->addItem(itemIndex, presetNames[i]);
            itemIndex++;
        }

        menu.addSectionHeader("Presets");
        menu.addSeparator();
        menu.addSubMenu("", *presetMenu);
    }

    void loadFiles()
    {
        auto presetFolders = juce::File("C:/ProgramData/Recluse-Audio/Pulsar/Presets/").findChildFiles(2, true);

        for (int i = 0; i < presetFolders.size(); i++)
        {
            filePaths.add(presetFolders[i].getFullPathName());
            presetNames.add(presetFolders[i].getFileNameWithoutExtension());
        }
    }

    juce::String& getFilePathFromIndex(int index) { return filePaths.getReference(index); }

private:
    PulsarUIFeel pulsarFeel;
    PopupMenu menu;
    juce::StringArray filePaths;
    juce::StringArray presetNames;

    PulsarAudioProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetMenu)
};
