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
    formatManager.registerBasicFormats();
    loadTablesFromDisk();
}

Table::~Table()
{
}
 // using assignment because gui calls getTable() and processor calls setTable() (value tree)
void Table::setTable(float tableIndex)
{
    normWaveIndex = tableIndex ;
    
    floatWaveIndex = NormalisableRange<float>(0.f, 
        (float)arraySize, 0.001, 1.f).convertFrom0to1(normWaveIndex.get());

    intWaveIndex = (int)floatWaveIndex.get(); // cast to int

    waveFraction = floatWaveIndex.get() - (float)intWaveIndex.get(); // overlap

    table0.makeCopyOf(*tableArray[intWaveIndex.get()]);
    table1.makeCopyOf(*tableArray[intWaveIndex.get() + 1]);


   // DBG(floatWaveIndex);
    //auto buff1 = tableArray[intWaveIndex]->getReadPointer(0);
    //auto buff2 = tableArray[intWaveIndex + 1]->getReadPointer(0);

    //interpolatedBuffer.clear();
    //interpolatedBuffer.setSize(1, tableSize);

    //interpolatedBuffer.addFromWithRamp(0, 0, buff1,
    //    tableSize, waveFraction.get(), waveFraction.get());

    //interpolatedBuffer.addFromWithRamp(0, 0, buff2,
    //    tableSize, 1.f - waveFraction.get(), 1.f - waveFraction.get());

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
    //return sincTable;
}

AudioBuffer<float>& Table::getTable0()
{
    return table0;
    //return sincTable;
}

AudioBuffer<float>& Table::getTable1()
{
    return table1;
   // return sincTable;
}

float Table::getWaveFraction()
{
    return waveFraction.get();
}

int Table::getTableSize()
{
    return tableSize;
}


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
