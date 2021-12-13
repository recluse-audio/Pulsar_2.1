/*
  ==============================================================================

    Pulsaret.h
    Created: 5 Nov 2020 12:01:10pm
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PulsaretTable.h"


class Pulsaret
{
public:
    Pulsaret ();
    virtual ~Pulsaret();
    
    void prepare(double mSampleRate);
    void setTable(int selection);
    void setFrequency (float frequency);
    juce::AudioBuffer<float>& getTable();
    virtual float getNextSample() noexcept;
    virtual void resetPhase();
    
    virtual void setRunning();
    bool isFree = true;
    
protected:
    PulsaretTable pulsaretTable;
    double mSampleRate;

    float currentIndex = 0.0f;
    float tableDelta = 0.0f;
    int tableSize = 1024;
};
