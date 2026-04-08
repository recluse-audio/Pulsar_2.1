/*
  ==============================================================================

    Envelope.cpp
    Created: 7 Nov 2020 7:31:31pm
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#include "Envelope.h"

Envelope::Envelope() : envelopeTable(1, 96000)
{
    createGaussian();
}

Envelope::~Envelope()
{
    
}

/*============================================================*/
/*======================== READING ==========================*/
void Envelope::prepare(double sampleRate)
{
    mSampleRate = sampleRate;
}




void Envelope::setFrequency(float freq)
{
    tableSize = envelopeTable.getNumSamples() - 1;
    auto tableSizeOverSampleRate = (float) tableSize / mSampleRate; // phase step at 1hz
    tableDelta = freq * tableSizeOverSampleRate;
}



float Envelope::getNextSample()
{
    auto index0 = (unsigned int) currentPhase;
    auto index1 = index0 + 1;
    
    auto frac = currentPhase - (float) index0;
    
    auto* table = envelopeTable.getReadPointer (0);
    auto value0 = table[index0];
    auto value1 = table[index1];
    
    currentValue = value0 + (frac * (value1 - value0));
    auto val = jlimit(0.0f, 1.0f, currentValue);
    currentPhase += tableDelta;
    
    if (currentPhase > (float) tableSize)
        resetEnvelope();

    return val;
}

void Envelope::resetEnvelope()
{
    currentPhase = 0;
    isComplete = true;
}

void Envelope::closeEnv(int sampLeft)
{
    currentPhase = tableSize - sampLeft;
}

float Envelope::getValueAtIndex(int index)
{
    auto index0 = (unsigned int)index;
    auto index1 = index0 + 1;

    auto frac = index - (float)index0;

    auto* table = envelopeTable.getReadPointer(0);
    auto value0 = table[index0];
    auto value1 = table[index1];

    currentValue = value0 + (frac * (value1 - value0));
    auto val = jlimit(0.0f, 1.0f, currentValue);

    return val;
}


juce::AudioBuffer<float>& Envelope::getTable()
{
    return envelopeTable;
}

float Envelope::getCurrentValue()
{
    return currentValue;
}

/*
    Used for writing from the EnvelopeVisualizer
*/
void Envelope::setValueAtCurrentIndex(float value)
{
    envelopeTable.setSample(0, currentIndex, value);
    currentIndex += 1;

    if (currentIndex >= envelopeTable.getNumSamples())
        currentIndex = 0;
}

void Envelope::resetIndex()
{
    currentIndex = 0;
}

int Envelope::getTotalLengthInSamples()
{
    return tableSize;
    
}

void Envelope::setTotalLengthInSamples(float samples)
{
    setTableSize(samples);
}


void Envelope::setTableSize(int size)
{
    envelopeTable.setSize(1, size);
    tableSize = size - 1;
    createGaussian();
}

void Envelope::createGaussian()
{
    envelopeTable.clear();
    
    auto* buffWrite = envelopeTable.getWritePointer (0);
    auto pi = juce::MathConstants<double>::pi;
    auto e = juce::MathConstants<double>::euler;
    
    double a = 2;
    double b = envelopeTable.getNumSamples()/2 - 1;
    double c = 2.35482; // FWHM for half power points, making 50% inside the points, 50% out
   
    
    for (unsigned int x = 0; x < envelopeTable.getNumSamples(); ++x)
    {
        double exp = -0.5f * pow(     (  (x - b) / (0.4f * b)  ),   2);
        double sample = pow(e, exp);
        
        buffWrite[x] += sample;
    }
    buffWrite[tableSize] = buffWrite[0];
}

