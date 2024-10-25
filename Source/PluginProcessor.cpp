/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <math.h>

//==============================================================================
CopePhattenerAudioProcessor::CopePhattenerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

CopePhattenerAudioProcessor::~CopePhattenerAudioProcessor()
{
}

//==============================================================================
const juce::String CopePhattenerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CopePhattenerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CopePhattenerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CopePhattenerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CopePhattenerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CopePhattenerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CopePhattenerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CopePhattenerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CopePhattenerAudioProcessor::getProgramName (int index)
{
    return {};
}

void CopePhattenerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CopePhattenerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // Set up your DSP modules here
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    
    osc.prepare (spec);
    bandPassFilter.prepare(spec);
}

void CopePhattenerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CopePhattenerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void CopePhattenerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    
    // Obtain settings from the AudioProcessorValueTreeState
    auto chainSettings = getChainSettings(apvts);
    float subFreq = chainSettings.subFreq;
    float subWet = chainSettings.subWet;
    float subThreshold = chainSettings.subThreshold;
    float noiseFreq = chainSettings.noiseFreq;
    float noiseWet = chainSettings.noiseWet;
    float noiseThreshold = chainSettings.noiseThreshold;
    float dry = chainSettings.dry;
    float volume = chainSettings.volume;

    juce::dsp::AudioBlock<float> audioBlock(buffer);
    auto leftBlock = audioBlock.getSingleChannelBlock(0);
    auto rightBlock = audioBlock.getSingleChannelBlock(1);
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    osc.setFrequency(subFreq);
    // *play oscillator with audio: (100% works but mixes completely with audio signal)
    //osc.process(juce::dsp::ProcessContextReplacing<float> (audioBlock));
    // Filter params
    float Q = 5.0f;
    bandPassFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(getSampleRate(), noiseFreq, Q);
    float subVal = 0;
    //int samplecount = buffer.getNumSamples();
    
    //Assured to be stereo audio.
    auto* channelData = buffer.getWritePointer(0);
    auto* channelData2 = buffer.getWritePointer(1);
    for (int sample = 0; sample < buffer.getNumSamples(); sample++)
    {
        float cleanSig = *channelData;
        float cleanSig2 = *channelData2;
        // ------- SUB OSC
        // *play oscillator by itself: (doesn't work but mixes to our needs)
        // channel = 0: left okay, right crackly, channel 1: right okay, left nothing

        subVal = osc.processSample(0.0f);
        
        // ------- WHITE NOISE
        float noiseValue = random.nextFloat() * 0.25f - 0.125f;
        noiseValue = bandPassFilter.processSample(noiseValue);
        // ------- SIDECHAINING
        //sub
        if (subSCValue <= 0)
        {
            subSCValue = 0;
        }
        if (*channelData > subThreshold and *channelData > subSCValue)
        {
            subSCValue = *channelData;
        }else{
            subSCValue -= 0.0001f;
        }
        //noise
        if (noiseSCValue <= 0)
        {
            noiseSCValue = 0;
        }
        if (*channelData > noiseThreshold and *channelData > noiseSCValue)
        {
            noiseSCValue = *channelData;
        }else{
            noiseSCValue -= 0.0001f;
        }
        
        // -------- MIX
        //*channelData = juce::jlimit(-1.0f, 1.0f, *channelData);
        *channelData = (cleanSig * dry + subVal * subSCValue * subWet + noiseValue * noiseSCValue * noiseWet) * volume;
        *channelData2 = (cleanSig2 * dry + subVal * subSCValue * subWet + noiseValue * noiseSCValue * noiseWet) * volume;
        //clip prevent
        *channelData = juce::jlimit(-1.0f, 1.0f, *channelData);
        *channelData2 = juce::jlimit(-1.0f, 1.0f, *channelData2);
        channelData++;
        channelData2++;
        
    }
}

//==============================================================================
bool CopePhattenerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CopePhattenerAudioProcessor::createEditor()
{
    return new CopePhattenerAudioProcessorEditor (*this);
    //EDITOR VIEW COMMENT
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void CopePhattenerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CopePhattenerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    settings.subFreq = apvts.getRawParameterValue("subFreq")->load();
    settings.subThreshold = apvts.getRawParameterValue("subThreshold")->load();
    settings.subWet = apvts.getRawParameterValue("subWet")->load();
    settings.noiseFreq = apvts.getRawParameterValue("noiseFreq")->load();
    settings.noiseThreshold = apvts.getRawParameterValue("noiseThreshold")->load();
    settings.noiseWet = apvts.getRawParameterValue("noiseWet")->load();
    settings.dry = apvts.getRawParameterValue("dry")->load();
    settings.volume = apvts.getRawParameterValue("volume")->load();
    
    return settings;
}

juce::AudioProcessorValueTreeState::ParameterLayout CopePhattenerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("subFreq",1),"subFreq",juce::NormalisableRange<float>(20.0f, 100.0f, 0.1f, 1.0f), 86.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("subThreshold",1),"subThreshold",juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("subWet",1),"subWet",juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f, 1.0f), 0.3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("noiseFreq",1),"noiseFreq",juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 1.0f), 10000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("noiseThreshold",1),"noiseThreshold",juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("noiseWet",1),"noiseWet",juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f, 1.0f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("dry",1),"dry",juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f, 1.0f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("volume",1),"volume",juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f), 0.5f));
    
    
    return layout;
};


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CopePhattenerAudioProcessor();
}
