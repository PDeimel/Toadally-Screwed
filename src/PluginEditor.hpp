#pragma once

#include "PluginProcessor.hpp"
#include "WaveformComponent.hpp"

//==============================================================================
class AvSynthAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                          public juce::ComboBox::Listener {
public:
    explicit AvSynthAudioProcessorEditor(AvSynthAudioProcessor &);
    ~AvSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

    void updateOscImage(int);
    void comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged) override;

private:
    std::vector<juce::Component *> GetComps();

    AvSynthAudioProcessor &processorRef;

    juce::Slider gainSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment;

    juce::Label gainLabel;

    juce::Slider frequencySlider;
    juce::AudioProcessorValueTreeState::SliderAttachment frequencyAttachment;

    juce::Label frequencyLabel;

    juce::ComboBox oscTypeComboBox;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment oscTypeAttachment;

    juce::Slider lowCutFreqSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment lowCutFreqAttachment;

    juce::Slider highCutFreqSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment highCutFreqAttachment;

    juce::Slider vowelMorphSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment vowelMorphAttachment;

    juce::Label vowelMorphLabel;

    juce::MidiKeyboardComponent keyboardComponent;
    WaveformComponent waveformComponent;

    juce::ImageComponent oscImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AvSynthAudioProcessorEditor)
};

class CustomLookAndFeel : public juce::LookAndFeel_V4 {
public:
    CustomLookAndFeel() {
        setColour(juce::Slider::thumbColourId, juce::Colours::orange);
        setColour(juce::Slider::trackColourId, juce::Colours::grey);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::darkgrey);
        setColour(juce::ComboBox::backgroundColourId, juce::Colours::black);
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.fillRoundedRectangle(bounds, 5.0f);
        g.setColour(juce::Colours::white);
        g.drawFittedText(box.getText(), bounds.toNearestInt().reduced(4), juce::Justification::centredLeft, 1);
    }
};

inline CustomLookAndFeel customLookAndFeel;