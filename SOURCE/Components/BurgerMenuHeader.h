/*
  ==============================================================================

    BurgerMenuComponent.h
    Created: 10 Aug 2021 6:48:41pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class BurgerMenuHeader  : public juce::Component
{
public:
    BurgerMenuHeader(SidePanel& sp) : sidePanel(sp)
    {
        static const unsigned char burgerMenuPathData[]
            = { 110,109,0,0,128,64,0,0,32,65,108,0,0,224,65,0,0,32,65,98,254,212,232,65,0,0,32,65,0,0,240,65,252,
                169,17,65,0,0,240,65,0,0,0,65,98,0,0,240,65,8,172,220,64,254,212,232,65,0,0,192,64,0,0,224,65,0,0,
                192,64,108,0,0,128,64,0,0,192,64,98,16,88,57,64,0,0,192,64,0,0,0,64,8,172,220,64,0,0,0,64,0,0,0,65,
                98,0,0,0,64,252,169,17,65,16,88,57,64,0,0,32,65,0,0,128,64,0,0,32,65,99,109,0,0,224,65,0,0,96,65,108,
                0,0,128,64,0,0,96,65,98,16,88,57,64,0,0,96,65,0,0,0,64,4,86,110,65,0,0,0,64,0,0,128,65,98,0,0,0,64,
                254,212,136,65,16,88,57,64,0,0,144,65,0,0,128,64,0,0,144,65,108,0,0,224,65,0,0,144,65,98,254,212,232,
                65,0,0,144,65,0,0,240,65,254,212,136,65,0,0,240,65,0,0,128,65,98,0,0,240,65,4,86,110,65,254,212,232,
                65,0,0,96,65,0,0,224,65,0,0,96,65,99,109,0,0,224,65,0,0,176,65,108,0,0,128,64,0,0,176,65,98,16,88,57,
                64,0,0,176,65,0,0,0,64,2,43,183,65,0,0,0,64,0,0,192,65,98,0,0,0,64,254,212,200,65,16,88,57,64,0,0,208,
                65,0,0,128,64,0,0,208,65,108,0,0,224,65,0,0,208,65,98,254,212,232,65,0,0,208,65,0,0,240,65,254,212,
                200,65,0,0,240,65,0,0,192,65,98,0,0,240,65,2,43,183,65,254,212,232,65,0,0,176,65,0,0,224,65,0,0,176,
                65,99,101,0,0 };

        Path p;
        p.loadPathFromData(burgerMenuPathData, sizeof(burgerMenuPathData));
        burgerButton.setShape(p, true, true, false);

        burgerButton.onClick = [this] { showOrHide(); };
        addAndMakeVisible(burgerButton);
    }

    ~BurgerMenuHeader() override
    {
        sidePanel.showOrHide(false);
    }

private:

    void paint (juce::Graphics& g) override
    {
        g.setColour(juce::Colours::black);
        g.fillAll();  

        g.setColour(juce::Colours::ghostwhite.withAlpha(0.5f));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 2.f, 1.f);
    }

    void resized() override
    {
        auto r = getLocalBounds();

        burgerButton.setBounds(r.removeFromRight(40).withSizeKeepingCentre(20, 20));
        titleLabel.setFont(Font((float)getHeight() * 0.5f, Font::plain));
        titleLabel.setBounds(r);
    }

    void showOrHide()
    {
        sidePanel.showOrHide(!sidePanel.isPanelShowing());
    }

    Label titleLabel{ "titleLabel", "JUCE Demo" };

    SidePanel& sidePanel;
    ShapeButton burgerButton{ "burgerButton", Colours::lightgrey, Colours::lightgrey, Colours::white };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BurgerMenuHeader)
};
