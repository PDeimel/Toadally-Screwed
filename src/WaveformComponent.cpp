#include "WaveformComponent.hpp"

/**
 * @brief A component that displays an audio waveform visualization
 * This component continuously updates to show the current state of an audio buffer
 */

/**
 * @brief Constructs the WaveformComponent
 * @param bufferRef Reference to the audio buffer to visualize
 * @param writePosRef Reference to the current write position in the buffer
 */
WaveformComponent::WaveformComponent(juce::AudioSampleBuffer &bufferRef, int &writePosRef)
    : buffer(bufferRef), writePos(writePosRef) {
    startTimerHz(60); // Starts timer to refresh display at ~60 frames per second
}

/**
 * @brief Set the color scheme for the waveform and border
 * @param waveColor Color for the waveform line
 * @param borderColor Color for the border (should be darker than wave color)
 */
void WaveformComponent::setColorScheme(juce::Colour waveColor, juce::Colour borderColor) {
    waveformColor = waveColor;
    this->borderColor = borderColor;
    repaint(); // Trigger a repaint to apply the new colors
}

/**
 * @brief Handles the painting of the component
 * @param g The graphics context used for drawing
 */
void WaveformComponent::paint(juce::Graphics &g) {
    auto bounds = getLocalBounds().toFloat();

    // Fill background with black
    g.fillAll(juce::Colours::black);

    // Draw border with the specified border color
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds, 3.0f, 2.0f);

    // Reduce bounds for the actual waveform to leave space for border
    auto waveformBounds = bounds.reduced(3.0f);
    g.reduceClipRegion(waveformBounds.toNearestInt());

    // Set waveform color and draw
    g.setColour(waveformColor);
    drawWaveform(g);
}

/**
 * @brief Timer callback that triggers repainting
 * Called approximately 60 times per second to update the display
 */
void WaveformComponent::timerCallback() {
    repaint();
}

/**
 * @brief Draws the waveform visualization
 * @param g The graphics context used for drawing
 */
void WaveformComponent::drawWaveform(juce::Graphics &g) const {
    auto bounds = getLocalBounds().toFloat().reduced(3.0f); // Account for border
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();
    auto centerY = bounds.getCentreY();

    juce::Path waveformPath;
    waveformPath.startNewSubPath(bounds.getX(), centerY); // Start path at vertical center

    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0) {
        // Draw a flat line if no samples
        waveformPath.lineTo(bounds.getRight(), centerY);
        g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
        return;
    }

    const float step = static_cast<float>(numSamples) / width; // Calculate samples per pixel
    const int start = (writePos + 1) % numSamples;             // Get starting point after current write position

    // Draw the waveform point by point
    for (int i = 0; i < static_cast<int>(width); ++i) {
        // Calculate the actual sample index with wraparound
        const float index = std::fmod(start + i * step, static_cast<float>(numSamples));
        // Get the audio sample value at this index
        const float sample = buffer.getSample(0, static_cast<int>(index));
        // Map the sample value (-1 to 1) to screen coordinates within the reduced bounds
        const float y = juce::jmap(sample, -1.0f, 1.0f, bounds.getBottom(), bounds.getY());
        waveformPath.lineTo(bounds.getX() + static_cast<float>(i), y);
    }

    // Draw the waveform with slightly thicker line for better visibility
    g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
}