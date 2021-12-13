/*
  ==============================================================================

    EnvelopeVisualizer.cpp
    Created: 7 Nov 2020 8:53:59am
    Author:  Billie (Govnah) Jean

  ==============================================================================
*/

#include "EnvelopeVisualizer.h"

//==============================================================================
EnvelopeVisualizer::EnvelopeVisualizer(PulsarAudioProcessor& p, Envelope& e) : audioProcessor(p), env(e)
{
    setSize(400, 150);

    
    putValuesInTable();
    
    startTimerHz(60);
    
    frame.setBounds(getWidth() * 0.16, getHeight() * 0.03, getWidth() * 0.85, getHeight() * 0.96);
    reducedFrame.setBounds(frame.getX() + pointSize, frame.getY() + pointSize, frame.getWidth() - (pointSize * 4), frame.getHeight() - (pointSize * 2));
    
    auto width = reducedFrame.getWidth();
    auto height = reducedFrame.getHeight();
    
    /*
     Stationary terminal nodes in the envelope.  Can only be adjusted in their Y axis
     */
    //startPoint.setPosition(frame.getX() + 5, height );
    
    startPoint.setCentre(reducedFrame.getX(), height);
    startPoint.setSize(pointSize, pointSize);
    
    endPoint.setCentre(reducedFrame.getWidth(), height);
    endPoint.setSize(pointSize, pointSize);
    
    juce::Font font ("Consolas", "Bold", 20.f);
    
    nameLabel = std::make_unique<Label>("", name);
    nameLabel->setBounds(0, 0, 150, 25);
    nameLabel->setCentrePosition(getLocalBounds().getCentre());
    nameLabel->setFont(font);
    nameLabel->setColour(juce::Label::textColourId, juce::Colours::lightgrey.withAlpha(0.3f));
    nameLabel->setJustificationType(juce::Justification::centred);
    nameLabel->toBack();
    addAndMakeVisible (nameLabel.get());
    
     juce::Font font2 ("Consolas", "Bold", 12.f);
    valueLabel = std::make_unique<Label>("", value);
    valueLabel->setBounds(250, 130, 100, 25);
    valueLabel->setFont(font2);
    valueLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    valueLabel->setJustificationType(juce::Justification::centred);
    valueLabel->toBack();
    addAndMakeVisible (valueLabel.get());
    
    slider = std::make_unique<Slider>(Slider::SliderStyle::RotaryVerticalDrag, Slider::NoTextBox);
    slider->setBounds(5, 100, 50, 50);
    slider->addListener(this);
    slider->setRange(0.0, 1.0);
    slider->setValue(0.5);
    addAndMakeVisible (slider.get());
    
    
    
    putValuesInTable();

}

EnvelopeVisualizer::~EnvelopeVisualizer()
{
}

/*
    what owns the graphic context of the component class?  What calls the paint method at all?
    looks like ComponentPeer is a class that does the real nitty gritty work involved in making
    a component, which is really just a graphics content if you think about it.
*/
void EnvelopeVisualizer::paint (juce::Graphics& g)
{
    g.setColour(juce::Colours::black);
    g.fillAll();
    
    g.setColour(juce::Colours::orange);
    g.drawRoundedRectangle(getLocalBounds().toFloat(), 1.f, 1.f);
    g.setColour(juce::Colours::orange.withAlpha(0.3f));
    g.drawRoundedRectangle(frame.toFloat(), 1.f, 1.f);
    

    g.setColour(juce::Colours::skyblue.withLightness(0.6f));
    envPath.clear();
    sliderPath.clear();
    
    
    // drawing ellipse based on top left coord, but the points operate using their centre coord
    //(don't want to change bc of mouse use)
    g.drawEllipse(startPoint, pointSize);
    g.drawEllipse(endPoint, pointSize);
    envPath.startNewSubPath(startPoint.getCentre());
    sortEnvPoints();
    for (int i = 1; i < envPoints.size(); ++i)
    {
        auto* point = envPoints[i];
        g.drawEllipse(*point, pointSize);
        envPath.lineTo(point->getCentre());
//        juce::Point<float> cp;
//        cp.setX((envPoints[i]->getCentreX() - envPoints[i-1]->getCentreX())*0.5);
//        cp.setY((envPoints[i]->getCentreY() - envPoints[i-1]->getCentreY())*0.5);
//        envPath.quadraticTo(cp, envPoints[i]->getCentre());
    }
    envPath.lineTo(endPoint.getCentre());
    juce::PathStrokeType stroke(0.75f, juce::PathStrokeType::curved);
    g.strokePath(envPath, stroke);

    setValue();
    
    envChange = false;
    
    auto val = 1 - slider->getValue();
    sliderPath.startNewSubPath(frame.getX(), frame.getHeight() * val);
    sliderPath.lineTo(getWidth(), frame.getHeight() * val);
    juce::PathStrokeType sliderStroke(1.75f, juce::PathStrokeType::curved);
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.strokePath(sliderPath, sliderStroke);
    
    juce::Rectangle<int> testRect;
    testRect.setBounds(0, 0, 20, 20);
    testRect.setCentre(envPath.getPointAlongPath(300).getX(), envPath.getPointAlongPath(300).getY());
    g.drawRect(testRect);

    
    
    g.drawRect(reducedFrame);
}

void EnvelopeVisualizer::sortEnvPoints()
{
    auto sortLambda = [](EnvelopeControlPoint<float>* x1, EnvelopeControlPoint<float>* x2)
    {
        return x1->getX() < x2->getX();
    };
    
    std::sort(envPoints.begin(), envPoints.end(), sortLambda);
    
}

bool EnvelopeVisualizer::compareX(EnvelopeControlPoint<float> x1, EnvelopeControlPoint<float> x2)
{
    return x1.getX() < x2.getX();
}

void EnvelopeVisualizer::resized()
{
    

}

void EnvelopeVisualizer::timerCallback()
{
//    if(envChange)
//    {
//        repaint();
//    }
    
    valueLabel->repaint();
    
}

void EnvelopeVisualizer::setName(juce::String mName)
{
    nameLabel->setText(mName, dontSendNotification);
    repaint();
}

void EnvelopeVisualizer::setValue()
{
    valueLabel->setText(String(env.getCurrentValue()), dontSendNotification);
}

void EnvelopeVisualizer::mouseDown(const MouseEvent& e)
{
    envChange = true;
    // check if it is within the component
    if(getLocalBounds().contains(e.x, e.y))
    {
        mousePosition.setXY(e.x, e.y);
        lastPosition.setXY(e.x, e.y);
        
        // check if our cursor is inside one the dots in the vector
        // first the terminal points
        if (startPoint.contains(mousePosition))
        {
            isDraggingStartPoint = true;
        }
    
        if (endPoint.contains(mousePosition))
        {
            isDraggingEndPoint = true;
        }
    
        // look through the array if we aren't grabbing the start or end points
       if (!startPoint.contains(mousePosition) || !endPoint.contains(mousePosition))
       {
            for (size_t i = 0; i < envPoints.size(); ++i)
            {
                auto* point = envPoints[i];
                if(point->contains(mousePosition))
                {
                    if(e.mods.isCommandDown() || e.mods.isRightButtonDown())
                    {
                        envPoints.removeObject(point);
                    }
                    else
                    {
                        isDraggingPoint = true;
                        selectedPoint = point;
                    }
                    
                }
            }
       }
        
        
    }
    
}

void EnvelopeVisualizer::mouseUp(const MouseEvent& e)
{
    isDraggingPoint = false;
    isDraggingStartPoint = false;
    isDraggingEndPoint = false;
    env.resetIndex();
    putValuesInTable();
}

void EnvelopeVisualizer::mouseMove(const MouseEvent& e)
{
    
}

void EnvelopeVisualizer::mouseDoubleClick(const MouseEvent& e)
{
    if(getLocalBounds().reduced(3.f).contains(e.x, e.y))
    {
        
        mousePosition.setXY(e.x, e.y);
        auto* newPoint = envPoints.add(new EnvelopeControlPoint<float>);
        newPoint->setPosition(e.x, e.y);
        newPoint->setSize(pointSize, pointSize);
        repaint();
        //envChange = true;
        //env.debugBuffer();
    }
        
}

void EnvelopeVisualizer::mouseDrag(const MouseEvent& e)
{
    if(reducedFrame.contains(e.x, e.y))
    {
        envChange = true;
        mousePosition.setXY(e.x, e.y);
        
//        auto y = jlimit(5, frame.getHeight(), e.y);
//        auto x = jlimit(5, frame.getWidth(), e.x);
        
        if (isDraggingStartPoint)
        {
            startPoint.setCentre(reducedFrame.getX(), e.y);
        }
        else if (isDraggingEndPoint)
        {
            endPoint.setCentre(reducedFrame.getX() + reducedFrame.getWidth(), e.y);
        }
        else if (isDraggingPoint)
        {
//            selectedPoint->setX(e.x);
//            selectedPoint->setY(e.y);
            selectedPoint->setCentre(e.x, e.y);
            
            // check to make sure no dots have the same x value
            for(size_t i = 0; i < envPoints.size(); ++i)
            {
                auto* point = envPoints[i]; // point we are checking against
                if (point->containsWithinSection(selectedPoint->getCentre()) && !point->isSelected())
                {
                    selectedPoint->translate(5.f, 0.f);
                }
            }
        }
    }
    repaint();
    
}

void EnvelopeVisualizer::putValuesInTable()
{
    sortEnvPoints();
    envPoints.insert(0, &startPoint); //  add start / end point to the array for this calculation only
    envPoints.add(&endPoint);
    
    int xDelt = 0;
    for (size_t i = 1; i < envPoints.size(); ++i)
    {
        
        auto height = reducedFrame.getHeight();
        auto y1 = height - envPoints[i-1]->getCentreY();
        auto y2 = height - envPoints[i]->getCentreY();
        float val1 = y1 / height;
        //DBG(val1);
        float val2 = y2 / height;
        //DBG(val2);
        float yDelta = val2 - val1; // change from previous point to current point in y axis scaled 0 - 1
        
        auto width = reducedFrame.getWidth();
        auto x1 = envPoints[i-1]->getCentre().getX();
        
        auto x2 = envPoints[i]->getCentre().getX();
        
        
        auto envelopeWidth = endPoint.getCentre().getX() - startPoint.getCentre().getX();
        float xDeltaPercent = (x2 - x1) / envelopeWidth; // percent of x axis this section is
        
        float xDeltaSamples = xDeltaPercent * 1024; // same percent applied to the 48000 samples in buffer
        float yDeltaPerSample = yDelta / xDeltaSamples; // how much change in amp 0 - 1 over the section
        
        xDelt += (int) xDeltaSamples;
        
        
        for (int j = 0; j <= (int) xDeltaSamples; ++j)
        {
            float yValue = val1 + (yDeltaPerSample * j);
            
            if(y1 < 0.01)
                yValue = 0;
            if(yValue > 0.99)
                yValue = 1;
            
            env.setValueAtCurrentIndex(yValue);
        }
        // interpolate from y1 to y2 over xDeltaSamples
    }
    int hangoverSamples = env.getTable().getNumSamples() - xDelt;
    
    for (int i = 0; i < hangoverSamples; ++i)
    {
       env.setValueAtCurrentIndex(0.f);
    }
    envPoints.remove(0, false); // remove start point
    envPoints.remove(envPoints.size()-1, false);
    
}

/*
    Used to pass the envelope over to the pulsar train and other things via the processor
*/
Envelope& EnvelopeVisualizer::getEnvelope()
{
    return env;
}

void EnvelopeVisualizer::sliderValueChanged(Slider* s)
{
    // % change in slider reflected as % change in points
    auto currentSliderVal = s->getValue();
    
    
    
    auto height = frame.getHeight();
    
    auto change = (currentSliderVal - lastSliderVal) * height;
    
    auto y1 = startPoint.getBottom() - change;


    if (y1 > height - 5)
        y1 = height - 5;
    if (y1 < 5)
        y1 = 5;

    startPoint.setY(y1);

    auto y2 = endPoint.getBottom() - change;


    if (y2 > height - 5)
        y2 = height - 5;
    if (y2 < 5)
        y2 = 5;

    endPoint.setY(y2);
    
    for (size_t i = 0; i < envPoints.size(); ++i)
    {
        auto y3 = envPoints[i]->getY() - change;
        //y3 = envPoints[i]->getY() - y3;
        
        if (y3 > height - 5)
            y3 = height - 5;
        
        if (y3 < 5)
            y3 = 5;
        
        envPoints[i]->setY(y3);

    }
    
    repaint();
    putValuesInTable();
    //audioProcessor.update();

    //env.setSliderVal(currentSliderVal);
    
    lastSliderVal = s->getValue();
    

}
