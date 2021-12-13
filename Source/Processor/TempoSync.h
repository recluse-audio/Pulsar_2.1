/*
  ==============================================================================

    TempoSync.h
    Created: 22 Oct 2020 7:44:02am
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class TempoSync
{
public:
    TempoSync();
    ~TempoSync();
    
    void setTempo(double tempo, int timeSigNum, int timeSigDenom);
    
    /* returns a triplet version of said rhythmic value.  Should always be shorter*/
    float asTriplet(float rhythmicValue);

    juce::String getSyncString(float sliderValue);
    
    float getSynchronizedFromMS(float period);

    float getSynchronizedFreq(float freq);
    
    
    
private:
    float wholeNote = 1000, halfNote = 500, quarterNote = 250, eighthNote = 125, sixteenthNote = 62.5;
    double bpm = 120;
};

