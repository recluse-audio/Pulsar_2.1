#include "PluginMainView.h"
#include "../Util/Version.h"

PluginMainView::PluginMainView(PulsarAudioProcessor& p)
: audioProcessor(p), pulsaretVisualizer(p.getTable())
{
    addAndMakeVisible(pulsaretVisualizer);

    buildSliders();
    initLabels();

    for (auto* b : { &presetButton, &savePresetButton })
    {
        addAndMakeVisible(*b);
        b->setColour(juce::TextButton::buttonColourId,  juce::Colours::black);
        b->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }

    setSize(kDesignWidth, kDesignHeight);
}

PluginMainView::~PluginMainView() = default;

void PluginMainView::initLabels()
{
    nameLabel.setText("P U L S A R", juce::dontSendNotification);
    addAndMakeVisible(nameLabel);
    nameLabel.setFont(juce::Font(juce::FontOptions("Consolas", "Regular", 20.f)));
    nameLabel.setLookAndFeel(&nameFeel);
    nameLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    versionLabel.setText("v" BUILD_VERSION_STRING, juce::dontSendNotification);
    addAndMakeVisible(versionLabel);
    versionLabel.setFont(juce::Font(juce::FontOptions("Consolas", "Regular", 10.f)));
    versionLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
}

void PluginMainView::buildSliders()
{
    using namespace Pulsar;

    controlComponent = std::make_unique<PulsarControlsComponent>(audioProcessor);
    addAndMakeVisible(controlComponent.get());

    fundamentalSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Fundamental, "Fund", true, "Grid", false);
    addAndMakeVisible(fundamentalSlider.get());
    fundamentalSlider->attachToState(audioProcessor.apvts,
        kFundamentalFreqID, kFundamentalSpreadID,
        kFundamentalRandomID, kRhythmicGridModeID);

    formantSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Formant, "Form", true, "Key", true);
    addAndMakeVisible(formantSlider.get());
    formantSlider->attachToState(audioProcessor.apvts,
        kFormantFreqID, kFormantSpreadID,
        kFormantRandomID, kFormantKeylockModeID,
        kFormKeyID, kFormScaleID);

    formantSlider2 = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Formant, "Form2", true, "Key", true);
    addAndMakeVisible(formantSlider2.get());
    formantSlider2->attachToState(audioProcessor.apvts,
        kFormantFreq2ID, kFormantSpread2ID,
        kFormantRandom2ID, kFormant2KeylockModeID,
        kFormKey2ID, kFormScale2ID);

    waveSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Standard, "Wave", false, "", false);
    addAndMakeVisible(waveSlider.get());
    waveSlider->attachToState(audioProcessor.apvts,
        kWaveTypeID, kWaveSpreadID, kWaveRandomID);

    ampSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Standard, "Amp", false, "", false);
    addAndMakeVisible(ampSlider.get());
    ampSlider->attachToState(audioProcessor.apvts,
        kAmpID, kAmpSpreadID, kAmpRandomID);

    panSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Horizontal, "Pan", false, "", false);
    addAndMakeVisible(panSlider.get());
    panSlider->attachToState(audioProcessor.apvts,
        kPanID, kPanSpreadID, kPanRandomID);
}

void PluginMainView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::yellow);
    g.drawRoundedRectangle(frame.toFloat(), 5.f, 2.f);

    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(getLocalBounds().reduced(5).toFloat(), 5.f, 2.f);
}

void PluginMainView::resized()
{
    frame = juce::Rectangle<int>(15, 50, getWidth() - 30, getHeight() - 65);

    pulsaretVisualizer.setBounds(frame);

    nameLabel   .setBounds(240,  0, 180, 45);
    versionLabel.setBounds(240, 36, 180, 22);

    savePresetButton.setBounds(getWidth() - 250, 15, 130, 24);
    presetButton    .setBounds(getWidth() - 110, 15,  90, 24);

    controlComponent->setBounds(0, 0, getWidth(), getHeight());

    fundamentalSlider->setBounds( 30, 67,  90, 203);
    formantSlider    ->setBounds(120, 67,  90, 203);
    formantSlider2   ->setBounds(210, 67,  90, 203);
    waveSlider       ->setBounds(300, 67,  90, 203);
    ampSlider        ->setBounds(390, 67,  90, 203);
    panSlider        ->setBounds(150, 281, 330, 58);
}

void PluginMainView::repaintPulsaret(bool flashing, float flashCoef)
{
    if (flashing)
    {
        pulsaretVisualizer.setNewWaveColour(juce::Colours::mediumvioletred.withHue(0.5f));
        pulsaretVisualizer.setAmp(flashCoef);
    }
    else
    {
        pulsaretVisualizer.setNewWaveColour(juce::Colours::black);
    }
    pulsaretVisualizer.repaint();
}
