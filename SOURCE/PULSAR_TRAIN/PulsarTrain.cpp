/*
  ==============================================================================

    PulsarTrain.cpp
    Created: 5 Nov 2020 11:24:59am
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#include "PulsarTrain.h"
#include "../Processor/PluginProcessor.h"

PulsarTrain::PulsarTrain()
{
    setTrigger(3, 2);
    tempoSync.setTempo(143, 4, 4);
}

PulsarTrain::~PulsarTrain()
{
    
    
}

void PulsarTrain::prepare(double sampleRate)
{
    pulsaret1.prepare(sampleRate);
    pulsaret2.prepare(sampleRate);
    pulsaretFactory.prepare(sampleRate);

    mSampleRate = sampleRate;
    smoothFund.reset(mSampleRate, glideTime.get());
    
    env.setSampleRate(sampleRate);
    
    fundRange.setStart(1);
    fundRange.setEnd(200);
    
    panRange.setStart(0);
    panRange.setEnd(100);
    
    ampRange.setStart(0);
    ampRange.setEnd(100);
}


void PulsarTrain::startTrain()
{
    pulsarPeriod = (1 / mFundFreq.get()) * mSampleRate;
    samplesRemainingInPeriod = pulsarPeriod;
}



//========================================================================*/
/*
    Seems small but this little function drives the whole apparatus
*/
int PulsarTrain::getPeriod()
{
    auto freq = mFundFreq.get();

    if(fundIsSpread.get() && randFund.nextFloat() <= mFundRand.get())
        freq = randFund.nextInt(fundRange);
    
    if (inSyncMode.get())
    {
        freq = tempoSync.getSynchronizedFreq(freq);
       
    }

    pulsarPeriod = (1 / freq) * mSampleRate;

    pulsaretFactory.passPulsarPeriod(pulsarPeriod); // used to calculate duty cycle

    return pulsarPeriod;
}
/*========================================================================*/

//==============================================================================
void PulsarTrain::doParameterChanged (const juce::String& parameterID, float newValue)
{
    using namespace Pulsar;

    if (parameterID == kFundamentalFreqID)
    {
        mQueuedFundamental = newValue;
        fundFreqChanged    = true;
        rangesNeedRecalc   = true;
    }
    else if (parameterID == kFundamentalSpreadID) { mFundSpread = newValue; rangesNeedRecalc = true; }
    else if (parameterID == kFundamentalRandomID)   mFundRand = newValue;
    else if (parameterID == kRhythmicGridModeID)    inSyncMode = (newValue > 0.5f);
    else if (parameterID == kPanID)
    {
        panR = newValue;
        panL = 100.f - newValue;
        rangesNeedRecalc = true;
    }
    else if (parameterID == kPanSpreadID)  { panSpread = newValue; rangesNeedRecalc = true; }
    else if (parameterID == kPanRandomID)    panRand = newValue;
    else if (parameterID == kAmpID)        { mAmp = newValue; rangesNeedRecalc = true; }
    else if (parameterID == kAmpSpreadID)  { mAmpSpread = newValue; rangesNeedRecalc = true; }
    else if (parameterID == kAmpRandomID)    mAmpRand = newValue;
    else if (parameterID == kIntermittanceID) intermittance = newValue;
    else if (parameterID == kTriggerOnID)    setTrigger((int)newValue, triggerOff.get());
    else if (parameterID == kTriggerOffID)   setTrigger(triggerOn.get(), (int)newValue);
    else if (parameterID == kGlideTimeID)  { glideTime = newValue; glideTimeChanged = true; }
    else if (parameterID == kAttackID)       attack = newValue;
    else if (parameterID == kDecayID)        decay = newValue;
    else if (parameterID == kSustainLevelID) sustain = newValue;
    else if (parameterID == kReleaseID)      release = newValue;
    else
        pulsaretFactory.doParameterChanged(parameterID, newValue);
}


void PulsarTrain::triggerPulsaret()
{
    pulsaretFactory.calculatePulsaret(pulsaret1, pulsaret2);
}

void PulsarTrain::triggerPulsaretWithNoAmp()
{
    pulsaret1.setAsMiss();
    pulsaret2.setAsMiss();
}


PulsaretTable& PulsarTrain::getPulsaretTable()
{
    return pulsaret1.getPulsaretTable();
}

Table& PulsarTrain::getTable()
{
    return pulsaret1.getTable();
}

void PulsarTrain::setTempo(double tempo, int timeSigNum, int timeSigDenom)
{
    tempoSync.setTempo(tempo, timeSigNum, timeSigDenom);
}


void PulsarTrain::generateNextBlock(juce::AudioBuffer<float>& buffer)
{
    if (fundFreqChanged.exchange(false))
        smoothFund.setTargetValue(mQueuedFundamental.get());

    if (glideTimeChanged.exchange(false))
        setGlideTime(glideTime.get());

    if (rangesNeedRecalc.exchange(false))
        calculateRanges();

    auto buffWrite = buffer.getArrayOfWritePointers();

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        mFundFreq = smoothFund.getNextValue();

        pulsaretFactory.nextSmoothValue();

        if (samplesRemainingInPeriod == 0) // we've gone through a whole pulsar period
        {
            setPulsaretParamsAndTrigger();
            samplesRemainingInPeriod = getPeriod();
        }
        
        flipFlashStateAtHalfPulsarPeriod();

        float sample = 0.f;
        
        // if the pulsaret is still running i.e. NOT free (affected by width)
        if (!pulsaret1.checkIfFree())
        {
            sample += pulsaret1.getNextSample();
        }
        // Done individually because their varied formFreqs might result in different duty cycles
        // as a result of varied widths (remember that width modulation will only return complete cycles of the waveform)
        if (!pulsaret2.checkIfFree())
        {
            sample += pulsaret2.getNextSample();
        }

        auto pLeft = panL.get() / 100.f;
        auto pRight = panR.get() / 100.f;
        auto amp = mAmp.get() / 100.f;

        auto envGain = env.getNextSample();
        if (!env.isActive())
        {
            isRunning = false;
        }

        flashCoef = envGain;
        
        if (buffer.getNumChannels() == 1)
        {
            buffWrite[0][i] = sample * amp * envGain;
        }
        else if (buffer.getNumChannels() > 1)
        {
            buffWrite[0][i] = sample * amp * pLeft * envGain ;
            buffWrite[1][i] = sample * amp * pRight * envGain;
        }
        samplesRemainingInPeriod--;
    }
}

void PulsarTrain::flipFlashStateAtHalfPulsarPeriod()
{
    float flashCycle = getPeriod() / 2.f;
    
    if (samplesRemainingInPeriod < flashCycle)
    {
        isFlashing = false;
    }
    else
    {
        isFlashing = true;
    }

    if (mFundFreq.get() > 20.f)
    {
        if (flashIndex <= 2400)
        {
            isFlashing = true;
        }
        else if (flashIndex > 2400 && flashIndex < 4800)
        {
            isFlashing = false;
        }
        else if (flashIndex == 4800)
        {
            flashIndex = 0;
        }
        flashIndex++;
    }
}

void PulsarTrain::triggerEnv()
{
    setGlideTime(glideTime.get());
    setEnv();
    isRunning = true;
    env.noteOn();
}

void PulsarTrain::triggerRelease()
{
    env.noteOff();
}




void PulsarTrain::setTrigger(int on, int off)
{
    if (off == 0)
    {
        isTriggerPattern = false;
    }
    else
    {
        triggerOn  = on;
        triggerOff = off;
        isTriggerPattern = true;
    }
}

void PulsarTrain::setGlideTime(float glideTime)
{
    auto seconds = glideTime / 1000;
    smoothFund.reset(mSampleRate, seconds);
    pulsaretFactory.setGlideTime(seconds);
}

void PulsarTrain::setEnv()
{
    envParam.attack  = attack.get()  / 1000.f;
    envParam.decay   = decay.get()   / 1000.f;
    envParam.sustain = sustain.get();
    envParam.release = release.get() / 1000.f;
    env.setParameters(envParam);
}

void PulsarTrain::calculateRanges()
{
    if(mFundSpread.get() != 1)
    {
        fundIsSpread = true;
        auto start = jlimit(1.f, 199.f, mFundFreq.get() / mFundSpread.get());
        auto end = jlimit(2.f, 200.f, mFundFreq.get() * mFundSpread.get());
        fundRange.setStart(start);
        fundRange.setEnd(end);
    }
    else if (mFundSpread.get() == 1)
    {
        fundIsSpread = false;
    }

    
    if(panSpread.get() != 1)
    {
        panIsSpread = true;
        auto start = jlimit(0.f, 99.f, panR.get() - panSpread.get()/2);
        auto end = jlimit(1.f, 100.f, panR.get() + panSpread.get()/2);
        
        panRange.setStart(start);
        panRange.setEnd(end);
    }
    else if(panSpread.get() == 1)
    {
        panIsSpread = false;
    }
   
    
    if (mAmpSpread.get() != 1)
    {
        ampIsSpread = true;
        auto start = jlimit(0.f, 99.f, mAmp.get() - mAmpSpread.get()/2);
        auto end = jlimit(1.f, 100.f, mAmp.get() + mAmpSpread.get()/2);
        ampRange.setStart(start);
        ampRange.setEnd(end);
    }
    else
    {
        ampIsSpread = false;
    }
  
}

/*=====================================================================================*/
/*========================== setPulsaretParamsAndTrigger() ============================*/
/*-------------------------------------------------------------------------------------*/

void PulsarTrain::setPulsaretParamsAndTrigger()
{
    if (isTriggerPattern.get()) // we are in a triggerPattern
    {
        if (onCount > 0)
        {
            if (randInter.nextFloat() <= intermittance.get()) // trigger if not a "miss" and a part of the triggerOn queue
            {
                if (ampIsSpread.get() && randAmp.nextFloat() <= mAmpRand.get())
                {
                    float amp = randAmp.nextInt(ampRange); // amp in range of 0 - 100
                    mAmp = amp;
                }

                if (panIsSpread.get() && randPan.nextFloat() <= panRand.get())
                {
                    panR = randPan.nextInt(panRange); // pan in range of 0 - 100
                    panL = 100.f - panR.get();
                }

                onCount--;
                triggerPulsaret();
                isFlashing = true;

            }

        }
        // start counting down the "off" counts once we have counted all the onCounts
        if (onCount <= 0)
        {
            if (offCount >= 0)
            {
                //triggerPulsaretWithNoAmp();
                isFlashing = false;
                offCount--;
            }

        }

        if (onCount <= 0 && offCount < 0)
        {
            onCount = triggerOn.get();
            offCount = triggerOff.get();
        }
    }

    if (!isTriggerPattern.get()) // we are NOT in a triggerPattern but still might need randomized values
    {

        if (randInter.nextFloat() <= intermittance.get())
        {
            if (ampIsSpread.get() && randAmp.nextFloat() <= mAmpRand.get())
            {
                float amp = randAmp.nextInt(ampRange); // amp in range of 0 - 100
                mAmp = amp;
            }

            if (panIsSpread.get() && randPan.nextFloat() <= panRand.get())
            {
                panR = randPan.nextInt(panRange); // pan in range of 0 - 100
                panL = 100.f - panR.get();
            }

            isFlashing = true;
            triggerPulsaret();
        }
        else
        {
            isFlashing = false;
        }
    }
}




