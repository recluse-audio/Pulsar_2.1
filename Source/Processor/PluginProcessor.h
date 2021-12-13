/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Pulsar Train/PulsarTrain.h"
#include "../Pulsaret/PulsaretTable.h"
#include "../Pulsaret/Table.h"

#include "TempoSync.h"


//==============================================================================
/**
*/
class PulsarAudioProcessor  : public juce::AudioProcessor, public ValueTree::Listener
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
    void savePreset(juce::String& presetPath); // name will be set and held in the menu component
    void getStateInformation (juce::MemoryBlock& destData) override;

    void loadPreset(juce::String& presetPath); // calls set
    void setStateInformation (const void* data, int sizeInBytes) override;

    
    void update();
    
    void handleMidi(juce::MidiBuffer midiBuffer);
    
    AudioProcessorValueTreeState apvts;
    AudioProcessorValueTreeState::ParameterLayout createParams();
    void valueTreePropertyChanged(ValueTree& tree, const Identifier& property) override;
    
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
    AudioPlayHead* playHead;
    AudioPlayHead::CurrentPositionInfo currentPos;

    PulsarTrain pulsarTrain;
    bool trainRunning = false;
    bool mustUpdateProcessing { false };
    bool inSyncMode{ false };
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PulsarAudioProcessor)
};
