/*
  ==============================================================================

    PulsaretFactory.h
    Created: 13 May 2021 8:12:19am
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "OwnedPulsaret.h"
#include "MidiTransformer.h"

/*
    A class built to handle the many calculations of Pulsaret Parameters.  Many of the lengthier functions
    are in pursuit of stochasticism Range/Rand 
*/
class PulsaretFactory
{
public:
    PulsaretFactory();
    ~PulsaretFactory();

    void doParameterChanged(const juce::String& parameterID, float newValue);

    void prepare(double sampleRate);

    void calculatePulsaret(OwnedPulsaret& p1, OwnedPulsaret& p2);
    void calculateFreq(OwnedPulsaret& p1, OwnedPulsaret& p2);
    void calculateWave(OwnedPulsaret& p1, OwnedPulsaret& p2);
    void calculateRanges(OwnedPulsaret& p1, OwnedPulsaret& p2);

    void setGlideTime(float seconds);

    void calculateDutyCycle(float width);

    void passPulsarPeriod(float pulsarPeriod);

    void nextSmoothValue();

    float getHarmonicFormant(float formFreq);
    int getTransformedMidi(int midiNumber);

    
private:
    MidiTransformer midiTransformer;
    MidiTransformer midiTransformer2;

    const double frequencyOfA;
    double mSampleRate = -1;

    float pulsaretSamples1 = 1.f;
    float pulsaretSamples2 = 1.f;
    float pulsarPeriodInSamples = 1024;

    Atomic<float> mQueuedFormant1  { 600.0f };
    std::atomic<bool> formant1Changed { false };
    Atomic<float> mFormFreq{ 600.0f };
    Atomic<float> mFormSpread{ 0.0f };
    Atomic<float> mFormRand{ 0.05 };
    Atomic<int> mFormKey{ 0 };
    Atomic<int> mFormScale{ 0 };

    juce::Random randFreq1;
    Atomic<bool> formIsSpread{ false };
    Atomic<bool> formIsHarm{ false };

    juce::Range<int>   formRange;
    juce::SmoothedValue<float, ValueSmoothingTypes::Linear> smoothForm;

    Atomic<float> mQueuedFormant2  { 800.0f };
    std::atomic<bool> formant2Changed { false };
    Atomic<float> mFormFreq2{ 800.0f };
    Atomic<float> mFormSpread2{ 0.f };
    Atomic<float> mFormRand2{ 0.05 };
    Atomic<int> mFormKey2{ 0 };
    Atomic<int> mFormScale2{ 0 };


    juce::Random randFreq2;
    Atomic<bool> formIsSpread2{ false };
    Atomic<bool> formIsHarm2{ false };

    juce::Range<int>   formRange2;
    juce::SmoothedValue<float, ValueSmoothingTypes::Linear> smoothForm2;

    Atomic<float> waveType{ 0.f };
    Atomic<float> waveSpread{ 0.f };
    Atomic<float> waveRand{ 0.f }; // number that must be surpassed to be triggered
    juce::Random randWave; // number that is measured against 
    Atomic<bool> waveIsSpread{ false };
    juce::Range<int> waveRange;


    Atomic<float> width{ 0.f };
    
};
