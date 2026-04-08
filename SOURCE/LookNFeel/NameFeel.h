/*
  ==============================================================================

    NameFeel.h
    Created: 16 Dec 2020 10:47:18am
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class  NameFeel : public juce::LookAndFeel_V4
{
public:
    NameFeel();
    ~NameFeel();
    
    Font getLabelFont(Label&) override;


private:
    
    Font getFont()
    {
        return Font ("Consolas", "Regular", 27.f);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NameFeel)
};
