/*
  ==============================================================================

    OwnedPulsaret.h
    Created: 21 Nov 2020 8:18:28pm
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#pragma once

#include "Pulsaret.h"
#include "Envelope.h"
#include "PulsaretTable.h"

/*
    A sub-class of Pulsaret for the pulsarets in the array owned by the pulsar train.
*/
class OwnedPulsaret : public Pulsaret
{
public:
    OwnedPulsaret();
    ~OwnedPulsaret();
    
    void prepare(double mSampleRate);
    void setTable(int selection);
    void setFrequency (float frequency);
    void calculateDeltas(float freq);
    
    void setRunning() override;
    
    
    float getNextSample() noexcept;
    
    juce::AudioBuffer<float> getBuffer();
    
    void resetPhase() override;

    void setLengthInSamples(float numSamples);
    void isSingleCycle(bool is);
    void setContinuous(bool test);
    void setAsHit(); 
    void setAsMiss();
    
    void setWaveType(float wave)
    {
        auto normalized = wave / 100.f;
        pulsaretTable.setTable(normalized);
    }
    
    juce::AudioBuffer<float>& getEnv();

    PulsaretTable& getPulsaretTable();
    
    PulsaretTable pulsaretTable;
    
    bool checkIfFree();
    void setPulsaretStatus(bool pulsaretStatus);
private:
    float pRatio = 2.f; // playback or pitchRatio.  Changing coefficient between 0.5 and 2.0 that will multiply freq or delta
    float freq;
    bool trigger = true; // affected by formFreq
    bool isFree = true;

    double mSampleRate = 0;
    Envelope env;
    Envelope penv;

    int pulsarPeriodInSamples;
    float currentIndex = 0.0f;
    float tableDelta = 0.0f;
    float cycleSamples = 1;
    float cycleSamplesLeft = 1;

    int cycles = 1;
    int numCycles = 1;
    
    int tableSize = 1024;
   
};
