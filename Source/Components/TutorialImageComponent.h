/*
  ==============================================================================

    TutorialImageComponent.h
    Created: 12 Aug 2021 5:19:35pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../LookNFeel/PulsarUIFeel.h"

//==============================================================================
/*
*/
class TutorialImageComponent  : public juce::ImageComponent, public Button::Listener
{
public:
    TutorialImageComponent()
    {
        setLookAndFeel(&pulsarFeel);

        nextImageButton = std::make_unique<TextButton>("Next");
        addAndMakeVisible(nextImageButton.get());
        nextImageButton->setBoundsRelative(0.83, 0.9, 0.15, 0.05);
        nextImageButton->setAlwaysOnTop(true);
        nextImageButton->addListener(this);
        nextImageButton->setColour(juce::TextButton::buttonColourId, juce::Colours::orange);

        prevImageButton = std::make_unique<TextButton>("Prev");
        addAndMakeVisible(prevImageButton.get());
        prevImageButton->setBoundsRelative(0.02, 0.9, 0.15, 0.05);
        prevImageButton->setAlwaysOnTop(true);
        prevImageButton->addListener(this);
        prevImageButton->setColour(juce::TextButton::buttonColourId, juce::Colours::orange);

        getNumImages();
        loadSlides();

    }

    ~TutorialImageComponent() override
    {
    }

    void buttonClicked(Button* b) override
    {
        if (b == nextImageButton.get())
        {
            loadNextImage();
        }
        if (b == prevImageButton.get())
        {
            loadPreviousImage();
        }
    }

    void loadNextImage()
    {
        currentSlide++;
        currentSlide = jlimit<int>(0, getNumImages(), currentSlide);
        setImageFromIndex(currentSlide);
    }

    void loadPreviousImage()
    {
        currentSlide--;
        currentSlide = jlimit<int>(0, getNumImages(), currentSlide);
        setImageFromIndex(currentSlide);
    }

    void paint(Graphics& g) override
    {
        g.setOpacity(1.0f);
        g.drawImage(getImage(), getLocalBounds().toFloat(), getImagePlacement());
    }
    void resized() override
    {
        nextImageButton->setBoundsRelative(0.83, 0.9, 0.15, 0.05);
        prevImageButton->setBoundsRelative(0.02, 0.9, 0.15, 0.05);
    }

    void start()
    {
        setImageFromIndex(0);
    }

    void setImageFromIndex(int index)
    {
        auto file = juce::File(getFilePathFromIndex(index));
        auto image = ImageFileFormat::loadFrom(file);
        setImage(image);
    }

    int getNumImages()
    {
        return filePaths.size() - 1;

    }

    void loadSlides()
    {
        auto slides = juce::File("C:/ProgramData/Recluse-Audio/Pulsar/Pulsar Overview/").findChildFiles(2, true);

        for (int i = 0; i < slides.size(); i++)
        {
            filePaths.add(slides[i].getFullPathName());
        }
    }

    juce::String& getFilePathFromIndex(int index)
    {
        return filePaths.getReference(index);
    }

    
private:
    int currentSlide = 0;

    std::unique_ptr<TextButton> nextImageButton;
    std::unique_ptr<TextButton> prevImageButton;

    PulsarUIFeel pulsarFeel;

    juce::StringArray filePaths;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TutorialImageComponent)
};
