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
    void updateColorTheme(int oscTypeIndex);

private:
    std::vector<juce::Component *> GetComps();
    juce::Colour getCurrentPrimaryColor() const;
    juce::Colour getCurrentSecondaryColor() const;

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

    // Reverb-Slider
    juce::Slider reverbSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment reverbAttachment;
    juce::Label reverbLabel;

    juce::MidiKeyboardComponent keyboardComponent;
    WaveformComponent waveformComponent;

    juce::ImageComponent oscImage;

    // Farbschema-Variablen
    int currentOscType = 0;
    juce::Colour primaryColor = juce::Colours::red;
    juce::Colour secondaryColor = juce::Colours::darkred;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AvSynthAudioProcessorEditor)
};

class CustomLookAndFeel : public juce::LookAndFeel_V4 {
private:
    juce::Colour primaryColor = juce::Colours::orange;
    juce::Colour secondaryColor = juce::Colours::darkorange;

public:
    CustomLookAndFeel() {
        updateColors(juce::Colours::orange, juce::Colours::darkorange);
    }

    void updateColors(juce::Colour primary, juce::Colour secondary) {
        primaryColor = primary;
        secondaryColor = secondary;

        // Slider-Farben
        setColour(juce::Slider::thumbColourId, primary);
        setColour(juce::Slider::trackColourId, secondary.withAlpha(0.6f));
        setColour(juce::Slider::backgroundColourId, juce::Colours::black.withAlpha(0.3f));

        // ComboBox-Farben
        setColour(juce::ComboBox::outlineColourId, primary);
        setColour(juce::ComboBox::backgroundColourId, juce::Colours::black.withAlpha(0.7f));
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::arrowColourId, primary);

        // Label-Farben
        setColour(juce::Label::textColourId, juce::Colours::white);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

        // Hintergrund mit Theme-Farbe
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.fillRoundedRectangle(bounds, 5.0f);

        // Umrandung mit Primary-Farbe
        g.setColour(primaryColor);
        g.drawRoundedRectangle(bounds, 5.0f, 2.0f);

        // Text
        g.setColour(juce::Colours::white);
        g.drawFittedText(box.getText(), bounds.toNearestInt().reduced(4), juce::Justification::centredLeft, 1);

        // Pfeil
        auto arrowBounds = juce::Rectangle<float>(bounds.getRight() - 20, bounds.getCentreY() - 3, 10, 6);
        g.setColour(primaryColor);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (style == juce::Slider::LinearVertical)
        {
            // Vertikaler Slider (für Reverb)
            auto trackBounds = juce::Rectangle<float>(x + width * 0.4f, y, width * 0.2f, height);

            // Track (Hintergrund)
            g.setColour(secondaryColor.withAlpha(0.3f));
            g.fillRoundedRectangle(trackBounds, 2.0f);

            // Filled Track (bis zur aktuellen Position)
            auto filledHeight = sliderPos - y;
            auto filledTrack = juce::Rectangle<float>(trackBounds.getX(), y, trackBounds.getWidth(), filledHeight);
            g.setColour(primaryColor.withAlpha(0.8f));
            g.fillRoundedRectangle(filledTrack, 2.0f);

            // Thumb (Slider-Knopf)
            auto thumbSize = 12.0f;
            auto thumbBounds = juce::Rectangle<float>(x + width * 0.5f - thumbSize * 0.5f,
                                                     sliderPos - thumbSize * 0.5f,
                                                     thumbSize, thumbSize);
            g.setColour(primaryColor);
            g.fillEllipse(thumbBounds);

            // Thumb-Outline
            g.setColour(juce::Colours::white);
            g.drawEllipse(thumbBounds, 2.0f);
        }
        else
        {
            // Horizontaler Slider
            LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }
};

inline CustomLookAndFeel customLookAndFeel;