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
    // Setup section titles
    addAndMakeVisible(rmsTitle);
    rmsTitle.setText("RMS LEVEL", juce::dontSendNotification);
    rmsTitle.setJustificationType(juce::Justification::centred);
    rmsTitle.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    rmsTitle.setColour(juce::Label::textColourId, juce::Colours::lightblue);

    addAndMakeVisible(lufsTitle);
    lufsTitle.setText("LUFS LOUDNESS", juce::dontSendNotification);
    lufsTitle.setJustificationType(juce::Justification::centred);
    lufsTitle.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    lufsTitle.setColour(juce::Label::textColourId, juce::Colours::lightgreen);

    // Setup RMS label
    addAndMakeVisible(rmsLabel);
    rmsLabel.setText("RMS: 0.000", juce::dontSendNotification);
    rmsLabel.setJustificationType(juce::Justification::centred);
    rmsLabel.setFont(juce::FontOptions(12.0f));

    // Setup LUFS labels
    addAndMakeVisible(momentaryLUFSLabel);
    momentaryLUFSLabel.setText("Momentary: -70.0 LUFS", juce::dontSendNotification);
    momentaryLUFSLabel.setJustificationType(juce::Justification::centred);
    momentaryLUFSLabel.setFont(juce::FontOptions(12.0f));

    addAndMakeVisible(shortTermLUFSLabel);
    shortTermLUFSLabel.setText("Short-term: -70.0 LUFS", juce::dontSendNotification);
    shortTermLUFSLabel.setJustificationType(juce::Justification::centred);
    shortTermLUFSLabel.setFont(juce::FontOptions(12.0f));

    addAndMakeVisible(integratedLUFSLabel);
    integratedLUFSLabel.setText("Integrated: -70.0 LUFS", juce::dontSendNotification);
    integratedLUFSLabel.setJustificationType(juce::Justification::centred);
    integratedLUFSLabel.setFont(juce::FontOptions(12.0f));

    // Setup tip label
    addAndMakeVisible(tipLabel);
    tipLabel.setText("Tip: Waiting for signal...", juce::dontSendNotification);
    tipLabel.setJustificationType(juce::Justification::centred);
    tipLabel.setFont(juce::FontOptions(11.0f));
    tipLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);

    // Start timer to update display (30 FPS)
    startTimer(33);

    setSize(400, 400); // Made taller to fit all the LUFS displays
}

TrackTweakAudioProcessorEditor::~TrackTweakAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TrackTweakAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Modern gradient background
    juce::ColourGradient gradient(juce::Colour(0xff1a1a1a), 0, 0,
        juce::Colour(0xff2d2d30), 0, getHeight(), false);
    g.setGradientFill(gradient);
    g.fillAll();

    // Plugin title
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.drawFittedText("TrackTweak Loudness Meter", getLocalBounds().removeFromTop(45),
        juce::Justification::centred, 1);

    // Draw section separators
    g.setColour(juce::Colours::grey.withAlpha(0.3f));
    g.drawHorizontalLine(120, 20, getWidth() - 20);
    g.drawHorizontalLine(260, 20, getWidth() - 20);
}

void TrackTweakAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(50); // Title space

    // RMS section
    rmsTitle.setBounds(bounds.removeFromTop(25).reduced(10, 0));
    rmsLabel.setBounds(bounds.removeFromTop(30).reduced(10, 0));
    bounds.removeFromTop(15); // Spacing

    // LUFS section
    lufsTitle.setBounds(bounds.removeFromTop(25).reduced(10, 0));
    momentaryLUFSLabel.setBounds(bounds.removeFromTop(25).reduced(10, 0));
    shortTermLUFSLabel.setBounds(bounds.removeFromTop(25).reduced(10, 0));
    integratedLUFSLabel.setBounds(bounds.removeFromTop(25).reduced(10, 0));
    bounds.removeFromTop(20); // Spacing

    // Tip section
    tipLabel.setBounds(bounds.removeFromTop(60).reduced(10, 5));
}

// Timer callback: updates all loudness values and mixing tips
void TrackTweakAudioProcessorEditor::timerCallback()
{
    // Get current values
    float rms = audioProcessor.getRMSLevel();
    float momentaryLUFS = audioProcessor.getMomentaryLUFS();
    float shortTermLUFS = audioProcessor.getShortTermLUFS();
    float integratedLUFS = audioProcessor.getIntegratedLUFS();

    // Update RMS display
    rmsLabel.setText("RMS: " + juce::String(rms, 3), juce::dontSendNotification);

    // Update LUFS displays
    momentaryLUFSLabel.setText("Momentary: " + juce::String(momentaryLUFS, 1) + " LUFS",
        juce::dontSendNotification);
    shortTermLUFSLabel.setText("Short-term: " + juce::String(shortTermLUFS, 1) + " LUFS",
        juce::dontSendNotification);
    integratedLUFSLabel.setText("Integrated: " + juce::String(integratedLUFS, 1) + " LUFS",
        juce::dontSendNotification);

    // Color-code short-term LUFS based on broadcast standards
    juce::Colour lufsColor = juce::Colours::white;
    if (shortTermLUFS > -14.0f)
        lufsColor = juce::Colours::red;      // Too loud for streaming
    else if (shortTermLUFS > -16.0f)
        lufsColor = juce::Colours::orange;   // Getting loud
    else if (shortTermLUFS > -23.0f)
        lufsColor = juce::Colours::green;    // Good range
    else if (shortTermLUFS > -30.0f)
        lufsColor = juce::Colours::yellow;   // Quiet
    else
        lufsColor = juce::Colours::grey;     // Very quiet

    shortTermLUFSLabel.setColour(juce::Label::textColourId, lufsColor);

    // Update mixing advice based on LUFS (more professional than RMS)
    tipLabel.setText(getLUFSAdvice(shortTermLUFS), juce::dontSendNotification);
}

juce::String TrackTweakAudioProcessorEditor::getLUFSAdvice(float lufs) const
{
    if (lufs <= -50.0f)
        return "     No signal detected";
    else if (lufs <= -30.0f)
        return "Tip: Signal very quiet - check your gain staging";
    else if (lufs <= -23.0f)
        return "Tip: Good for dialogue/quiet content";
    else if (lufs <= -16.0f)
        return "Tip: Perfect for streaming platforms (Spotify: -14 LUFS)";
    else if (lufs <= -14.0f)
        return "Tip: Approaching streaming loudness target";
    else if (lufs <= -11.0f)
        return "Tip: Too loud for streaming - will be limited";
    else
        return "Tip: Very loud! Risk of distortion and limiting";
}