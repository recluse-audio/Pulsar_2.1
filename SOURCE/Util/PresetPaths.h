#pragma once

#include <JuceHeader.h>

namespace Pulsar
{
    inline juce::File pulsarPresetsRoot()
    {
        return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                   .getChildFile("recluse-audio")
                   .getChildFile("Pulsar")
                   .getChildFile("Presets");
    }

    inline juce::File factoryPresetDir()
    {
        auto dir = pulsarPresetsRoot().getChildFile("Factory");
        if (!dir.exists())
            dir.createDirectory();
        return dir;
    }

    inline juce::File userPresetDir()
    {
        auto dir = pulsarPresetsRoot().getChildFile("User");
        if (!dir.exists())
            dir.createDirectory();
        return dir;
    }
}
