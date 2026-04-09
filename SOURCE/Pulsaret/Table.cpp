/*
  ==============================================================================

    Table.cpp
    Created: 1 Sep 2021 8:18:53pm
    Author:  ryand

  ==============================================================================
*/

#include "Table.h"

Table::Table() : interpolatedBuffer(1, 2048), table0(1, 2048), table1(1, 2048)
{
    for (int i = 0; i < 24; i++)
    {
        tableArray.add(new AudioBuffer<float>(1, tableSize+1));
    }
    //formatManager.registerBasicFormats();
    //loadTablesFromDisk();
    generateBasicWaveforms();
}

Table::~Table()
{
}

void Table::generateBasicWaveforms()
{
    // 0: Sine, 1: Triangle, 2: Sawtooth, 3: Square
    arraySize = 4;

    const float twoPi = juce::MathConstants<float>::twoPi;

    // Sine
    {
        auto* w = tableArray[0]->getWritePointer(0);
        for (int i = 0; i <= tableSize; ++i)
            w[i] = std::sin(twoPi * (float)i / (float)tableSize);
    }

    // Triangle
    {
        auto* w = tableArray[1]->getWritePointer(0);
        for (int i = 0; i <= tableSize; ++i)
        {
            float t = (float)i / (float)tableSize;
            w[i] = 1.f - 4.f * std::abs(t - std::round(t));
        }
    }

    // Sawtooth
    {
        auto* w = tableArray[2]->getWritePointer(0);
        for (int i = 0; i <= tableSize; ++i)
        {
            float t = (float)i / (float)tableSize;
            w[i] = 2.f * t - 1.f;
        }
    }

    // Square
    {
        auto* w = tableArray[3]->getWritePointer(0);
        for (int i = 0; i <= tableSize; ++i)
        {
            float t = (float)i / (float)tableSize;
            w[i] = t < 0.5f ? 1.f : -1.f;
        }
    }

    // Initialise table0/table1 so getTable() is valid before setTable() is called
    table0.makeCopyOf(*tableArray[0]);
    table1.makeCopyOf(*tableArray[1]);
}

 // using assignment because gui calls getTable() and processor calls setTable() (value tree)
void Table::setTable(float tableIndex)
{
    if (arraySize < 2)
        return;

    normWaveIndex = tableIndex;

    floatWaveIndex = NormalisableRange<float>(0.f,
        (float)(arraySize - 1), 0.001f, 1.f).convertFrom0to1(normWaveIndex.get());

    intWaveIndex = jlimit(0, arraySize - 2, (int)floatWaveIndex.get());

    waveFraction = floatWaveIndex.get() - (float)intWaveIndex.get();

    table0.makeCopyOf(*tableArray[intWaveIndex.get()]);
    table1.makeCopyOf(*tableArray[intWaveIndex.get() + 1]);
}

/*
    Do interpolation here instead of in pulsaret visualizer and owned pulsaret hopefully
*/
AudioBuffer<float>& Table::getTable()
{
    auto buff1 = tableArray[intWaveIndex.get()]->getReadPointer(0);
    auto buff2 = tableArray[intWaveIndex.get() + 1]->getReadPointer(0);

    interpolatedBuffer.clear();
    interpolatedBuffer.setSize(1, tableSize);

    interpolatedBuffer.addFromWithRamp(0, 0, buff1,
        tableSize, waveFraction.get(), waveFraction.get());

    interpolatedBuffer.addFromWithRamp(0, 0, buff2,
        tableSize, 1.f - waveFraction.get(), 1.f - waveFraction.get());

    return interpolatedBuffer;
}

AudioBuffer<float>& Table::getTable0()
{
    return table0;
}

AudioBuffer<float>& Table::getTable1()
{
    return table1;
}

float Table::getWaveFraction()
{
    return waveFraction.get();
}

int Table::getTableSize()
{
    return tableSize;
}

/*
void Table::loadTablesFromDisk()
{
    auto waveFolders = juce::File("C:/ProgramData/Recluse-Audio/Pulsar/Waveforms/Echo Sound Works Core Tables/Artie/").findChildFiles(2, true, "*.wav");

    arraySize = waveFolders.size();
    for (int i = 0; i < waveFolders.size(); i++)
    {
        auto waveFile = juce::File(waveFolders[i].getFullPathName());
        std::unique_ptr<juce::AudioFormatReader> formatReader{ formatManager.createReaderFor(waveFile) };
        auto buffWrite = tableArray[i]->getWritePointer(0);
        formatReader->read(tableArray[i], 0, tableSize, 0, true, false);
    }
}
*/
