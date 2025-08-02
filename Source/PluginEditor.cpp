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
    // Setup section titles with professional styling
    addAndMakeVisible(rmsTitle);
    rmsTitle.setText("RMS LEVEL", juce::dontSendNotification);
    rmsTitle.setJustificationType(juce::Justification::centred);
    rmsTitle.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    rmsTitle.setColour(juce::Label::textColourId, juce::Colour(0xff4da6ff)); // Professional blue

    addAndMakeVisible(lufsTitle);
    lufsTitle.setText("LUFS LOUDNESS", juce::dontSendNotification);
    lufsTitle.setJustificationType(juce::Justification::centred);
    lufsTitle.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    lufsTitle.setColour(juce::Label::textColourId, juce::Colour(0xff66cc66)); // Professional green

    addAndMakeVisible(spectrumTitle);
    spectrumTitle.setText("SPECTRUM ANALYZER", juce::dontSendNotification);
    spectrumTitle.setJustificationType(juce::Justification::centred);
    spectrumTitle.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    spectrumTitle.setColour(juce::Label::textColourId, juce::Colour(0xffff9933)); // Professional orange

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
    tipLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffcc66)); // Soft yellow

    // Setup professional spectrum analyzer
    spectrumAnalyzer = std::make_unique<SpectrumAnalyzer>(audioProcessor);
    addAndMakeVisible(*spectrumAnalyzer);

    // Start timer to update display (30 FPS)
    startTimer(33);

    setSize(600, 650); // Optimized size for all components
}

TrackTweakAudioProcessorEditor::~TrackTweakAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TrackTweakAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Professional gradient background
    juce::ColourGradient gradient(juce::Colour(0xff1a1a1a), 0, 0,
        juce::Colour(0xff2d2d30), 0, getHeight(), false);
    g.setGradientFill(gradient);
    g.fillAll();

    // Professional plugin title
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawFittedText("TrackTweak Pro Analyzer", getLocalBounds().removeFromTop(45),
        juce::Justification::centred, 1);

    // FIXED: Adjusted separator line positions to match actual layout
    g.setColour(juce::Colours::grey.withAlpha(0.25f));
    g.drawHorizontalLine(130, 20, getWidth() - 20);  // After RMS
    g.drawHorizontalLine(245, 20, getWidth() - 20);  // After LUFS, before spectrum (adjusted position)
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
    bounds.removeFromTop(10); // Extra spacing to move title below line
    lufsTitle.setBounds(bounds.removeFromTop(25).reduced(10, 0));
    momentaryLUFSLabel.setBounds(bounds.removeFromTop(25).reduced(10, 0));
    shortTermLUFSLabel.setBounds(bounds.removeFromTop(25).reduced(10, 0));
    integratedLUFSLabel.setBounds(bounds.removeFromTop(25).reduced(10, 0));
    bounds.removeFromTop(15); // Spacing after LUFS

    // Spectrum section - title and analyzer both BELOW the line
    bounds.removeFromTop(10); // Space after separator line
    spectrumTitle.setBounds(bounds.removeFromTop(25).reduced(10, 0));
    bounds.removeFromTop(5); // Small spacing between title and analyzer
    spectrumAnalyzer->setBounds(bounds.removeFromTop(200).reduced(15, 0));
    bounds.removeFromTop(15); // Spacing

    // Tip section
    tipLabel.setBounds(bounds.removeFromTop(60).reduced(10, 5));
}

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

    // Professional color coding based on broadcast/streaming standards
    juce::Colour lufsColor = juce::Colours::white;
    if (shortTermLUFS > -14.0f)
        lufsColor = juce::Colour(0xffff4444);      // Red: Too loud for streaming
    else if (shortTermLUFS > -16.0f)
        lufsColor = juce::Colour(0xffff8844);      // Orange: Getting loud
    else if (shortTermLUFS > -23.0f)
        lufsColor = juce::Colour(0xff44ff44);      // Green: Perfect range
    else if (shortTermLUFS > -35.0f)
        lufsColor = juce::Colour(0xffffff44);      // Yellow: Quiet but OK
    else
        lufsColor = juce::Colour(0xff888888);      // Grey: Very quiet

    shortTermLUFSLabel.setColour(juce::Label::textColourId, lufsColor);

    // Update spectrum analyzer (repaints automatically)
    spectrumAnalyzer->repaint();

    // Intelligent advice based on content type and levels
    tipLabel.setText(getLUFSAdvice(shortTermLUFS), juce::dontSendNotification);
}

juce::String TrackTweakAudioProcessorEditor::getLUFSAdvice(float lufs) const
{
    // Intelligent advice for both music production and microphone input
    if (lufs <= -60.0f)
        return "Tip: No signal detected";
    else if (lufs <= -50.0f)
        return "Tip: Very quiet signal - check input gain";
    else if (lufs <= -35.0f)
        return "Tip: Good level for dialogue/vocals";
    else if (lufs <= -23.0f)
        return "Tip: Perfect for broadcast content (-23 LUFS standard)";
    else if (lufs <= -16.0f)
        return "Tip: Great for YouTube (-16 LUFS target)";
    else if (lufs <= -14.0f)
        return "Tip: Perfect for Spotify/streaming (-14 LUFS target)";
    else if (lufs <= -11.0f)
        return "Tip: Getting loud - watch for distortion";
    else if (lufs <= -8.0f)
        return "Tip: Very loud - platforms will apply limiting";
    else
        return "Tip: Signal too loud - reduce gain to prevent clipping";
}