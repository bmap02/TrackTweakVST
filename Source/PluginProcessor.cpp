/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TrackTweakAudioProcessor::TrackTweakAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
}

TrackTweakAudioProcessor::~TrackTweakAudioProcessor()
{
}

//==============================================================================
const juce::String TrackTweakAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TrackTweakAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool TrackTweakAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool TrackTweakAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double TrackTweakAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TrackTweakAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int TrackTweakAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TrackTweakAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String TrackTweakAudioProcessor::getProgramName(int index)
{
    return {};
}

void TrackTweakAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void TrackTweakAudioProcessor::prepareToPlay(double sr, int samplesPerBlock)
{
    // Store sample rate (renamed parameter to avoid hiding member variable)
    sampleRate = sr;

    // Prepare circular buffers for LUFS measurements
    int momentarySize = static_cast<int>(sr * 0.4); // 400ms for momentary LUFS
    int shortTermSize = static_cast<int>(sr * 3.0); // 3 seconds for short-term LUFS

    momentaryBuffer.setSize(2, momentarySize);
    shortTermBuffer.setSize(2, shortTermSize);

    momentaryBuffer.clear();
    shortTermBuffer.clear();

    momentaryWritePos = 0;
    shortTermWritePos = 0;
}

void TrackTweakAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TrackTweakAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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

void TrackTweakAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // --- RMS detection on first input channel only (keep existing)
    auto* channelData = buffer.getReadPointer(0);  // use left channel for analysis
    int numSamples = buffer.getNumSamples();

    float sumSquares = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        sumSquares += channelData[i] * channelData[i];

    currentRMSLevel.store(std::sqrt(sumSquares / static_cast<float>(numSamples)));

    // --- LUFS measurement (new)
    updateLUFSMeasurements(buffer);
}

//==============================================================================
void TrackTweakAudioProcessor::updateLUFSMeasurements(const juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = std::min(buffer.getNumChannels(), 2); // Limit to stereo

    // Copy samples into circular buffers
    for (int sample = 0; sample < numSamples; ++sample)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float inputSample = buffer.getSample(channel, sample);

            // Store in momentary buffer (400ms)
            momentaryBuffer.setSample(channel, momentaryWritePos, inputSample);

            // Store in short-term buffer (3s)
            shortTermBuffer.setSample(channel, shortTermWritePos, inputSample);
        }

        momentaryWritePos = (momentaryWritePos + 1) % momentaryBuffer.getNumSamples();
        shortTermWritePos = (shortTermWritePos + 1) % shortTermBuffer.getNumSamples();
    }

    // Calculate LUFS values and store atomically
    currentMomentaryLUFS.store(calculateSimpleLUFS(momentaryBuffer, momentaryBuffer.getNumSamples()));
    currentShortTermLUFS.store(calculateSimpleLUFS(shortTermBuffer, shortTermBuffer.getNumSamples()));
    currentIntegratedLUFS.store(currentShortTermLUFS.load()); // Simplified - use short-term for now
}

float TrackTweakAudioProcessor::calculateSimpleLUFS(const juce::AudioBuffer<float>& buffer, int numSamplesToUse) const
{
    float sum = 0.0f;
    int channels = std::min(buffer.getNumChannels(), 2);

    for (int channel = 0; channel < channels; ++channel)
    {
        const float* channelData = buffer.getReadPointer(channel);
        for (int i = 0; i < numSamplesToUse; ++i)
        {
            float sample = channelData[i];
            sum += sample * sample;
        }
    }

    float meanSquare = sum / (numSamplesToUse * channels);
    if (meanSquare <= 0.0f)
        return -70.0f; // Silence threshold

    // Convert to approximate LUFS (simplified - no K-weighting filter yet)
    // The -0.691 offset approximates some of the K-weighting effect
    return 10.0f * std::log10(meanSquare) - 0.691f;
}

//==============================================================================
float TrackTweakAudioProcessor::getRMSLevel() const
{
    return currentRMSLevel.load();
}

float TrackTweakAudioProcessor::getMomentaryLUFS() const
{
    return currentMomentaryLUFS.load();
}

float TrackTweakAudioProcessor::getShortTermLUFS() const
{
    return currentShortTermLUFS.load();
}

float TrackTweakAudioProcessor::getIntegratedLUFS() const
{
    return currentIntegratedLUFS.load();
}

//==============================================================================
bool TrackTweakAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TrackTweakAudioProcessor::createEditor()
{
    return new TrackTweakAudioProcessorEditor(*this);
}

//==============================================================================
void TrackTweakAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TrackTweakAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TrackTweakAudioProcessor();
}