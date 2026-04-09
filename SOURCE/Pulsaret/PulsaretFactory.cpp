/*
  ==============================================================================

    PulsaretFactory.cpp
    Created: 13 May 2021 8:12:19am
    Author:  ryand

  ==============================================================================
*/

#include "PulsaretFactory.h"
#include "../Processor/PluginProcessor.h"

PulsaretFactory::PulsaretFactory() : frequencyOfA(440.0)
{
}

PulsaretFactory::~PulsaretFactory(){}

void PulsaretFactory::doParameterChanged(const juce::String& parameterID, float newValue)
{
    using namespace Pulsar;

    if      (parameterID == kFormantFreqID)           { mQueuedFormant1 = newValue; formant1Changed = true; }
    else if (parameterID == kFormantSpreadID)         mFormSpread = newValue;
    else if (parameterID == kFormantRandomID)         mFormRand = newValue;
    else if (parameterID == kFormantKeylockModeID)    formIsHarm = (newValue > 0.5f);
    else if (parameterID == kFormKeyID)               mFormKey = (int)newValue;
    else if (parameterID == kFormScaleID)             mFormScale = (int)newValue;
    else if (parameterID == kFormantFreq2ID)          { mQueuedFormant2 = newValue; formant2Changed = true; }
    else if (parameterID == kFormantSpread2ID)        mFormSpread2 = newValue;
    else if (parameterID == kFormantRandom2ID)        mFormRand2 = newValue;
    else if (parameterID == kFormant2KeylockModeID)   formIsHarm2 = (newValue > 0.5f);
    else if (parameterID == kFormKey2ID)              mFormKey2 = (int)newValue;
    else if (parameterID == kFormScale2ID)            mFormScale2 = (int)newValue;
    else if (parameterID == kWaveTypeID)              waveType = newValue;
    else if (parameterID == kWaveSpreadID)            waveSpread = newValue;
    else if (parameterID == kWaveRandomID)            waveRand = newValue;
    else if (parameterID == kWidthID)                 width = newValue;
}

void PulsaretFactory::prepare(double sampleRate)
{
    mSampleRate = sampleRate;

    formRange.setStart(100);
    formRange.setEnd(10000);

    formRange2.setStart(100);
    formRange2.setEnd(10000);

    waveRange.setStart(0);
    waveRange.setEnd(100);
}

void PulsaretFactory::calculatePulsaret(OwnedPulsaret& p1, OwnedPulsaret& p2)
{
    mFormFreq = smoothForm.getNextValue();
    mFormFreq2 = smoothForm2.getNextValue();

    calculateRanges(p1, p2);
    calculateFreq(p1, p2);
    calculateWave(p1, p2);
    calculateDutyCycle(width.get());

    p1.setAsHit();
    p2.setAsHit();
    p1.setLengthInSamples(pulsaretSamples1);
    p2.setLengthInSamples(pulsaretSamples2);
    p1.setRunning();
    p2.setRunning();




}


void PulsaretFactory::calculateFreq(OwnedPulsaret& p1, OwnedPulsaret& p2)
{
    float pulsaretFreq1;
    float pulsaretFreq2;

    if (randFreq1.nextFloat() < mFormRand.get() && formIsSpread.get()) // As the value approaches 1.0, we approach a 100% chance we get a random
        pulsaretFreq1 = randFreq1.nextInt(formRange);
    else
        pulsaretFreq1 = mFormFreq.get();

    if (randFreq2.nextFloat() < mFormRand2.get() && formIsSpread2.get()) 
        pulsaretFreq2 = randFreq2.nextInt(formRange2);
    else
        pulsaretFreq2 = mFormFreq2.get();
    
    if (formIsHarm.get())
    {
        midiTransformer.setTransformType(mFormScale.get());
        midiTransformer.setKey(mFormKey.get());
        pulsaretFreq1 = getHarmonicFormant(pulsaretFreq1);
    }

    if (formIsHarm2.get())
    {
        midiTransformer.setTransformType(mFormScale2.get());
        midiTransformer.setKey(mFormKey2.get());
        pulsaretFreq2 = getHarmonicFormant(pulsaretFreq2);
    }

    p1.setFrequency(pulsaretFreq1); // get pulsaret ready to read
    p2.setFrequency(pulsaretFreq2);
}

void PulsaretFactory::calculateWave(OwnedPulsaret& p1, OwnedPulsaret& p2)
{

    if (randWave.nextFloat() < waveRand.get() && waveIsSpread.get())
    {
        auto waveNum = randWave.nextInt(waveRange); // notice I am using same rand object, I don't believe this causes any issues

        p1.table.setTable(waveNum / 100.f);
        p2.table.setTable(waveNum / 100.f);

    }
    else
    {

        p1.table.setTable(waveType.get() / 100.f);
        p2.table.setTable(waveType.get() / 100.f);

    }

}

void PulsaretFactory::calculateRanges(OwnedPulsaret& p1, OwnedPulsaret& p2)
{
    if (mFormSpread.get() != 1)
    {
        formIsSpread = true;
        auto start = jlimit(100.f, 9999.f, mFormFreq.get() / mFormSpread.get());
        auto end = jlimit(101.f, 10000.f, mFormFreq.get() * mFormSpread.get());

        formRange.setStart(start);
        formRange.setEnd(end);
    }
    else if (mFormSpread.get() == 1)
    {
        formIsSpread = false;
    }

    if (mFormSpread2.get() != 1)
    {
        formIsSpread2 = true;
        auto start = jlimit(100.f, 9999.f, mFormFreq2.get() / mFormSpread2.get());
        auto end = jlimit(101.f, 10000.f, mFormFreq2.get() * mFormSpread2.get());
        formRange2.setStart(start);
        formRange2.setEnd(end);
    }
    else if (mFormSpread2.get() == 1)
    {
        formIsSpread2 = false;
    }

    if (waveSpread.get() != 1)
    {
        waveIsSpread = true;
        auto start = jlimit(0.f, 99.f, waveType.get() - waveSpread.get() / 2);
        auto end = jlimit(1.f, 100.f, waveType.get() + waveSpread.get() / 2);
        waveRange.setStart(start);
        waveRange.setEnd(end);

    }
    else
    {
        waveIsSpread = false;
    }

}




void PulsaretFactory::calculateDutyCycle(float wid)
{
    float dutyCycle = pulsarPeriodInSamples * wid;
    pulsaretSamples1 = jlimit<float>(1.f, pulsarPeriodInSamples - 1, dutyCycle);
    pulsaretSamples2 = jlimit<float>(1.f, pulsarPeriodInSamples - 1, dutyCycle);

    if (wid < 0.02)
    {
        pulsaretSamples1 = (1 / mFormFreq.get()) * mSampleRate;
        pulsaretSamples2 = (1 / mFormFreq2.get()) * mSampleRate;
    }

}

void PulsaretFactory::setGlideTime(float seconds)
{
    smoothForm.reset(mSampleRate, seconds);
    smoothForm2.reset(mSampleRate, seconds);
}

void PulsaretFactory::passPulsarPeriod(float pulsarPeriod)
{
    pulsarPeriodInSamples = pulsarPeriod;
}

void PulsaretFactory::nextSmoothValue()
{
    if (formant1Changed.exchange(false))
        smoothForm.setTargetValue(mQueuedFormant1.get());
    if (formant2Changed.exchange(false))
        smoothForm2.setTargetValue(mQueuedFormant2.get());

    mFormFreq  = smoothForm.getNextValue();
    mFormFreq2 = smoothForm2.getNextValue();
}

float PulsaretFactory::getHarmonicFormant(float formFreq)
{
    int midi = jmap<float>(formFreq, 100.f, 10000.f, 48, 127);
    int transMidi = midiTransformer.getTransformedNote(midi);
    float freq = frequencyOfA * std::pow(2.0, (transMidi - 69) / 12.0); // freqofA set in header taken from the juce function midiMessage
   
    return freq;
}
/*
    returns a float freq to pass
*/
int PulsaretFactory::getTransformedMidi(int midiNumber)
{

    return 0;
}
