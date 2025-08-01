/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TrackTweakAudioProcessorEditor::TrackTweakAudioProcessorEditor(TrackTweakAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Setup RMS label
    addAndMakeVisible(rmsLabel);
    rmsLabel.setText("RMS Level: 0.000", juce::dontSendNotification);
    rmsLabel.setJustificationType(juce::Justification::centred);

    // Setup tip label
    addAndMakeVisible(tipLabel);
    tipLabel.setText("Tip: Waiting for signal...", juce::dontSendNotification);
    tipLabel.setJustificationType(juce::Justification::centred);

    // Start timer to update RMS display (30 FPS)
    startTimer(33);

    setSize(400, 300);
}

TrackTweakAudioProcessorEditor::~TrackTweakAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TrackTweakAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(15.0f));
    g.drawFittedText("TrackTweak Plugin", getLocalBounds().removeFromTop(50), juce::Justification::centred, 1);
}

void TrackTweakAudioProcessorEditor::resized()
{
    // Layout for the RMS and tip labels
    rmsLabel.setBounds(10, 60, getWidth() - 20, 40);
    tipLabel.setBounds(10, 120, getWidth() - 20, 40);
}

// Timer callback: updates RMS value and mixing tip
void TrackTweakAudioProcessorEditor::timerCallback()
{
    float rms = audioProcessor.getRMSLevel();
    rmsLabel.setText("RMS Level: " + juce::String(rms, 3), juce::dontSendNotification);

    if (rms > 0.25f)
        tipLabel.setText("Tip: Your mix is loud – consider compression!", juce::dontSendNotification);
    else if (rms > 0.05f)
        tipLabel.setText("Tip: Mix levels are healthy.", juce::dontSendNotification);
    else
        tipLabel.setText("Tip: Signal is too quiet.", juce::dontSendNotification);
}