/*
  ==============================================================================

    PulsaretTable.h
    Created: 6 Nov 2020 4:34:52pm
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


class PulsaretTable
{
public:
    PulsaretTable();
    ~PulsaretTable();
    
    void createSineTable();
    void createSincTable();
    void createTriTable();
    void createSawTable();
    void createTables();
    
    void setTable(float selection);
    juce::AudioBuffer<float>& getTable();
    juce::AudioBuffer<float>& getNextTable();
    void setEnvLevel(float envLevel);
    float getWaveFraction();
    
    float getWaveIndex();
    
private:
    //juce::OwnedArray<AudioBuffer<float>> tableArray;
    //juce::OwnedArray<AudioBuffer<float>> tableArray;
    juce::Array<AudioBuffer<float>> tableArray;
    
    //juce::AudioBuffer<float> pulsaretTable;
    juce::AudioBuffer<float> sineTable;
    juce::AudioBuffer<float> sincTable;
    juce::AudioBuffer<float> triTable;
    juce::AudioBuffer<float> sawTable;
    
    Atomic<float> sineGain {0.f};
    Atomic<float> sincGain {0.f};
    Atomic<float> triGain  {0.f};
    Atomic<float> sawGain  {0.f};
    
    Atomic<float> waveFraction {0.f}; // how much to interpolate to next wavetable
    Atomic<float> floatWaveIndex {0.f};
    Atomic<float> normalWaveIndex {0.f};
    Atomic<int> intWaveIndex {0}; // selected wavetable, rounded down
    
    
    int tableSize = 1024;
    float currentIndex = 0.0f, tableDelta = 0.0f;
    
};
