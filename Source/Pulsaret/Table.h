/*
  ==============================================================================

    Table.h
    Created: 1 Sep 2021 8:18:53pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Table
{
public:
    Table();
    ~Table();

    void setTable(float tableIndex);
    AudioBuffer<float>& getTable();
    AudioBuffer<float>& getTable0();
    AudioBuffer<float>& getTable1();
    float getWaveFraction();
    void loadTablesFromDisk();

    int getTableSize();

    void createSinc();
private:
    OwnedArray<AudioBuffer<float>, CriticalSection> tableArray;
    int arraySize = 0; // stores actual number of samples in array
    juce::AudioBuffer<float> table0;
    juce::AudioBuffer<float> table1;


    AudioBuffer<float> interpolatedBuffer;

    Atomic<float> floatWaveIndex{ 0.f };
    Atomic<float> waveFraction{ 0.f };
    Atomic<float> normWaveIndex{ 0.f };
    Atomic<int> intWaveIndex{ 0 }; // selected wavetable, rounded down
    AudioFormatManager formatManager;

    int tableSize = 2048;
};