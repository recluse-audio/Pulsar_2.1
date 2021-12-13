/*
  ==============================================================================

    PulsarSynthesiser.cpp
    Created: 29 Jan 2021 9:43:29am
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#include "PulsarSynthesiser.h"

//PulsarSynthesiser::PulsarSynthesiser(Atomic<float>& fund, juce::Range<int>& fuRange, Atomic<float>& fundRand, Atomic<bool>& fuIsSpread,
//                                     Atomic<float>& form, juce::Range<int>& fRange, Atomic<float>& formRand, Atomic<bool>& fIsSpread,
//                                     Atomic<float>& form2, juce::Range<int>& fRange2, Atomic<float>& formRand2, Atomic<bool>& fIsSpread2,
//                                     Atomic<float>& pL, Atomic<float>& pR, juce::Range<int>& pRange, Atomic<float>& pRand, Atomic<bool>& pIsSpread,
//                                     Atomic<float>& amp, juce::Range<int>& aRange, Atomic<float>& ampRand, Atomic<bool>& aIsSpread,
//                                     Atomic<float>& inter, Atomic<float>& wType, juce::Range<int>& wRange, Atomic<float>& wRand, Atomic<bool>& wIsSpread) :
//mFundFreq(fund), fundRange(fuRange), mFundRand(fundRand), fundIsSpread(fuIsSpread),
//mFormFreq(form), formRange(fRange), mFormRand(formRand), formIsSpread(fIsSpread),
//mFormFreq2(form2), formRange2(fRange2), mFormRand2(formRand2), formIsSpread2(fIsSpread2),
//panL(pL), panR(pR), panRange(pRange), panRand(pRand), panIsSpread(pIsSpread), mAmp(amp), ampRange(aRange), mAmpRand(ampRand), ampIsSpread(aIsSpread), intermittance(inter), waveType(wType), waveRange(wRange), waveRand(wRand), waveIsSpread(wIsSpread)
//{
//    for (auto i = 0; i < 5; ++i)
//    {
//        addVoice (new PulsarSynthVoice);
//    }
//
//    setVoiceStealingEnabled (true);
//}
//
//PulsarSynthesiser::~PulsarSynthesiser()
//{
//    
//}
//
//void PulsarSynthesiser::prepare(double sampleRate, int blockSize)
//{
//    mSampleRate = sampleRate;
//    mBlockSize = blockSize;
//    
//    setCurrentPlaybackSampleRate(sampleRate);
//}
//
//void PulsarSynthesiser::renderNextSubBlock(AudioBuffer<float> &outputAudio, int startSample, int numSamples)
//{
//    for (auto* v : voices)
//    {
//        
//    }
//    
//    MPESynthesiser::renderNextSubBlock (outputAudio, startSample, numSamples);
//}

