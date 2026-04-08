/*
  ==============================================================================

    Pulsaret.cpp
    Created: 5 Nov 2020 12:01:10pm
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#include "Pulsaret.h"
/*
    Maybe make a sub-class for the owned array of these, and keep this as a base class.  The base class would
    contain functions like prepare(),
*/
Pulsaret::Pulsaret()
{
    
}

Pulsaret::~Pulsaret()
{
    
}


/*=========================================================*/
void Pulsaret::prepare(double sampleRate)
{
    mSampleRate = sampleRate;
}

/*=========================================================*/
void Pulsaret::setFrequency (float frequency)
{
    auto tableSizeOverSampleRate = (float) tableSize / mSampleRate;
    tableDelta = frequency * tableSizeOverSampleRate;
}

/*=========================================================*/
void Pulsaret::setRunning()
{
    isFree = false;
}

/*=========================================================*/
float Pulsaret::getNextSample() noexcept
{
    auto index0 = (unsigned int) currentIndex;
    auto index1 = index0 + 1;
    
    auto frac = currentIndex - (float) index0;
    
    auto* table = pulsaretTable.getTable().getReadPointer (0);
    auto value0 = table[index0];
    auto value1 = table[index1];
    
    auto currentSample = value0 + (frac * (value1 - value0));
    
    if ((currentIndex += tableDelta) > (float) tableSize)
    {
        isFree = true;
        currentIndex = 0;
        //moveToBack(*this);
    }
        
        
        return currentSample;
}




void Pulsaret::resetPhase()
{
    currentIndex = 0;
}

juce::AudioBuffer<float>& Pulsaret::getTable()
{
    return pulsaretTable.getTable();
}



