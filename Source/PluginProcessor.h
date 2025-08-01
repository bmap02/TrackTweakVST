/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin processor.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <atomic>

//==============================================================================
class TrackTweakAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    TrackTweakAudioProcessor();
    ~TrackTweakAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Loudness measurement access for GUI
    float getRMSLevel() const;
    float getMomentaryLUFS() const;
    float getShortTermLUFS() const;
    float getIntegratedLUFS() const;

private:
    //==============================================================================
    // RMS calculation variables
    std::atomic<float> currentRMSLevel{ 0.0f };

    // LUFS calculation variables
    std::atomic<float> currentMomentaryLUFS{ -70.0f };
    std::atomic<float> currentShortTermLUFS{ -70.0f };
    std::atomic<float> currentIntegratedLUFS{ -70.0f };

    // Simple circular buffers for LUFS timing
    juce::AudioBuffer<float> momentaryBuffer;
    juce::AudioBuffer<float> shortTermBuffer;
    int momentaryWritePos = 0;
    int shortTermWritePos = 0;
    double sampleRate = 44100.0;

    // Helper methods for LUFS calculation
    void updateLUFSMeasurements(const juce::AudioBuffer<float>& buffer);
    float calculateSimpleLUFS(const juce::AudioBuffer<float>& buffer, int numSamplesToUse) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackTweakAudioProcessor)
};