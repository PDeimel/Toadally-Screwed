#pragma once

#include "PluginProcessor.hpp"
#include "WaveformComponent.hpp"
#include "ADSRComponent.hpp"

//==============================================================================
class AvSynthAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                          public juce::ComboBox::Listener,
                                          public juce::Button::Listener,
                                          public juce::Timer{
public:
    explicit AvSynthAudioProcessorEditor(AvSynthAudioProcessor &);
    ~AvSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

    void updateOscImage(int);
    void comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged) override;
    void buttonClicked(juce::Button* button) override;
    void updateColorTheme(int oscTypeIndex);

private:
    void timerCallback() override;
    std::vector<juce::Component *> GetComps();
    juce::Colour getCurrentPrimaryColor() const;
    juce::Colour getCurrentSecondaryColor() const;

    // Preset-Funktionen
    void loadToadPreset(int presetIndex);
    void setupPresetButtons();

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

    // Toad Preset Buttons
    juce::TextButton toadPreset1Button;
    juce::TextButton toadPreset2Button;
    juce::TextButton toadPreset3Button;
    juce::TextButton toadPreset4Button;
    juce::Label presetLabel;

    juce::MidiKeyboardComponent keyboardComponent;
    WaveformComponent waveformComponent;
    ADSRComponent adsrComponent;

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

        // Button-Farben
        setColour(juce::TextButton::buttonColourId, juce::Colours::black.withAlpha(0.7f));
        setColour(juce::TextButton::buttonOnColourId, primary.withAlpha(0.8f));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::TextButton::textColourOnId, juce::Colours::white);

        // PopupMenu-Farben für bessere ComboBox-Dropdown-Darstellung
        setColour(juce::PopupMenu::backgroundColourId, juce::Colours::black.withAlpha(0.9f));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, primary.withAlpha(0.6f));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& /*backgroundColour*/,
                             bool /*shouldDrawButtonAsHighlighted*/, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        // Hintergrund
        if (shouldDrawButtonAsDown || button.getToggleState()) {
            g.setColour(primaryColor.withAlpha(0.8f));
        } else {
            g.setColour(juce::Colours::black.withAlpha(0.7f));
        }
        g.fillRoundedRectangle(bounds, 5.0f);

        // Umrandung
        g.setColour(primaryColor);
        g.drawRoundedRectangle(bounds, 5.0f, 2.0f);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                      int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/, juce::ComboBox& box) override
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
        g.setFont(juce::FontOptions(14.0f)); // Neue JUCE Font API
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
        auto trackBounds = juce::Rectangle<float>(static_cast<float>(x + width) * 0.4f, static_cast<float>(y), static_cast<float>(width) * 0.2f, static_cast<float>(height));

        // Track (Hintergrund)
        g.setColour(secondaryColor.withAlpha(0.3f));
        g.fillRoundedRectangle(trackBounds, 2.0f);

        // Filled Track (von unten bis zur aktuellen Position)
        // Bei vertikalen Slidern geht sliderPos von unten (maxSliderPos) nach oben (minSliderPos)
        auto filledHeight = maxSliderPos - sliderPos; // Höhe vom unteren Ende bis zur aktuellen Position
        auto filledTrack = juce::Rectangle<float>(trackBounds.getX(),
                                                 sliderPos, // Start bei aktueller Position
                                                 trackBounds.getWidth(),
                                                 filledHeight); // Bis zum unteren Ende
        g.setColour(primaryColor.withAlpha(0.8f));
        g.fillRoundedRectangle(filledTrack, 2.0f);

        // Thumb (Slider-Knopf)
        auto thumbSize = 12.0f;
        auto thumbBounds = juce::Rectangle<float>(static_cast<float>(x + width) * 0.5f - thumbSize * 0.5f,
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