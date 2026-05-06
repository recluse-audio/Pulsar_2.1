
/*
 ==============================================================================
 
 LookAndFeel.cpp
 Created: 12 Jul 2020 8:49:21am
 Author:  Billie (Govnah) Jean
 
 ==============================================================================
 */

#include "MixFeel.h"

MixFeel::MixFeel()
{
    setColour(juce::Slider::backgroundColourId, Colours::grey);
    setColour(juce::Slider::trackColourId, Colours::darkred);
    setColour(juce::TextButton::textColourOnId, Colours::antiquewhite);
    setColour(juce::TextButton::textColourOffId, Colours::antiquewhite);
    setColour(juce::TextButton::buttonColourId, Colours::black);
    setColour(juce::TextButton::buttonOnColourId, Colours::black);
    
}

MixFeel::~MixFeel()
{
    
}



/*=================================================================================*/
/*----------------------------- drawRotarySlider ----------------------------------*/
/*=================================================================================*/
void MixFeel::drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle, Slider& slider)
{
    
    if(slider.getSliderStyle() == Slider::SliderStyle::LinearBarVertical)
    {
        
        juce::Rectangle<float> bgFrame(x, y, width, height);
        g.setColour(juce::Colours::whitesmoke);
        g.drawRoundedRectangle(bgFrame, 1.f, 2.f);

        g.setColour(juce::Colours::transparentBlack.withAlpha(0.f));
        g.fillAll();

       
        juce::Rectangle<float> centerRect(x, sliderPos, width, 2);
        g.setColour(juce::Colours::orange);
        g.fillRoundedRectangle(centerRect, 1.f);

    }

    if(slider.getSliderStyle() == Slider::SliderStyle::LinearVertical)
    {
        juce::Rectangle<int> bgRect;
        bgRect.setX(x);
        bgRect.setY(y);
        bgRect.setSize(width, height);
        g.setColour(juce::Colours::whitesmoke);
        g.drawRoundedRectangle(bgRect.toFloat(), 1.f, 1.f);
        
        
        juce::Rectangle<int> centerTick(x, height/2, width, 2);
        
        
        auto pos = sliderPos;
        if (pos >= height - 3)
            pos = height - 3;
        
        juce::Rectangle<int> thumb(0, pos, width, 2);
        g.setColour(juce::Colours::yellow);
        g.fillRoundedRectangle(thumb.toFloat(), 5.f);
        
        
        g.setColour(juce::Colours::whitesmoke.darker());
        g.fillRoundedRectangle(centerTick.toFloat(), 3.f);
    }

    
    if (slider.getSliderStyle() == Slider::SliderStyle::LinearHorizontal)
    {
        juce::Rectangle<int> bgRect;
        bgRect.setX(x);
        bgRect.setY(y);
        bgRect.setSize(width, height);
        g.setColour(juce::Colours::whitesmoke);
        g.drawRoundedRectangle(bgRect.toFloat(), 1.f, 1.f);

        juce::Rectangle<int> centerTick(width/2, y, 1, height);
        
        g.setColour(juce::Colours::transparentBlack.withAlpha(0.f));
        g.fillAll();
        
        auto pos = sliderPos;
        if (pos >= width - 3)
            pos = width - 3;
        
        juce::Rectangle<int> thumb(pos, 0, 2, 20);
        g.setColour(juce::Colours::yellow);
        g.fillRoundedRectangle(thumb.toFloat(), 2.f);
        
        g.setColour(juce::Colours::whitesmoke.darker());
        g.fillRoundedRectangle(centerTick.toFloat(), 2.f);
    }
    
    if(slider.getSliderStyle() == Slider::ThreeValueVertical)
    {
        juce::Rectangle<float> bgFrame(x, y, width, height);
        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(bgFrame, 1.f, 2.f);
        
        float h = jlimit<float>(1, height, minSliderPos - maxSliderPos);
        float y = jlimit<float>(2, height - 2, maxSliderPos);
        juce::Rectangle<float> fillRect(x, y, width, h);

        auto color = slider.findColour(Slider::backgroundColourId);
        juce::ColourGradient gradient(color.darker(), width / 2, height, color.withHue(0.5f), width / 2, 0, false);

        g.setGradientFill(gradient);
        g.fillRect(fillRect);
    }
    
    if(slider.getSliderStyle() == Slider::ThreeValueHorizontal)
    {
        juce::Rectangle<float> bgFrame(x, y, width, height);
        g.setColour(juce::Colours::whitesmoke);
        g.drawRoundedRectangle(bgFrame, 1.f, 2.f);
        
        juce::Rectangle<float> fillRect(minSliderPos + 2, y, maxSliderPos-minSliderPos, height);

        auto color = slider.findColour(Slider::backgroundColourId);
        juce::ColourGradient gradient(color.darker(), 0, height/2, color.withHue(0.5f), width, height/2, false);
        g.setGradientFill(gradient);
        g.fillRect(fillRect);
    }
    
    
}

juce::Slider::SliderLayout MixFeel::getSliderLayout(Slider& slider)
{
    Slider::SliderLayout layout;
    auto bounds = slider.getLocalBounds();
    
    juce::Rectangle<int> textBox(0, bounds.getHeight()*0.7, bounds.getWidth(), 15);
    
    layout.textBoxBounds = textBox;
    layout.sliderBounds  = bounds;

    
    return layout;
}


Label* MixFeel::createSliderTextBox(Slider& slider)
{
    
    Label* l = LookAndFeel_V4::createSliderTextBox(slider);
    // make sure editor text is black (so it shows on white background)
    l->setColour(juce::Label::outlineColourId, juce::Colours::white.withAlpha(0.0f));
    l->setColour(juce::Slider::textBoxBackgroundColourId, Colours::hotpink.withAlpha(0.f));
    l->setColour(juce::Label::textColourId, Colours::white);

    
    return l;
}

/*=================================== FONT =========================================*/

Font MixFeel::getLabelFont(Label& label)
{
   return getFont();
    
}

void MixFeel::drawButtonBackground(Graphics& g, Button& button, const Colour& backgroundColour,
                                    bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    juce::Rectangle<int> bounds = button.getLocalBounds();
    
    if(button.isDown())
    {
        g.setColour(button.findColour(TextButton::buttonOnColourId).withAlpha(0.f));
        g.fillRoundedRectangle(bounds.toFloat(), 5.f);
        
    } else
    {
        g.setColour(button.findColour(TextButton::buttonColourId).withAlpha(0.f));
        g.fillRoundedRectangle(bounds.toFloat(), 5.f);
    }
    
    
}







