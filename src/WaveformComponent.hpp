#pragma once

#include "JuceHeader.h"

class WaveformComponent : public juce::Component, public juce::Timer {
public:
  WaveformComponent(juce::AudioSampleBuffer &bufferRef, int &writePosRef);

  void paint(juce::Graphics &g) override;

  /**
   * @brief Set the color scheme for the waveform and border
   * @param waveColor Color for the waveform line
   * @param borderColor Color for the border (should be darker than wave color)
   */
  void setColorScheme(juce::Colour waveColor, juce::Colour borderColor);

private:
  void timerCallback() override;

  void drawWaveform(juce::Graphics &g) const;

  juce::AudioSampleBuffer &buffer;
  int &writePos;

  // Color scheme
  juce::Colour waveformColor = juce::Colours::lime;
  juce::Colour borderColor = juce::Colours::darkgreen;
};