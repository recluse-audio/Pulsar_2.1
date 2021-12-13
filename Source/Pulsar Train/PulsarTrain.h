
#pragma once
#include <JuceHeader.h>
#include "../Pulsaret/OwnedPulsaret.h"
#include "../Pulsaret/PulsaretFactory.h"
#include "../Processor/TempoSync.h"


/*
    This will be the train of pulsars.  It will take arguments for fundamental frequency.
    This will read from the
*/
class PulsarTrain
{
public:
    PulsarTrain();
    ~PulsarTrain();
    
    void prepare(double sampleRate);
    void startTrain();
  
    void update(AudioProcessorValueTreeState& apvts);
    void triggerPulsaret();
    void generateNextBlock(juce::AudioBuffer<float>& buffer);
    
    void setSmoothedFundamental(float fundFreq);
    
    int  getPeriod();
    bool needsCalculation = false;

    void flipFlashStateAtHalfPulsarPeriod();
    
    void updateFundamental(float freq);

    void calculateRanges();
    
    void setTrigger(int on, int off);
    void setPulsaretParamsAndTrigger();
    void setGlideTime(float glideTime);
    void triggerEnv();
    void triggerRelease();
    void setEnv();
    
    void triggerPulsaretWithNoAmp();


    PulsaretTable& getPulsaretTable();
    Table& getTable();

    bool trainIsRunning()
    {
        return isRunning;
    }

    bool checkIfFlashing()
    {
        return isFlashing.get();
    }

    float getFlashCoef() { return flashCoef; }

    void setTempo(double tempo, int timeSigNum, int timeSigDenom);

    bool isSynchronized() { return inSyncMode;  }

private:
    int flashCountdown = 2048;
    int flashIndex = 0;
    TempoSync   tempoSync;
    PulsaretFactory pulsaretFactory;
    int tableSize = 1024;

    juce::OwnedArray<OwnedPulsaret> pulsaretArray;
    OwnedPulsaret pulsaret1;
    OwnedPulsaret pulsaret2;
    
    juce::ADSR env;
    juce::ADSR::Parameters envParam;

    bool isContinuous = false;
    
    int  flag = 0;
    float attack;
    float decay;
    float sustain;
    float release;
    
    float pulsaretWidth = 0.1f;
    
    Atomic<bool> isFlashing{ false };
    float flashCoef = 0.f;
    bool isRunning = false;
    
    
    int triggerOn = 1;

    int onCount = 1;
    int triggerOff = 0;

    int offCount = 0;
    bool isTriggerPattern = true;
    bool trigger;

    double mSampleRate;

    int pulsarPeriod = 1024;
    int pulsarPhase;
    
    
    int samplesRemainingInPeriod = 0;
    int samplesRemainingInEnvelope = 0;
    
    Atomic<float> mAmp               { 0.0f };
    Atomic<float> mAmpSpread         { 0.0f };
    Atomic<float> mAmpRand           { 0.05f };
    juce::Random randAmp;
    Atomic<bool> ampIsSpread         { false };
    juce::Range<int> ampRange;
    
    Atomic<float> intermittance{ 1.0f };
    juce::Random randInter;

    Atomic<float> mFundFreq          { 40.0f };
    Atomic<float> mFundSpread        { 0.0f };
    Atomic<float> mFundRand          { 0.05 };
    juce::Random randFund;
    Atomic<bool> fundIsSpread        { false };
    juce::Range<int> fundRange;
    bool inSyncMode{ false };

    
    Atomic<float> panL               { 1.f };
    Atomic<float> panR               { 0.f };
    Atomic<float> panSpread          { 0.f };
    Atomic<float> panRand            { 0.f };
    juce::Random randPan;
    Atomic<bool> panIsSpread         { false };
    juce::Range<int> panRange;
    
    Atomic<float> width {0.1f};
   
    Atomic<float> glideTime { 100.f };
    
    juce::SmoothedValue<float, ValueSmoothingTypes::Multiplicative> smoothFund;
    juce::SmoothedValue<float, ValueSmoothingTypes::Multiplicative> smoothAmp;
    
};
