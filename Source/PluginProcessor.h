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

    // Spectrum analyzer access for GUI
    void getSpectrumData(std::vector<float>& spectrumData);
    static constexpr int spectrumSize = 512; // Number of frequency bins for display

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

    // Spectrum analyzer variables
    static constexpr int fftOrder = 11;         // 2^11 = 2048 samples
    static constexpr int fftSize = 1 << fftOrder; // 2048

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize> fftData;
    std::array<float, fftSize * 2> fftBuffer; // Complex FFT needs 2x size
    std::vector<float> spectrumMagnitudes;

    juce::AbstractFifo abstractFifo;
    std::array<float, fftSize> fifoBuffer;

    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    juce::CriticalSection spectrumDataMutex;
    std::vector<float> smoothedSpectrum;

    // Helper methods for LUFS calculation
    void updateLUFSMeasurements(const juce::AudioBuffer<float>& buffer);
    float calculateSimpleLUFS(const juce::AudioBuffer<float>& buffer, int numSamplesToUse) const;

    // Helper methods for spectrum analysis
    void pushSamplesToFifo(const juce::AudioBuffer<float>& buffer);
    void performFFT();
    void updateSpectrum();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackTweakAudioProcessor)
};