/*
  ==============================================================================

    Envelope.cpp
    Created: 7 Nov 2020 7:31:31pm
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#include "Envelope.h"

Envelope::Envelope() : envelopeTable(1, 1024)
{
    
    createGaussian();
}

Envelope::Envelope(bool isWavetableEnv) : envelopeTable(1, 1024), isWaveEnv(isWavetableEnv)
{
    tableSize = envelopeTable.getNumSamples() - 1;
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


void Envelope::setTotalMilliseconds(float ms)
{
//    totalMilliseconds = ms;
//    envLengthInSamples = (ms * mSampleRate) / 1000;
//    tableIncrement = (float) tableSize / envLengthInSamples; // divides our table up into sample size increments that are less than one index of the table
}

void Envelope::setFrequency(float freq)
{
    tableSize = envelopeTable.getNumSamples() - 1;
    auto tableSizeOverSampleRate = (float) tableSize / mSampleRate;
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
    
    envVal = value0 + (frac * (value1 - value0));
    
    currentValue = envVal.get();
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

/*============================================================*/
/*======================= WRITING ============================*/
void Envelope::createEnvelopeTable()
{
    
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


void Envelope::debugBuffer()
{
    for (int i = 0; i < envelopeTable.getNumSamples(); ++i)
    {
        auto sample = envelopeTable.getSample(0, i);
        //DBG(sample);
    }
    
}

int Envelope::getTotalLengthInSamples()
{
    return envLengthInSamples;
    
}

float Envelope::getTotalMilliseconds()
{
    return totalMilliseconds;
}

/*
    This is useful to make copies of envelopes, really just getting the relevant information 
*/
void Envelope::copyEnvelope(Envelope& env)
{
    envelopeTable.makeCopyOf(env.getTable());
    setTotalMilliseconds(env.getTotalMilliseconds());
}

void Envelope::setSliderVal(double val)
{
    sliderVal = val;
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

