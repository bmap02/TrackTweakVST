/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class TrackTweakAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    TrackTweakAudioProcessorEditor(TrackTweakAudioProcessor&);
    ~TrackTweakAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    juce::String getLUFSAdvice(float lufs) const;

    TrackTweakAudioProcessor& audioProcessor;

    // Display labels
    juce::Label rmsLabel;
    juce::Label momentaryLUFSLabel;
    juce::Label shortTermLUFSLabel;
    juce::Label integratedLUFSLabel;
    juce::Label tipLabel;

    // Section titles
    juce::Label rmsTitle;
    juce::Label lufsTitle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackTweakAudioProcessorEditor)
};