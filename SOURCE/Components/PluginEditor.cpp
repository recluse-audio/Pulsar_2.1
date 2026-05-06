/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "../Processor/PluginProcessor.h"
#include "PluginEditor.h"

PulsarAudioProcessorEditor::PulsarAudioProcessorEditor(PulsarAudioProcessor& p)
: AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&pulsarFeel);

    mainView = std::make_unique<PluginMainView>(audioProcessor);
    addAndMakeVisible(mainView.get());

    presetManagerView = std::make_unique<PresetManagerView>(audioProcessor);
    addChildComponent(presetManagerView.get());
    presetManagerView->setAlwaysOnTop(true);

    createPresetView = std::make_unique<CreatePresetView>(audioProcessor);
    addChildComponent(createPresetView.get());
    createPresetView->setAlwaysOnTop(true);
    createPresetView->onSaved = [this] { presetManagerView->refresh(); };

    mainView->getPresetButton().onClick = [this]
    {
        presetManagerView->refresh();
        presetManagerView->setVisible(true);
        presetManagerView->toFront(true);
    };

    mainView->getSaveButton().onClick = [this]
    {
        createPresetView->setVisible(true);
        createPresetView->toFront(true);
    };

    setResizable(true, true);
    if (auto* c = getConstrainer())
    {
        c->setFixedAspectRatio((double) PluginMainView::kDesignWidth
                               / (double) PluginMainView::kDesignHeight);
        c->setSizeLimits(400, 300, 1600, 1200);
    }

    setSize(PluginMainView::kDesignWidth, PluginMainView::kDesignHeight);

    setWantsKeyboardFocus(true);
    startTimerHz(120);
}

PulsarAudioProcessorEditor::~PulsarAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void PulsarAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void PulsarAudioProcessorEditor::resized()
{
    const float scale = (float) getWidth() / (float) PluginMainView::kDesignWidth;
    const auto t = juce::AffineTransform::scale(scale);

    if (mainView != nullptr)
    {
        mainView->setBounds(0, 0, PluginMainView::kDesignWidth, PluginMainView::kDesignHeight);
        mainView->setTransform(t);
    }

    if (presetManagerView != nullptr)
    {
        presetManagerView->setBounds(0, 0, PluginMainView::kDesignWidth, PluginMainView::kDesignHeight);
        presetManagerView->setTransform(t);
    }

    if (createPresetView != nullptr)
    {
        createPresetView->setBounds(0, 0, PluginMainView::kDesignWidth, PluginMainView::kDesignHeight);
        createPresetView->setTransform(t);
    }
}

void PulsarAudioProcessorEditor::timerCallback()
{
    if (mainView != nullptr)
        mainView->repaintPulsaret(audioProcessor.isFlashing() && audioProcessor.isTrainRunning(),
                                  audioProcessor.getFlashCoef());
}
