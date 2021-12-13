/*
  ==============================================================================

    PulsaretTable.cpp
    Created: 6 Nov 2020 4:34:52pm
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#include "PulsaretTable.h"

PulsaretTable::PulsaretTable()
: sineTable(1, 1024), sincTable(1, 1024), triTable(1, 1024), sawTable(1, 1024)
{
    createSineTable();
    createSincTable();
    createTriTable();
    createSawTable();
    
//    createTables();
    //tableArray.add(&sineTable);
//    tableArray.add(&sincTable);
//    tableArray.add(&triTable);
//    tableArray.add(&sawTable);
    //tableArray.add(&sineTable);
    
}

PulsaretTable::~PulsaretTable()
{
    
}

void PulsaretTable::createTables()
{
    createSineTable();
    createSincTable();
    createTriTable();
    createSawTable();
    
    
}

void PulsaretTable::setTable(float selection)
{
    
    normalWaveIndex = selection; // 0  to 1 version of waveIndex
    floatWaveIndex = NormalisableRange<float>(0.f, 3.f, 0.001f, 1.f).convertFrom0to1(selection); // converted to size of array
    intWaveIndex = floatWaveIndex.get(); // cast to int
    waveFraction = floatWaveIndex.get() - intWaveIndex.get(); // overlap
    
    
}

/*
    I'd love to get these next three as one function, but I don't know how to return 3 values
    Maybe sample calc should happen in the pulsaret table rather than the pulsaret itself?
*/
juce::AudioBuffer<float>& PulsaretTable::getTable()
{
    switch(intWaveIndex.get())
    {
        case 0:  return sineTable;
            break;
        case 1: return triTable;
            break;
        case 2: return sawTable;
            break;
        case 3: return sincTable;
            break;
        default: return sineTable;
            break;
    }
    //return *tableArray[intWaveIndex.get()];
}

juce::AudioBuffer<float>& PulsaretTable::getNextTable()
{
    switch(intWaveIndex.get() + 1)
    {
        case 0:  return sineTable;
            break;
        case 1: return triTable;
            break;
        case 2: return sawTable;
            break;
        case 3: return sincTable;
            break;
        case 4: return sineTable;
            break;
            
        default: return sineTable;
            break;
    }
    
}

float PulsaretTable::getWaveFraction()
{
    return waveFraction.get();
}

void PulsaretTable::setEnvLevel(float envLevel)
{
    
    intWaveIndex = envLevel;
    floatWaveIndex = envLevel;
    waveFraction = floatWaveIndex.get() - intWaveIndex.get();
    
//    if(intWaveIndex.get() > tableArray.size())
//        intWaveIndex = tableArray.size() - 1;
}

float PulsaretTable::getWaveIndex()
{
    return floatWaveIndex.get();
}
/*------------------------------------------------------*/

void PulsaretTable::createSineTable()
{
    
    sineTable.setSize (1, (int) tableSize + 1);
    sineTable.clear();

    auto* buffWrite = sineTable.getWritePointer (0);

    auto angleDelta = juce::MathConstants<double>::twoPi / (double) (tableSize - 1);
    auto pi = juce::MathConstants<double>::pi;
    double currentAngle = -pi;
    
    for (unsigned int i = 0; i < tableSize; ++i)
    {
        float sample;
        sample = std::sin(currentAngle);
        
        
        buffWrite[i] += sample;
        currentAngle += angleDelta;
    }
        

    buffWrite[tableSize] = buffWrite[0];
}


/*=========================================================*/
void PulsaretTable::createSincTable()
{
    sincTable.setSize (1, (int) tableSize + 1);
    sincTable.clear();

    auto* buffWrite = sincTable.getWritePointer (0);
    
    auto angleDelta = juce::MathConstants<double>::twoPi / (double) (tableSize - 1);
    auto pi = juce::MathConstants<double>::pi;
    double currentAngle = -pi;
    
    for (unsigned int i = 0; i < tableSize; ++i)
    {
        float sample;
        
        if (currentAngle == 0)
        {
            sample = 1;
        }
        else
        {
            sample = std::sin (pi * currentAngle) / (pi * currentAngle);
            buffWrite[i] += sample;
            
        }
        
        currentAngle += angleDelta;
    }
    
    buffWrite[tableSize] = buffWrite[0];
}

void PulsaretTable::createTriTable()
{
    triTable.setSize (1, (int) tableSize + 1);
    triTable.clear();
    auto* buffWrite = triTable.getWritePointer (0);

    
    int harmonics[] = { 1, 3, 5, 7, 9, 11, 13, 15 };
        
    for (auto harmonic = 0; harmonic < juce::numElementsInArray (harmonics); harmonic += 2)
    {
        auto angleDelta = juce::MathConstants<double>::twoPi / (double) (tableSize - 1) * harmonics[harmonic];
        auto currentAngle = 0.0;
        
        float harmonicAmp = 0.9f / (harmonics[harmonic] * harmonics[harmonic]);
       
        for (unsigned int i = 0; i < tableSize; ++i)
        {
            auto sample = std::sin (currentAngle);
            buffWrite[i] += (float) sample * harmonicAmp; //harmonicWeights[harmonic];
            currentAngle += angleDelta;
        }
    }
    
    for (auto harmonic = 1; harmonic < juce::numElementsInArray (harmonics); harmonic += 2)
    {
        auto angleDelta = juce::MathConstants<double>::twoPi / (double) (tableSize - 1) * harmonics[harmonic];
        auto pi = juce::MathConstants<double>::pi;
        auto currentAngle = pi;
        
        float harmonicAmp = 0.9f / (harmonics[harmonic] * harmonics[harmonic]);
        
        for (unsigned int i = 0; i < tableSize; ++i)
        {
            auto sample = std::sin (currentAngle);
            buffWrite[i] += (float) sample * harmonicAmp;
            currentAngle += angleDelta;
        }
    }
    
    buffWrite[tableSize] = buffWrite[0];
}

void PulsaretTable::createSawTable()
{
    sawTable.setSize (1, (int) tableSize+ 1);
    sawTable.clear();
    auto* buffWrite = sawTable.getWritePointer (0);
    
    int harmonics = 32;
    
    for (auto i = 0; i < harmonics; i++)
    {
        float harmonic = i + 1;
        auto angleDelta = juce::MathConstants<double>::twoPi / (double) (tableSize - 1) * harmonic;
        auto currentAngle = 0.0;
        
        float harmonicAmp = 0.9f / (harmonic * harmonic);
        
        for (unsigned int i = 0; i < tableSize; ++i)
        {
            auto sample = std::sin (currentAngle);
            buffWrite[i] += (float) sample * harmonicAmp; //harmonicWeights[harmonic];
            currentAngle += angleDelta;
        }
    }

    buffWrite[tableSize] = buffWrite[0];
}


