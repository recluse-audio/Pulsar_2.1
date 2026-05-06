/*
  ==============================================================================

    OwnedPulsaret.cpp
    Created: 21 Nov 2020 8:18:28pm
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#include "OwnedPulsaret.h"

OwnedPulsaret::OwnedPulsaret()
{
    setLengthInSamples(1024);
    pulsaretTable.createTables();
    tableSize = 1024;
    
    env.setTableSize(tableSize);
    penv.setTableSize(tableSize);

}

OwnedPulsaret::~OwnedPulsaret()
{
    
}

/*=========================================================*/
void OwnedPulsaret::prepare(double sampleRate)
{
    mSampleRate = sampleRate;
    env.prepare(sampleRate);
    penv.prepare(sampleRate);
    
}




/*=========================================================*/
void OwnedPulsaret::setFrequency (float frequency) // for setting externally from pulsar train
{
    freq = frequency;
}

void OwnedPulsaret::calculateDeltas(float freq) // for changing based on pulsaret's pitch trajectory
{
    auto tableSizeOverSampleRate = (float) tableSize / mSampleRate;
    tableDelta = freq * tableSizeOverSampleRate;
    env.setFrequency(freq);
    penv.setFrequency(freq);
}



float OwnedPulsaret::getNextSample() noexcept
{
    auto index0 = (unsigned int) currentIndex;
    auto index1 = index0 + 1;
    
    auto frac = currentIndex - (float) index0;
    
    auto* table1 = pulsaretTable.getTable().getReadPointer (0);
    auto value0 = table1[index0];
    auto value1 = table1[index1];
    
    auto* table2 = pulsaretTable.getNextTable().getReadPointer (0);
    auto value2 = table2[index0];
    auto value3 = table2[index1];
    
    auto waveSample1 = value0 + (frac * (value1 - value0));
    waveSample1 *= 1.f - pulsaretTable.getWaveFraction(); // pulsaretTable.getWaveFraction() should equal 0.f if no interp should occur
    
    
    auto waveSample2 = value2 + (frac * (value3 - value2));
    waveSample2 *= pulsaretTable.getWaveFraction();
    
    auto currentSample = waveSample1 + waveSample2;

    currentIndex += tableDelta;
    pRatio = 1 + penv.getNextSample(); // penv starts at 0.f so...
    calculateDeltas(freq * pRatio); // changing delta for next sample
    
    if (currentIndex > (float) tableSize)
    {
        currentIndex = 0;
        cycles--;
        if (cycles <= 0)
        {
            isFree = true;
            cycles = numCycles;
        }
    }
       
    if (trigger)
    {
        return currentSample;
    }
    else
    {
        return 0.f;
    }
    
}



void OwnedPulsaret::setLengthInSamples(float numSamples)
{
    cycleSamples = numSamples;
    float pulsaretPeriod = (1/freq) * mSampleRate;
    cycles = cycleSamples / pulsaretPeriod;
 
    if(cycles <= 1)
    {
        cycles = 1;
    }
    
    numCycles = cycles;
}

void OwnedPulsaret::setRunning()
{
    setAsHit();
    isFree = false;
}

void OwnedPulsaret::resetPhase()
{
    currentIndex = 0;
}
juce::AudioBuffer<float>& OwnedPulsaret::getEnv()
{
    return env.getTable();
}

void OwnedPulsaret::setContinuous(bool test)
{

    
}

void OwnedPulsaret::setAsHit()
{
    trigger = true;
}

void OwnedPulsaret::setAsMiss()
{
    trigger = false;
}

PulsaretTable& OwnedPulsaret::getPulsaretTable()
{
    return pulsaretTable;
}

bool OwnedPulsaret::checkIfFree()
{
    return isFree;
}

void OwnedPulsaret::setPulsaretStatus(bool pulsaretStatus)
{
    isFree = pulsaretStatus;
}