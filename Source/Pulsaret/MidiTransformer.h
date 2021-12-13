/*
  ==============================================================================

    MidiTransformer.h
    Created: 30 May 2021 4:18:43pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class MidiTransformer
{
public:
    MidiTransformer();
    ~MidiTransformer();

    enum TransformTypes
    {
        major,
        minor,
        majorPent,
        minorPent,
        smallNegative,
        largeNegative
    };

    void setKey(int key);
    void setTransformType(int scaleArrayIndex);
    
    void passOriginalNote(int note);
    void calculatePitchClass();
    void transformNote();
    int getTransformedNote(int note);

    int getKey();
    int getPitchClass();
    int getNegativePitchClass();
    


private:
    int transformType;
    juce::Random    randHarm;
    int mKey = 0; // 0 - 11 to represent each home key
    int originalNote = 60;
    int pitchClass = 0;
    int transformedNote = 0; // middle c

    // These represent the offsets applied to each pitch class to achieve a particular goal
    //  and not the 
    // pitch class itself.            0   1  2  3  4  5  6  7  8  9  10 11
    juce::Array<Array<int>> scaleArray;
    juce::Array<int> majorScale   { 0, -1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0 };
    juce::Array<int> minorScale   { 0, -1, 0, 0, -1, 0, 1, 0, 0, -1, 0, 0 };
    juce::Array<int> majorPenta     { 0, -1, 0, 1, 0, 2, 1, 0, 1, 0, 2, 3 };
    juce::Array<int> minorPenta     { 0, -1, 1, 0, -1, 0, 1, 0, -1, 1, 0, 0 };
    juce::Array<int> smallNeg  { -5,  5,  3,  1, -1, -3, -5,  5,  3,  1, -1, -3 };
    juce::Array<int> largeNeg  {   7, -7, -9, -11, 11, 9,  7, -7, -9, -11, 11, 9 };
};