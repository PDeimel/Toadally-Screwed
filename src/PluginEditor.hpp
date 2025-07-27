#pragma once

#include "PluginProcessor.hpp"
#include "WaveformComponent.hpp"
#include "ADSRComponent.hpp"
#include "PresetManager.hpp"

/**
 * @file PluginEditor.hpp
 * @brief Main editor class for the AvSynth audio plugin
 */

/**
 * @brief Custom look and feel class for the AvSynth UI
 *
 * This class provides custom styling and theming capabilities for the plugin interface,
 * with dynamic color schemes based on oscillator type.
 */
class CustomLookAndFeel : public juce::LookAndFeel_V4 {
public:
    /**
     * @brief Constructor with default colors
     */
    CustomLookAndFeel();

    /**
     * @brief Update the color scheme
     * @param primary Primary theme color
     * @param secondary Secondary theme color
     */
    void updateColors(juce::Colour primary, juce::Colour secondary);

    /**
     * @brief Custom button background drawing
     * @param g Graphics context
     * @param button Button to draw
     * @param backgroundColour Background color
     * @param shouldDrawButtonAsHighlighted Highlight state
     * @param shouldDrawButtonAsDown Pressed state
     */
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    /**
     * @brief Custom combo box drawing
     * @param g Graphics context
     * @param width Component width
     * @param height Component height
     * @param isButtonDown Button pressed state
     * @param buttonX Button area X
     * @param buttonY Button area Y
     * @param buttonW Button area width
     * @param buttonH Button area height
     * @param box ComboBox reference
     */
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;

    /**
     * @brief Custom linear slider drawing
     * @param g Graphics context
     * @param x Slider X position
     * @param y Slider Y position
     * @param width Slider width
     * @param height Slider height
     * @param sliderPos Current slider position
     * @param minSliderPos Minimum slider position
     * @param maxSliderPos Maximum slider position
     * @param style Slider style
     * @param slider Slider reference
     */
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override;

private:
    juce::Colour primaryColor = juce::Colours::orange;   ///< Primary theme color
    juce::Colour secondaryColor = juce::Colours::darkorange; ///< Secondary theme color
};

/**
 * @brief Main editor class for the AvSynth audio plugin
 *
 * This class provides the graphical user interface for the AvSynth synthesizer,
 * including parameter controls, visualization, and preset management.
 */
class AvSynthAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                          public juce::ComboBox::Listener,
                                          public juce::Button::Listener,
                                          public juce::Timer {
public:
    /**
     * @brief Constructor
     * @param processor Reference to the audio processor
     */
    explicit AvSynthAudioProcessorEditor(AvSynthAudioProcessor &processor);

    /**
     * @brief Destructor
     */
    ~AvSynthAudioProcessorEditor() override;

    void setupLabels();

    //==============================================================================
    // Component overrides

    void addAndMakeVisibleComponents();

    /**
     * @brief Paint the component background
     * @param g Graphics context for drawing
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Layout all child components
     */
    void resized() override;

    //==============================================================================
    // Listener implementations

    /**
     * @brief Handle combo box selection changes
     * @param comboBoxThatHasChanged Pointer to the changed combo box
     */
    void comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged) override;

    /**
     * @brief Handle button clicks
     * @param button Pointer to the clicked button
     */
    void buttonClicked(juce::Button* button) override;

    /**
     * @brief Timer callback for regular UI updates
     */
    void timerCallback() override;

    //==============================================================================
    // Public utility methods

    /**
     * @brief Update the color theme based on oscillator type
     * @param oscTypeIndex Index of the oscillator type (0-3)
     */
    void updateColorTheme(int oscTypeIndex);

    /**
     * @brief Update the oscillator waveform image
     * @param oscTypeIndex Index of the oscillator type (0-3)
     */
    void updateOscImage(int oscTypeIndex);

    /**
     * @brief Get current primary color
     * @return Current primary theme color
     */
    juce::Colour getCurrentPrimaryColor() const { return primaryColor; }

    /**
     * @brief Get current secondary color
     * @return Current secondary theme color
     */
    juce::Colour getCurrentSecondaryColor() const { return secondaryColor; }

private:
    //==============================================================================
    // Private methods

    /**
     * @brief Get all components for mass operations
     * @return Vector of pointers to all main components
     */
    std::vector<juce::Component *> getComponents();

    /**
     * @brief Setup preset buttons with proper styling and listeners
     */
    void setupPresetButtons();

    /**
     * @brief Load a Toad preset by index
     * @param presetIndex Index of the preset to load (0-3)
     */
    void loadToadPreset(int presetIndex);

    /**
     * @brief Setup ADSR component callbacks and initial values
     */
    void setupADSRComponent();

    void setupOscillatorComboBox();

    /**
     * @brief Update UI components to reflect current parameter values
     */
    void updateUIFromParameters();

    //==============================================================================
    // Member variables

    // Processor reference
    AvSynthAudioProcessor &processorRef; ///< Reference to the audio processor

    // Parameter controls
    juce::Slider gainSlider;                                              ///< Main gain control
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment; ///< Gain parameter attachment
    juce::Label gainLabel;                                                ///< Gain control label

    juce::Slider frequencySlider;                                              ///< Frequency control
    juce::AudioProcessorValueTreeState::SliderAttachment frequencyAttachment; ///< Frequency parameter attachment
    juce::Label frequencyLabel;                                                ///< Frequency control label

    juce::ComboBox oscTypeComboBox;                                              ///< Oscillator type selector
    juce::AudioProcessorValueTreeState::ComboBoxAttachment oscTypeAttachment; ///< Oscillator type attachment

    juce::Slider vowelMorphSlider;                                              ///< Vowel morphing control
    juce::AudioProcessorValueTreeState::SliderAttachment vowelMorphAttachment; ///< Vowel morph attachment
    juce::Label vowelMorphLabel;                                                ///< Vowel morph label

    juce::Slider reverbSlider;                                              ///< Reverb amount control
    juce::AudioProcessorValueTreeState::SliderAttachment reverbAttachment; ///< Reverb parameter attachment
    juce::Label reverbLabel;                                                ///< Reverb control label

    juce::Slider bitCrusherSlider;                                              ///< Bit crusher control
    juce::AudioProcessorValueTreeState::SliderAttachment bitCrusherAttachment; ///< Bit crusher attachment
    juce::Label bitCrusherLabel;                                                ///< Bit crusher label

    // Preset controls
    juce::TextButton toadPreset1Button; ///< Toad preset button 1
    juce::TextButton toadPreset2Button; ///< Toad preset button 2
    juce::TextButton toadPreset3Button; ///< Toad preset button 3
    juce::TextButton toadPreset4Button; ///< Toad preset button 4
    juce::Label presetLabel;            ///< Preset section label

    // Interactive components
    juce::MidiKeyboardComponent keyboardComponent; ///< MIDI keyboard component
    WaveformComponent waveformComponent;           ///< Waveform visualization component
    ADSRComponent adsrComponent;                   ///< ADSR envelope component

    // Visual elements
    juce::ImageComponent oscImage; ///< Oscillator waveform image display

    // Theme and styling
    CustomLookAndFeel customLookAndFeel; ///< Custom look and feel instance
    int currentOscType = 0;              ///< Current oscillator type index
    juce::Colour primaryColor = juce::Colours::red;     ///< Current primary theme color
    juce::Colour secondaryColor = juce::Colours::darkred; ///< Current secondary theme color

    // UI update optimization
    static constexpr int UI_UPDATE_RATE_HZ = 30; ///< UI update rate in Hz
    static constexpr int TIMER_INTERVAL_MS = 1000 / UI_UPDATE_RATE_HZ; ///< Timer interval in milliseconds

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AvSynthAudioProcessorEditor)
};