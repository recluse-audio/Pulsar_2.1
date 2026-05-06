/*
  ==============================================================================

    PresetMenu.h
    Created: 12 Aug 2021 7:30:57pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PulsarUIFeel.h"
#include "PluginProcessor.h"

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
        menu.showMenuAsync(PopupMenu::Options().withMinimumWidth(200)
                           .withMaximumNumColumns(3)
                           .withTargetScreenArea(getScreenBounds())
                           .withPreferredPopupDirection(PopupMenu::Options::PopupDirection::upwards),
        [this](int action)
        {
            if (action == 0)
            {
                menu.dismissAllActiveMenus();

            }
            else if (action >= 1)
            {
                /*
                    TODO: clean this up
                    array of file paths starts at 0, but menu index starts at 1 (0 = close menu)
                */
                int index = action - 1;
                auto filePath = getFilePathFromIndex(index);
                audioProcessor.loadPreset(filePath);
                showMenu();
            }
    
        });

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
        /*
            Doesnt' like when I put "Macintosh HD" in front
        */
        auto presetFolder = juce::File("/Library/Application Support/Recluse-Audio/Pulsar/Presets").findChildFiles(2, true);

        for (int i = 0; i < presetFolder.size(); i++)
        {
            filePaths.add(presetFolder[i].getFullPathName());
            presetNames.add(presetFolder[i].getFileNameWithoutExtension());
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
