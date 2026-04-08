/*
  ==============================================================================

    MixFeel.h
    Created: 15 Dec 2020 12:26:27pm
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class  MixFeel : public juce::LookAndFeel_V4
{
public:
    MixFeel();
    ~MixFeel();
    
    void drawLinearSlider (Graphics&,
                           int x, int y, int width, int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const Slider::SliderStyle,
                           Slider&) override;
    
    juce::Slider::SliderLayout getSliderLayout(Slider&) override;
    Label* createSliderTextBox(Slider&) override;
    Font getLabelFont(Label&) override;
    
    void drawButtonBackground(Graphics& g, Button& button, const Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    
private:
    Font getFont()
    {
        return Font ("Consolas", "Regular", 10.f);
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixFeel)
};

