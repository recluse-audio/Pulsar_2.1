/*
  ==============================================================================

    TempoSync.cpp
    Created: 22 Oct 2020 7:44:03am
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#include "TempoSync.h"

TempoSync::TempoSync()
{
    setTempo(120, 4, 4);
}

TempoSync::~TempoSync(){}

void TempoSync::setTempo(double tempo, int timeSigNum, int timeSigDenom)
{
    bpm = tempo;
    quarterNote = 60000 / bpm; // 60 bpm, quarter=1000, 
    wholeNote = quarterNote * 4; // 
    halfNote = quarterNote * 2;
    eighthNote = quarterNote / 2;
    sixteenthNote = quarterNote / 4;
}

float TempoSync::asTriplet(float rhythmicValue)
{
    auto triplet = (2.f * rhythmicValue) / 3.f;
    return triplet;
}

juce::String TempoSync::getSyncString(float sliderValue)
{

    if (sliderValue <= 40)
        return "1/16";

    if (sliderValue > 400 && sliderValue <= 800)
        return "1/8";

    if (sliderValue > 800 && sliderValue <= 120)
        return "1/4";

    if (sliderValue > 1200 && sliderValue <= 1600)
        return "1/2";

    if (sliderValue > 1600 && sliderValue <= 2000)
        return "1/1";

}

float TempoSync::getSynchronizedFromMS(float ms)
{
    if (ms <= 40)
        return sixteenthNote;

    if (ms > 40 && ms <= 800)
        return eighthNote;

    if (ms > 800 && ms <= 1200)
        return quarterNote;

    if (ms > 1200 && ms <= 1600)
        return halfNote;
//
    if (ms > 1600) // max delay is one whole no
        return wholeNote;
}

// just normalizing the fund range to fit this grid, not really an accurate representation more gui enabling
float TempoSync::getSynchronizedFreq(float freq)
{
    // whole note is in ms, must return freq that corresponds with a period of this manty samples

    if (freq <= 1)
        return 1000.f / wholeNote;

    if (freq > 1 && freq <= 10)
        return 1000.f / halfNote;

    if (freq > 10 && freq <= 20)
        return 1000.f / asTriplet(halfNote);

    if (freq > 20 && freq <= 40)
        return 1000.f / quarterNote;

    if (freq > 40 && freq <= 60)
        return 1000.f / asTriplet(quarterNote);

    if (freq > 60 && freq <= 80)
        return 1000.f / eighthNote;

    if (freq > 80 && freq <= 100)
        return 1000.f / asTriplet(eighthNote);
    //
    if (freq > 100 && freq <= 120)
        return 1000.f / sixteenthNote;

    if (freq > 100 && freq <= 120)
        return 1000.f / asTriplet(sixteenthNote);

    if (freq > 120 && freq <= 140)
        return 1000.f / (sixteenthNote * 0.5f);

    if (freq > 140 && freq <= 200)
        return 1000.f / asTriplet(sixteenthNote * 0.5f);

    return 1.0f;
}



