/*
  ==============================================================================
    This file contains the basic framework code for a JUCE plugin editor.
  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// FIXED: Professional Ableton-Style Spectrum Analyzer Component
class SpectrumAnalyzer : public juce::Component
{
public:
    SpectrumAnalyzer(TrackTweakAudioProcessor& p) : audioProcessor(p)
    {
        setOpaque(false);
    }

    void paint(juce::Graphics& g) override
    {
        // Professional dark background like Ableton
        g.fillAll(juce::Colour(0xff1a1a1a));

        // Subtle border
        g.setColour(juce::Colours::grey.withAlpha(0.4f));
        g.drawRect(getLocalBounds(), 1);

        // Get spectrum data
        std::vector<float> spectrumData;
        audioProcessor.getSpectrumData(spectrumData);

        if (spectrumData.empty()) return;

        auto bounds = getLocalBounds().toFloat();
        auto width = bounds.getWidth();
        auto height = bounds.getHeight();

        // IMPROVED: Professional grid system
        g.setColour(juce::Colours::grey.withAlpha(0.15f));

        // Frequency grid lines with better spacing
        std::vector<std::pair<float, juce::String>> freqMarkers = {
            {0.1f, "100"}, {0.2f, "200"}, {0.35f, "500"}, {0.5f, "1k"},
            {0.65f, "2k"}, {0.78f, "5k"}, {0.88f, "10k"}
        };

        for (const auto& marker : freqMarkers)
        {
            float x = width * marker.first;
            g.drawVerticalLine(static_cast<int>(x), 0, height);
        }

        // IMPROVED: dB grid with better range
        for (int dB = -80; dB <= 0; dB += 20)
        {
            float y = juce::jmap(static_cast<float>(dB), -80.0f, 0.0f, height, 0.0f);
            g.drawHorizontalLine(static_cast<int>(y), 0, width);
        }

        // Frequency labels
        g.setColour(juce::Colours::lightgrey.withAlpha(0.8f));
        g.setFont(juce::FontOptions(9.0f));

        for (const auto& marker : freqMarkers)
        {
            float x = width * marker.first;
            g.drawText(marker.second, static_cast<int>(x - 15), static_cast<int>(height - 15),
                30, 12, juce::Justification::centred);
        }

        // dB scale labels
        g.setFont(juce::FontOptions(8.0f));
        for (int dB = -60; dB <= 0; dB += 20)
        {
            float y = juce::jmap(static_cast<float>(dB), -80.0f, 0.0f, height, 0.0f);
            g.drawText(juce::String(dB), 2, static_cast<int>(y - 6), 25, 12,
                juce::Justification::left);
        }

        // FIXED: Create spectrum path with proper range
        juce::Path spectrumPath;
        juce::Path fillPath;
        bool firstPoint = true;

        for (int i = 0; i < static_cast<int>(spectrumData.size()); ++i)
        {
            auto magnitude = spectrumData[i];

            // IMPROVED: Better range mapping with extended low end
            magnitude = juce::jlimit(-80.0f, 0.0f, magnitude);

            auto x = juce::jmap(static_cast<float>(i), 0.0f,
                static_cast<float>(spectrumData.size() - 1), 0.0f, width);
            auto y = juce::jmap(magnitude, -80.0f, 0.0f, height, 0.0f);

            if (firstPoint)
            {
                spectrumPath.startNewSubPath(x, y);
                fillPath.startNewSubPath(x, height);
                fillPath.lineTo(x, y);
                firstPoint = false;
            }
            else
            {
                spectrumPath.lineTo(x, y);
                fillPath.lineTo(x, y);
            }
        }

        // Complete fill path
        fillPath.lineTo(width, height);
        fillPath.closeSubPath();

        // Professional filled area with gradient
        juce::ColourGradient fillGradient(
            juce::Colour(0xff002060), 0, height,
            juce::Colour(0xff0066cc), 0, height * 0.6f, false
        );
        fillGradient.addColour(0.7, juce::Colour(0xff4099ff));
        fillGradient.addColour(0.85, juce::Colour(0xff80d4ff));
        fillGradient.addColour(0.95, juce::Colour(0xffffff99));

        g.setGradientFill(fillGradient);
        g.setOpacity(0.3f);
        g.fillPath(fillPath);

        // Spectrum line
        juce::ColourGradient lineGradient(
            juce::Colour(0xff0080ff), 0, height,
            juce::Colour(0xffffffff), 0, height * 0.2f, false
        );
        lineGradient.addColour(0.8, juce::Colour(0xff66ccff));
        lineGradient.addColour(0.95, juce::Colour(0xffffff99));

        g.setGradientFill(lineGradient);
        g.setOpacity(1.0f);
        g.strokePath(spectrumPath, juce::PathStrokeType(1.5f));

        // Reference lines
        g.setColour(juce::Colours::red.withAlpha(0.4f));
        float zeroDbY = juce::jmap(0.0f, -80.0f, 0.0f, height, 0.0f);
        g.drawHorizontalLine(static_cast<int>(zeroDbY), 0, width);

        g.setColour(juce::Colours::orange.withAlpha(0.3f));
        float warningY = juce::jmap(-12.0f, -80.0f, 0.0f, height, 0.0f);
        g.drawHorizontalLine(static_cast<int>(warningY), 0, width);

        // DEBUGGING: Show if we're getting data
        g.setColour(juce::Colours::yellow);
        g.setFont(juce::FontOptions(10.0f));

        // Find max value for debugging
        float maxVal = *std::max_element(spectrumData.begin(), spectrumData.end());
        g.drawText("Max: " + juce::String(maxVal, 1) + "dB",
            width - 100, 5, 95, 15, juce::Justification::right);
    }

private:
    TrackTweakAudioProcessor& audioProcessor;
};

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
    juce::Label spectrumTitle;

    // Spectrum analyzer component
    std::unique_ptr<SpectrumAnalyzer> spectrumAnalyzer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackTweakAudioProcessorEditor)
};