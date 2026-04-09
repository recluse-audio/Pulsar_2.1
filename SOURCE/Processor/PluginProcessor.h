/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../PULSAR_TRAIN/PulsarTrain.h"
#include "../Pulsaret/PulsaretTable.h"
#include "../Pulsaret/Table.h"

#include "TempoSync.h"

namespace Pulsar
{

  static const juce::String kFundamentalFreqID        = "fundamental_freq";
  static const juce::String kFundamentalSpreadID      = "fundamental_spread";
  static const juce::String kFundamentalRandomID      = "fundamental_random";
  static const juce::String kRhythmicGridModeID       = "rhythmic_grid_mode";

  static const juce::String kFormantFreqID            = "formant_freq";
  static const juce::String kFormantSpreadID          = "formant_spread";
  static const juce::String kFormantRandomID          = "formant_random";
  static const juce::String kFormantKeylockModeID     = "formant_keylock_mode";
  static const juce::String kFormKeyID                = "form_key";
  static const juce::String kFormScaleID              = "form_scale";

  static const juce::String kFormantFreq2ID           = "formant_freq2";
  static const juce::String kFormantSpread2ID         = "formant_spread2";
  static const juce::String kFormantRandom2ID         = "formant_random2";
  static const juce::String kFormant2KeylockModeID    = "formant2_keylock_mode";
  static const juce::String kFormKey2ID               = "form_key2";
  static const juce::String kFormScale2ID             = "form_scale2";

  static const juce::String kPanID                    = "pan";
  static const juce::String kPanSpreadID              = "pan_spread";
  static const juce::String kPanRandomID              = "pan_random";

  static const juce::String kWaveTypeID               = "wave_type";
  static const juce::String kWaveSpreadID             = "wave_spread";
  static const juce::String kWaveRandomID             = "wave_random";

  static const juce::String kAmpID                    = "amp";
  static const juce::String kAmpSpreadID              = "amp_spread";
  static const juce::String kAmpRandomID              = "amp_random";

  static const juce::String kIntermittanceID          = "intermittance";
  static const juce::String kTriggerOnID              = "trigger_on";
  static const juce::String kTriggerOffID             = "trigger_off";
  static const juce::String kTriggerID                = "trigger";

  static const juce::String kGlideTimeID              = "glide_time";
  static const juce::String kWidthID                  = "width";

  static const juce::String kAttackID                 = "attack";
  static const juce::String kDecayID                  = "decay";
  static const juce::String kSustainLevelID           = "sustain_level";
  static const juce::String kReleaseID                = "release";
} // end namespace Pulsar
//==============================================================================
/**
*/
class PulsarAudioProcessor  : public juce::AudioProcessor
                            , public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    PulsarAudioProcessor();
    ~PulsarAudioProcessor() override;

    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void savePreset(const juce::String& presetPath); // name will be set and held in the menu component
    void getStateInformation (juce::MemoryBlock& destData) override;

    void loadPreset(const juce::String& presetPath); // calls set
    void setStateInformation (const void* data, int sizeInBytes) override;

        //==============================================================================
    // AudioProcessorValueTreeState::Listener
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    void handleMidi(juce::MidiBuffer midiBuffer);
    
    AudioProcessorValueTreeState apvts;
    AudioProcessorValueTreeState::ParameterLayout createParams();
    
    void triggerPulsarTrain();
    void releasePulsarTrain();
    
    PulsaretTable& getPulsaretTable();
    Table& getTable();
    
    /*
        These three are used to communicate between the train and the editor
    */
    bool isTrainRunning();
    bool isFlashing(); // checks if train should flash for a "hit" or not flash for a "miss" using trigger patterns / stochasticism
    float getFlashCoef(); // scale flash by amp 

    void loadDefaultParameters();

private:
    PulsarTrain pulsarTrain;
    bool trainRunning = false;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PulsarAudioProcessor)
};
