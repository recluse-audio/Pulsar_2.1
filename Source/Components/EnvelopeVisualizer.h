/*
  ==============================================================================

    EnvelopeVisualizer.h
    Created: 7 Nov 2020 8:53:58am
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EnvelopeControlPoint.h"
#include "../Envelope/Envelope.h"
#include "../Processor/PluginProcessor.h"



//==============================================================================

class EnvelopeVisualizer  : public juce::Component, public juce::Timer, public juce::Slider::Listener
{
public:
    EnvelopeVisualizer(PulsarAudioProcessor&, Envelope&);
    ~EnvelopeVisualizer() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const MouseEvent& e) override;
    void mouseUp   (const MouseEvent& e) override;
    void mouseMove (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseDoubleClick (const MouseEvent& e) override;
    
    bool compareX(EnvelopeControlPoint<float> x1, EnvelopeControlPoint<float> x2);
    void sortEnvPoints();
    void timerCallback() override;
    void putValuesInTable();
    Envelope& getEnvelope();
    
    // I think doing this would be a less confusing option rather than put it in the constructor
    // Imagine trying to implement the class and thinking "WTF is going on, why won't it work"?  Setting the name seems more like a feature than a necessity
    void setName(juce::String mName);
    void setValue();
    
    void sliderValueChanged(Slider* s) override;
    
    /*
        need this to redraw pulsaret only for waveEnvelopes
    */
    
private:
    juce::Point<float> mousePosition;
    juce::Point<float> lastPosition;
    std::unique_ptr<juce::Slider> slider; // each env gets a slider whose range is the height of the frame
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment>     sliderAttachment;
    
    juce::OwnedArray<EnvelopeControlPoint<float>> envPoints;
    EnvelopeControlPoint<float> startPoint;
    EnvelopeControlPoint<float> endPoint;
    EnvelopeControlPoint<float>* selectedPoint;
    float pointSize = 3.f;
    juce::Rectangle<int> frame;
    juce::Rectangle<int> reducedFrame;
    
    bool isDraggingPoint = false;
    bool isDraggingStartPoint = false;
    bool isDraggingEndPoint = false;
    
    juce::Path envPath;
    juce::Path sliderPath;
    
    Envelope& env;
    
    std::unique_ptr<Label> nameLabel;
    std::unique_ptr<Label> valueLabel;
    juce::String name = "";
    juce::String value = "value";
    
    bool envChange = false;
    
    double mSampleRate;
    double lastSliderVal = 0;
    
    PulsarAudioProcessor& audioProcessor;
   
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeVisualizer)
};
