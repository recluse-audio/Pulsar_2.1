/*
  ==============================================================================

    Database.h
    Created: 10 Aug 2021 3:20:08pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#pragma once
#include <JuceHeader.h>
// This class is for intializing each artist and their romples.  This holds the array of file paths
// I wish I could get this to store file paths by artist, but returnign the selected index id from 
// the popup menu is simpler than searching for a key/value via the string of the menu seleciton
// See:  Rompler.h
class Database
{
public:
    Database() { loadFiles(); }
    ~Database() {}

    void loadFiles()
    {
        auto folders = juce::File::getCurrentWorkingDirectory().getSiblingFile("Resources").findChildFiles(juce::File::findDirectories, true);

        for (int i = 0; i < folders.size(); i++)
        {

            //auto artist = new Artist(folders[i].getFileName());

            auto files = folders[i].findChildFiles(juce::File::findFiles, true);

            for (int j = 0; j < files.size(); j++)
            {
               // filePaths.add(romples[j].getFullPathName());
               // artist->addRomple(romples[j].getFileNameWithoutExtension());
            }
            //artists.add(artist);
        }
    }

    juce::String& getFilePathFromIndex(int index) { return filePaths.getReference(index); }


private:
   // juce::OwnedArray<Artist> artists;
    juce::StringArray filePaths;
    friend class RompMenu;
};