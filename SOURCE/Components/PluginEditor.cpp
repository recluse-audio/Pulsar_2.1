/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "../Processor/PluginProcessor.h"
#include "PluginEditor.h"
#include "../Util/Version.h"

//==============================================================================
PulsarAudioProcessorEditor::PulsarAudioProcessorEditor (PulsarAudioProcessor& p)
: AudioProcessorEditor (&p), pulsaretVisualizer(p.getTable()), audioProcessor (p)
{
    setSize(600, 450);
    frame = juce::Rectangle<int>(15, 50, getWidth() - 30, getHeight() - 65);

    setLookAndFeel(&pulsarFeel);

    pulsaretVisualizer.setBounds(frame);
    addAndMakeVisible(pulsaretVisualizer);

    resetSliders();
    init();

    presetButton = std::make_unique<juce::TextButton>("PRESETS");
    addAndMakeVisible(presetButton.get());
    presetButton->setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    presetButton->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    presetButton->setAlwaysOnTop(true);
    presetButton->onClick = [this]
    {
        presetManagerView->refresh();
        presetManagerView->setVisible(true);
        presetManagerView->toFront(true);
    };

    savePresetButton = std::make_unique<juce::TextButton>("SAVE AS PRESET");
    addAndMakeVisible(savePresetButton.get());
    savePresetButton->setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    savePresetButton->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    savePresetButton->setAlwaysOnTop(true);
    savePresetButton->onClick = [this]
    {
        createPresetView->setVisible(true);
        createPresetView->toFront(true);
    };

    presetManagerView = std::make_unique<PresetManagerView>(audioProcessor);
    addChildComponent(presetManagerView.get());
    presetManagerView->setAlwaysOnTop(true);

    createPresetView = std::make_unique<CreatePresetView>(audioProcessor);
    addChildComponent(createPresetView.get());
    createPresetView->setAlwaysOnTop(true);
    createPresetView->onSaved = [this] { presetManagerView->refresh(); };

    resized();

    setWantsKeyboardFocus(true);
    startTimerHz(120);
}

PulsarAudioProcessorEditor::~PulsarAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void PulsarAudioProcessorEditor::init()
{
    nameLabel = std::make_unique<juce::Label>("", "P U L S A R");
    addAndMakeVisible(nameLabel.get());
    nameLabel->setBoundsRelative(0.4f, 0.f, 0.3f, 0.1f);
    nameLabel->setFont(juce::Font(juce::FontOptions("Consolas", "Regular", 20.f)));
    nameLabel->setLookAndFeel(&nameFeel);
    nameLabel->setColour(juce::Label::textColourId, juce::Colours::white);

    versionLabel = std::make_unique<juce::Label>("", "v" BUILD_VERSION_STRING);
    addAndMakeVisible(versionLabel.get());
    versionLabel->setBoundsRelative(0.4f, 0.08f, 0.3f, 0.05f);
    versionLabel->setFont(juce::Font(juce::FontOptions("Consolas", "Regular", 10.f)));
    versionLabel->setColour(juce::Label::textColourId, juce::Colours::grey);
}

void PulsarAudioProcessorEditor::resetSliders()
{
    controlComponent = std::make_unique<PulsarControlsComponent>(audioProcessor);
    addAndMakeVisible(controlComponent.get());
    controlComponent->setBoundsRelative(0.f, 0.f, 1.f, 1.f);

    fundamentalSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Fundamental, "Fund", true, "Grid", false);
    addAndMakeVisible(fundamentalSlider.get());
    fundamentalSlider->setBoundsRelative(0.05f, 0.15f, 0.15f, 0.45f);
    using namespace Pulsar;

    fundamentalSlider->attachToState(audioProcessor.apvts,
        kFundamentalFreqID, kFundamentalSpreadID,
        kFundamentalRandomID, kRhythmicGridModeID);

    formantSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Formant, "Form", true, "Key", true);
    addAndMakeVisible(formantSlider.get());
    formantSlider->setBoundsRelative(0.2f, 0.15f, 0.15f, 0.45f);
    formantSlider->attachToState(audioProcessor.apvts,
        kFormantFreqID, kFormantSpreadID,
        kFormantRandomID, kFormantKeylockModeID,
        kFormKeyID, kFormScaleID);

    formantSlider2 = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Formant, "Form2", true, "Key", true);
    addAndMakeVisible(formantSlider2.get());
    formantSlider2->setBoundsRelative(0.35f, 0.15f, 0.15f, 0.45f);
    formantSlider2->attachToState(audioProcessor.apvts,
        kFormantFreq2ID, kFormantSpread2ID,
        kFormantRandom2ID, kFormant2KeylockModeID,
        kFormKey2ID, kFormScale2ID);

    waveSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Standard, "Wave",
        false, "", false);
    addAndMakeVisible(waveSlider.get());
    waveSlider->setBoundsRelative(0.5f, 0.15f, 0.15f, 0.45f);
    waveSlider->attachToState(audioProcessor.apvts,
        kWaveTypeID, kWaveSpreadID, kWaveRandomID);

    ampSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Standard, "Amp",
        false, "", false);
    addAndMakeVisible(ampSlider.get());
    ampSlider->setBoundsRelative(0.65f, 0.15f, 0.15f, 0.45f);
    ampSlider->attachToState(audioProcessor.apvts,
        kAmpID, kAmpSpreadID, kAmpRandomID);

    panSlider = std::make_unique<PulsarSlider>(audioProcessor, SliderType::Horizontal, "Pan",
        false, "", false);
    addAndMakeVisible(panSlider.get());
    panSlider->setBoundsRelative(0.25f, 0.625f, 0.55f, 0.13f);
    panSlider->attachToState(audioProcessor.apvts,
        kPanID, kPanSpreadID, kPanRandomID);
}

//==============================================================================
void PulsarAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setColour(color);
    g.fillAll();

    g.setColour(juce::Colours::yellow);
    g.drawRoundedRectangle(frame.toFloat(), 5.f, 2.f);

    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(getLocalBounds().reduced(5).toFloat(), 5.f, 2.f);
}

void PulsarAudioProcessorEditor::resized()
{
    if (savePresetButton != nullptr)
        savePresetButton->setBounds(getWidth() - 250, 15, 130, 24);

    if (presetButton != nullptr)
        presetButton->setBounds(getWidth() - 110, 15, 90, 24);

    if (presetManagerView != nullptr)
        presetManagerView->setBounds(getLocalBounds());

    if (createPresetView != nullptr)
        createPresetView->setBounds(getLocalBounds());
}

void PulsarAudioProcessorEditor::repaintPulsaret()
{
    if (audioProcessor.isFlashing() && audioProcessor.isTrainRunning())
    {
        pulsaretVisualizer.setNewWaveColour(juce::Colours::mediumvioletred.withHue(0.5f));
        pulsaretVisualizer.setAmp(audioProcessor.getFlashCoef());
        pulsaretVisualizer.repaint();
    }
    else
    {
        pulsaretVisualizer.setNewWaveColour(juce::Colours::black);
        pulsaretVisualizer.repaint();
    }
}

void PulsarAudioProcessorEditor::timerCallback()
{
    repaint();
    repaintPulsaret();
}
