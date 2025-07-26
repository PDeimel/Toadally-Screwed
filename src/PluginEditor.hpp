#pragma once

#include "PluginProcessor.hpp"
#include "WaveformComponent.hpp"
#include "ADSRComponent.hpp"

//==============================================================================
class AvSynthAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                          public juce::ComboBox::Listener,
                                          public juce::Slider::Listener,
                                          public juce::Timer{
public:
    explicit AvSynthAudioProcessorEditor(AvSynthAudioProcessor &);
    ~AvSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

    void updateOscImage(int);
    void comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged) override;
    void updateColorTheme(int oscTypeIndex);
    void sliderValueChanged(juce::Slider* slider) override;

private:
    void timerCallback() override;
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

    juce::Slider vowelMorphSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment vowelMorphAttachment;
    juce::Label vowelMorphLabel;

    // Reverb-Slider
    juce::Slider reverbSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment reverbAttachment;
    juce::Label reverbLabel;

    juce::Slider bitCrusherSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment bitCrusherAttachment;
    juce::Label bitCrusherLabel;

    juce::MidiKeyboardComponent keyboardComponent;
    WaveformComponent waveformComponent;
    ADSRComponent adsrComponent;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;

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

        // PopupMenu-Farben für bessere ComboBox-Dropdown-Darstellung
        setColour(juce::PopupMenu::backgroundColourId, juce::Colours::black.withAlpha(0.9f));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, primary.withAlpha(0.6f));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
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

        // Text-Bereich definieren (Platz für Pfeil lassen)
        auto textBounds = bounds.reduced(8, 4);
        textBounds.setWidth(textBounds.getWidth() - 20); // Platz für Pfeil

        // Text zeichnen
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f));
        g.drawFittedText(box.getText(), textBounds.toNearestInt(), juce::Justification::centredLeft, 1);

        // Pfeil zeichnen
        auto arrowBounds = juce::Rectangle<float>(bounds.getRight() - 25, bounds.getCentreY() - 4, 15, 8);
        g.setColour(primaryColor);

        // Einfacher Pfeil nach unten
        juce::Path arrow;
        arrow.addTriangle(arrowBounds.getX(), arrowBounds.getY(),
                         arrowBounds.getX() + arrowBounds.getWidth() * 0.5f, arrowBounds.getBottom(),
                         arrowBounds.getRight(), arrowBounds.getY());
        g.fillPath(arrow);
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