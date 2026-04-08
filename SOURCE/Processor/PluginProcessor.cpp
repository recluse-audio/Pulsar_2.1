/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "../Processor/PluginProcessor.h"
#include "../Components/PluginEditor.h"

//==============================================================================
PulsarAudioProcessor::PulsarAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts(*this, nullptr, "Params", createParams())
#endif
{
    //
    apvts.state.addListener(this);
    

}

PulsarAudioProcessor::~PulsarAudioProcessor()
{
}



//==============================================================================
const juce::String PulsarAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PulsarAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PulsarAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PulsarAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PulsarAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PulsarAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PulsarAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PulsarAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PulsarAudioProcessor::getProgramName (int index)
{
    return {};
}

void PulsarAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PulsarAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    pulsarTrain.prepare(sampleRate);
    update();
 }

void PulsarAudioProcessor::releaseResources()
{
    
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PulsarAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PulsarAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    

    if (pulsarTrain.isSynchronized())
    {
        playHead = this->getPlayHead();
        playHead->getCurrentPosition(currentPos);
        pulsarTrain.setTempo(currentPos.bpm, currentPos.timeSigNumerator, currentPos.timeSigDenominator);
    }

    if (mustUpdateProcessing)
    {
        update();
    }

    for (auto it = midiMessages.findNextSamplePosition (0); it != midiMessages.cend(); ++it)
    {
        const auto metadata = *it;

        if (metadata.samplePosition >= buffer.getNumSamples())
            break;

        auto message = metadata.getMessage();

        if (message.isNoteOn() || message.isSustainPedalOn())
        {
            pulsarTrain.triggerEnv();
        }
        if (message.isNoteOff() || message.isSustainPedalOff())
        {
            pulsarTrain.triggerRelease();
        }

    }
    pulsarTrain.generateNextBlock(buffer);
}


void PulsarAudioProcessor::handleMidi(juce::MidiBuffer midiBuffer)
{
   
}


//==============================================================================
bool PulsarAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PulsarAudioProcessor::createEditor()
{
    return new PulsarAudioProcessorEditor (*this);
}


void PulsarAudioProcessor::savePreset(juce::String& presetPath)
{
    MemoryBlock block;
    auto file = juce::File(presetPath);
    getStateInformation(block); // copies xml state to this 'block' a binary blob
    file.replaceWithData(block.getData(), block.getSize());

    // need to call getStateInformation(MemoryBlock& destData) and somehow associate the presetPath with teh memory block
}

//==============================================================================
void PulsarAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree copyState = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml = copyState.createXml();
    copyXmlToBinary (*xml.get(), destData); 
}
/*--------------------------------------------------------------------*/

void PulsarAudioProcessor::loadPreset(juce::String& presetPathName)
{
    // want to call setStateInformation(const void* data, int sizeInBytes)
    // file to memory block?
    MemoryBlock block;
    auto file = juce::File(presetPathName);
    file.loadFileAsData(block); // fills block with files data.  See setStateInformation() in Audio Processor Base class
    setStateInformation(block.getData(), (int)block.getSize());
    mustUpdateProcessing = true;
}

void PulsarAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml = getXmlFromBinary (data, sizeInBytes);
    juce::ValueTree copyState = juce::ValueTree::fromXml (*xml.get());
    apvts.replaceState (copyState);
}





//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PulsarAudioProcessor();
}



void PulsarAudioProcessor::update()
{
    mustUpdateProcessing = false;
    
    pulsarTrain.update(apvts);
    

    auto rhythmic = apvts.getRawParameterValue("Rhythmic Grid Mode");
}



/*=============================================================================================*/
/*=============================================================================================*/
void PulsarAudioProcessor::valueTreePropertyChanged(ValueTree& tree, const Identifier& property)
{
    mustUpdateProcessing = true;
}


/*=============================================================================================*/
/*===================================== FUNDAMENTAL ===========================================*/
juce::AudioProcessorValueTreeState::ParameterLayout PulsarAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Fundamental Freq", "Fundamental Freq",
                                                                NormalisableRange<float> (1.f, 200.f, 1.f, 0.5f), 5.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Fundamental Spread", "Fundamental Spread",
                                                                NormalisableRange<float> (1.f, 10.f, 0.001f, 0.5f), 1.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Fundamental Random", "Fundamental Random",
                                                                NormalisableRange<float> (0.f, 1.f, 0.001f, 1.f), 0.f));
    
    parameters.push_back(std::make_unique<AudioParameterBool>("Rhythmic Grid Mode", "Rhythmic Grid Mode", false));

    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Formant Freq", "Formant Freq",
                                                                NormalisableRange<float> (100.f, 10000.f, 1.f, 0.3f), 600.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Formant Spread", "Formant Spread",
                                                                NormalisableRange<float> (1.f, 10.f, 0.001f, 1.f), 1.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Formant Random", "Formant Random",
                                                                NormalisableRange<float> (0.f, 1.f, 0.001f, 1.f), 0.f));
    
    parameters.push_back(std::make_unique<AudioParameterBool>("Formant Keylock Mode", "Formant Keylock Mode", false));
    
    parameters.push_back(std::make_unique<AudioParameterInt>("Form Key", "Form Key", 0, 11, 1));

    parameters.push_back(std::make_unique<AudioParameterInt>("Form Scale", "Form Scale", 0, 5, 1));

    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Formant Freq2", "Formant Freq2",
                                                                NormalisableRange<float> (100.f, 10000.f, 1.f, 0.3f), 800.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Formant Spread2", "Formant Spread2",
                                                                NormalisableRange<float> (1.f, 10.f, 0.001f, 1.f), 1.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Formant Random2", "Formant Random2",
                                                                NormalisableRange<float> (0.f, 1.f, 0.001f, 1.f), 0.f));
    
    parameters.push_back(std::make_unique<AudioParameterBool>("Formant2 Keylock Mode", "Formant2 Keylock Mode", false));

    parameters.push_back(std::make_unique<AudioParameterInt>("Form Key2", "Form Key2", 0, 11, 1));

    parameters.push_back(std::make_unique<AudioParameterInt>("Form Scale2", "Form Scale2", 0, 5, 1));

    
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Pan", "Pan",
                                                                NormalisableRange<float> (0.0f, 100.f, 0.001f, 1.f), 50.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Pan Spread", "Pan Spread",
                                                                NormalisableRange<float> (1.f, 100.f, 0.01f, 1.f), 1.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Pan Random", "Pan Random",
                                                                NormalisableRange<float> (0.f, 1.f, 0.01f, 1.f), 0.f));
    
    
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Wave Type", "Wave Type",
                                                                NormalisableRange<float> (0.f, 99.f, 0.01f, 1.f), 50.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Wave Spread", "Wave Spread",
                                                                NormalisableRange<float> (1.f, 100.f, 0.1f, 1.f), 1.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Wave Random", "Wave Random",
                                                                NormalisableRange<float> (0.f, 1.f, 0.001f, 1.f), 0.f));
    
    
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Amp", "Amp",
                                                                NormalisableRange<float> (0.0f, 100.f, 0.1f, 1.f), 50.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Amp Spread", "Amp Spread",
                                                                NormalisableRange<float> (1.f, 100.f, 0.1f, 1.f), 1.f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Amp Random", "Amp Random",
                                                                NormalisableRange<float> (0.f, 1.f, 0.001f, 1.f), 0.00f));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Intermittance", "Intermittance",
                                                                NormalisableRange<float> (0.f, 1.f, 0.001f, 1.f), 1.f));
    
    parameters.push_back (std::make_unique<AudioParameterInt>("Trigger On", "Trigger On", 1, 10, 1));
    
    parameters.push_back (std::make_unique<AudioParameterInt>("Trigger Off", "Trigger Off", 0, 10, 0));
    
    parameters.push_back (std::make_unique<AudioParameterBool>("Trigger", "Trigger", false));
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Glide Time", "Glide Time", NormalisableRange<float> (10.f, 1000.f, 0.001f, 1.f), 50.f)); // in ms, converted later to sec
    
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Width", "Width",
                                                                NormalisableRange<float> (0.f, 1.f, 0.001f, 1.f), 0.0f));
    
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("Attack", "Attack",
                                                                NormalisableRange<float> (0.f, 10000.f, 1.f, 0.5f), 100.f));
    parameters.push_back (std::make_unique<AudioParameterFloat>("Decay", "Decay",
                                                                NormalisableRange<float> (0.f, 5000.f, 1.f, 0.5f), 50.f));
    parameters.push_back (std::make_unique<AudioParameterFloat>("Sustain Level", "Sustain Level",
                                                                NormalisableRange<float> (0.f, 1.f, 0.01f, 1.f), 0.5f));
    parameters.push_back (std::make_unique<AudioParameterFloat>("Release", "Release",
                                                                NormalisableRange<float> (0.f, 10000.f, 1.f, 0.5f), 500.f));
    
    
    return { parameters.begin(), parameters.end() };
}

void PulsarAudioProcessor::triggerPulsarTrain()
{
    pulsarTrain.triggerEnv();
}

void PulsarAudioProcessor::releasePulsarTrain()
{
    pulsarTrain.triggerRelease();
}

PulsaretTable& PulsarAudioProcessor::getPulsaretTable()
{
    return pulsarTrain.getPulsaretTable();
}

Table& PulsarAudioProcessor::getTable()
{
    return pulsarTrain.getTable();
}

bool PulsarAudioProcessor::isFlashing()
{
    return pulsarTrain.checkIfFlashing();
}

bool PulsarAudioProcessor::isTrainRunning()
{
    return pulsarTrain.trainIsRunning();
}

float PulsarAudioProcessor::getFlashCoef()
{
    
    return pulsarTrain.getFlashCoef();
}

void PulsarAudioProcessor::loadDefaultParameters()
{

}

