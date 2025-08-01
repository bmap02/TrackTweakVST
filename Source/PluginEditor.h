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

    TrackTweakAudioProcessor& audioProcessor;

    juce::Label rmsLabel;
    juce::Label tipLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackTweakAudioProcessorEditor)
};



