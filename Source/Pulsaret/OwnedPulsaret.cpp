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
    //setLengthInSamples(table.getTableSize());
    //pulsaretTable.createTables();
    table.loadTablesFromDisk();
    tableSize = table.getTableSize();
    env.setTableSize(table.getTableSize());

}

OwnedPulsaret::~OwnedPulsaret()
{
    
}

/*=========================================================*/
void OwnedPulsaret::prepare(double sampleRate)
{
    mSampleRate = sampleRate;
    env.prepare(sampleRate);
}


/*=========================================================*/
void OwnedPulsaret::setFrequency (float frequency) // for setting externally from pulsar train
{
    freq = frequency;
    calculateDeltas(freq);
}

void OwnedPulsaret::calculateDeltas(float freq) // for changing based on pulsaret's pitch trajectory
{
    auto tableSizeOverSampleRate = (float) tableSize / mSampleRate;
    tableDelta = freq * tableSizeOverSampleRate;
    env.setFrequency(freq);
}



float OwnedPulsaret::getNextSample() noexcept
{
    auto index0 = (unsigned int) currentIndex;
    auto index1 = index0 + 1;
    
    auto frac = currentIndex - (float) index0;
    
    
    //auto* table1 = pulsaretTable.getTable().getReadPointer (0);
    //auto value0 = table1[index0];
    //auto value1 = table1[index1];
    //
    //auto* table2 = pulsaretTable.getNextTable().getReadPointer (0);
    //auto value2 = table2[index0];
    //auto value3 = table2[index1];

    //auto waveSample1 = value0 + (frac * (value1 - value0));
    //    waveSample1 *= 1.f - pulsaretTable.getWaveFraction(); // pulsaretTable.getWaveFraction() should equal 0.f if no interp should occur
    //
    //
    //auto waveSample2 = value2 + (frac * (value3 - value2));
    //waveSample2 *= pulsaretTable.getWaveFraction();
    //
    //auto currentSample = waveSample1 + waveSample2;

    //auto tableRead = table.getTable().getReadPointer(0);
    //auto value0 = tableRead[index0];
    //auto value1 = tableRead[index1];

    //auto currentSample = value0 + (frac * (value1 - value0));

    auto* table1 = table.getTable0().getReadPointer(0);
    auto value0 = table1[index0];
    auto value1 = table1[index1];
 
    auto* table2 = table.getTable1().getReadPointer(0);
    auto value2 = table2[index0];
    auto value3 = table2[index1];

    auto waveSample1 = value0 + (frac * (value1 - value0));
    waveSample1 *= 1.f - table.getWaveFraction(); // pulsaretTable.getWaveFraction() should equal 0.f if no interp should occur


    auto waveSample2 = value2 + (frac * (value3 - value2));
    waveSample2 *= table.getWaveFraction();

    auto currentSample = waveSample1 + waveSample2;
    currentSample *= env.getNextSample();

    currentIndex += tableDelta;
    
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

void OwnedPulsaret::setWaveType(float wave)
{
    table.setTable(wave);
}

PulsaretTable& OwnedPulsaret::getPulsaretTable()
{
    return pulsaretTable;
}

Table& OwnedPulsaret::getTable()
{
    return table;
}

bool OwnedPulsaret::checkIfFree()
{
    return isFree;
}

void OwnedPulsaret::setPulsaretStatus(bool pulsaretStatus)
{
    isFree = pulsaretStatus;
}