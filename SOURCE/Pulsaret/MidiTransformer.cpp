/*
  ==============================================================================

    MidiTransformer.cpp
    Created: 30 May 2021 4:18:43pm
    Author:  ryand

  ==============================================================================
*/

#include "MidiTransformer.h"

MidiTransformer::MidiTransformer()
{
    scaleArray.add(major, minor, majorPenta, minorPenta, smallNeg, largeNeg);
}

MidiTransformer::~MidiTransformer()
{

}

void MidiTransformer::setKey(int key)
{
    mKey = key;
}

void MidiTransformer::setTransformType(int scaleArrayIndex)
{
    DBG(scaleArrayIndex);
    transformType = scaleArrayIndex;
}

void MidiTransformer::passOriginalNote(int note)
{
    originalNote = note;
    calculatePitchClass();
}

void MidiTransformer::calculatePitchClass()
{
    int note = originalNote - mKey; 
    pitchClass = note % 12;
}

void MidiTransformer::transformNote()
{
   auto scale = scaleArray[transformType];
   transformedNote = originalNote + scale[pitchClass];
}

int MidiTransformer::getTransformedNote(int note)
{
    originalNote = note;
    calculatePitchClass();
    transformNote();
    return transformedNote;
}

int MidiTransformer::getKey()
{
    return mKey;
}

int MidiTransformer::getPitchClass()
{
    return pitchClass;
}

int MidiTransformer::getNegativePitchClass()
{
    int note = transformedNote - mKey;
    note %= 12;
    return note;
}
