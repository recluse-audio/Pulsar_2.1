/*
  ==============================================================================

    EnvelopeControlPoint.h
    Created: 7 Nov 2020 9:09:01am
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
template <typename ValueType>
class EnvelopeControlPoint : public Rectangle<ValueType>
{
public:
    EnvelopeControlPoint() = default;
    ~EnvelopeControlPoint() = default;

    void setSelected()
    {
        selected = true;
    }
    
    void setUnselected()
    {
        selected = false;
    }
    bool isSelected()
    {
        return selected;
    }
    
    bool containsWithinSection(juce::Point<float> point)
    {
        auto x1 = this->getCentreX() - 15;
        auto x2 = this->getCentreX() + 15;
        if (point.getX() > x1 && point.getX() < x2)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    
private:
    //juce::Rectangle<float> bounds;
    bool selected = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeControlPoint)
};
