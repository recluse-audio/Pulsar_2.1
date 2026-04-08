/*
  ==============================================================================

    OwnedPulsaret.h
    Created: 21 Nov 2020 8:18:28pm
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#pragma once

#include "Pulsaret.h"
#include "../Envelope/Envelope.h"
#include "PulsaretTable.h"
#include "Table.h"

/*
    A sub-class of Pulsaret for the pulsarets in the array owned by the pulsar train.
*/
class OwnedPulsaret : public Pulsaret
{
public:
    OwnedPulsaret();
    ~OwnedPulsaret();
    
    void prepare(double mSampleRate);
    void setFrequency (float frequency);
    void calculateDeltas(float freq);
    
    void setRunning() override;
    
    float getNextSample() noexcept;
    
    void resetPhase() override;

    void setLengthInSamples(float numSamples);

    void setContinuous(bool test);
    void setAsHit(); 
    void setAsMiss();
    
    void setWaveType(float wave);
    
    juce::AudioBuffer<float>& getEnv();

    PulsaretTable& getPulsaretTable();
    PulsaretTable pulsaretTable;

    Table& getTable();
    Table table;

    bool checkIfFree();
    void setPulsaretStatus(bool pulsaretStatus);
private:
    float freq;
    bool trigger = true; // affected by formFreq
    bool isFree = true;

    double mSampleRate = 0;
    Envelope env;

    int pulsarPeriodInSamples;
    float currentIndex = 0.0f;
    float tableDelta = 0.0f;
    float cycleSamples = 1;
    float cycleSamplesLeft = 1;

    int cycles = 1;
    int numCycles = 1;
    
    int tableSize = 2048;
   
};
