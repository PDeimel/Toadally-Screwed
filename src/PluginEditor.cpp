#include "PluginEditor.hpp"
#include "PluginProcessor.hpp"
#include "Utils.hpp"
#include <magic_enum/magic_enum.hpp>

//==============================================================================
// CustomLookAndFeel Implementation

CustomLookAndFeel::CustomLookAndFeel() {
    updateColors(juce::Colours::orange, juce::Colours::darkorange);
}

void CustomLookAndFeel::updateColors(juce::Colour primary, juce::Colour secondary) {
    primaryColor = primary;
    secondaryColor = secondary;

    // Slider colors
    setColour(juce::Slider::thumbColourId, primary);
    setColour(juce::Slider::trackColourId, secondary.withAlpha(0.6f));
    setColour(juce::Slider::backgroundColourId, juce::Colours::black.withAlpha(0.3f));

    // ComboBox colors
    setColour(juce::ComboBox::outlineColourId, primary);
    setColour(juce::ComboBox::backgroundColourId, juce::Colours::black.withAlpha(0.7f));
    setColour(juce::ComboBox::textColourId, juce::Colours::white);
    setColour(juce::ComboBox::arrowColourId, primary);

    // Label colors
    setColour(juce::Label::textColourId, juce::Colours::white);

    // Button colors
    setColour(juce::TextButton::buttonColourId, juce::Colours::black.withAlpha(0.7f));
    setColour(juce::TextButton::buttonOnColourId, primary.withAlpha(0.8f));
    setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    setColour(juce::TextButton::textColourOnId, juce::Colours::white);

    // PopupMenu colors for better ComboBox dropdown display
    setColour(juce::PopupMenu::backgroundColourId, juce::Colours::black.withAlpha(0.9f));
    setColour(juce::PopupMenu::textColourId, juce::Colours::white);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, primary.withAlpha(0.6f));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                           const juce::Colour& /*backgroundColour*/,
                                           bool /*shouldDrawButtonAsHighlighted*/,
                                           bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat();

    // Background
    if (shouldDrawButtonAsDown || button.getToggleState()) {
        g.setColour(primaryColor.withAlpha(0.8f));
    } else {
        g.setColour(juce::Colours::black.withAlpha(0.7f));
    }
    g.fillRoundedRectangle(bounds, 5.0f);

    // Border
    g.setColour(primaryColor);
    g.drawRoundedRectangle(bounds, 5.0f, 2.0f);
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                   int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                   juce::ComboBox& box) {
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

    // Background with theme color
    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.fillRoundedRectangle(bounds, 5.0f);

    // Border with primary color
    g.setColour(primaryColor);
    g.drawRoundedRectangle(bounds, 5.0f, 2.0f);

    // Text area (leave space for arrow)
    auto textBounds = bounds.reduced(8, 4);
    textBounds.setWidth(textBounds.getWidth() - 20); // Space for arrow

    // Draw text
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(14.0f));
    g.drawFittedText(box.getText(), textBounds.toNearestInt(), juce::Justification::centredLeft, 1);

    // Draw arrow
    auto arrowBounds = juce::Rectangle<float>(bounds.getRight() - 25, bounds.getCentreY() - 4, 15, 8);
    g.setColour(primaryColor);

    // Simple downward arrow
    juce::Path arrow;
    arrow.addTriangle(arrowBounds.getX(), arrowBounds.getY(),
                     arrowBounds.getX() + arrowBounds.getWidth() * 0.5f, arrowBounds.getBottom(),
                     arrowBounds.getRight(), arrowBounds.getY());
    g.fillPath(arrow);
}

void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float minSliderPos, float maxSliderPos,
                                       const juce::Slider::SliderStyle style, juce::Slider& slider) {
    if (style == juce::Slider::LinearVertical) {
        // Vertical slider (for reverb)
        auto trackBounds = juce::Rectangle<float>(static_cast<float>(x + width) * 0.4f,
                                                 static_cast<float>(y),
                                                 static_cast<float>(width) * 0.2f,
                                                 static_cast<float>(height));

        // Track (background)
        g.setColour(secondaryColor.withAlpha(0.3f));
        g.fillRoundedRectangle(trackBounds, 2.0f);

        // Filled track (from bottom to current position)
        auto filledHeight = maxSliderPos - sliderPos;
        auto filledTrack = juce::Rectangle<float>(trackBounds.getX(),
                                                 sliderPos,
                                                 trackBounds.getWidth(),
                                                 filledHeight);
        g.setColour(primaryColor.withAlpha(0.8f));
        g.fillRoundedRectangle(filledTrack, 2.0f);

        // Thumb (slider knob)
        auto thumbSize = 12.0f;
        auto thumbBounds = juce::Rectangle<float>(static_cast<float>(x + width) * 0.5f - thumbSize * 0.5f,
                                                 sliderPos - thumbSize * 0.5f,
                                                 thumbSize, thumbSize);
        g.setColour(primaryColor);
        g.fillEllipse(thumbBounds);

        // Thumb outline
        g.setColour(juce::Colours::white);
        g.drawEllipse(thumbBounds, 2.0f);
    } else {
        // Horizontal slider - use default implementation
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }
}

//==============================================================================
// AvSynthAudioProcessorEditor Implementation

AvSynthAudioProcessorEditor::AvSynthAudioProcessorEditor(AvSynthAudioProcessor &p)
    : AudioProcessorEditor(&p),
      processorRef(p),

      // Initialize sliders with proper styles
      gainSlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      gainAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Gain>().data(), gainSlider),

      frequencySlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      frequencyAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Frequency>().data(), frequencySlider),

      oscTypeComboBox(),
      oscTypeAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::OscType>().data(), oscTypeComboBox),

      vowelMorphSlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      vowelMorphAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::VowelMorph>().data(), vowelMorphSlider),

      reverbSlider(juce::Slider::LinearVertical, juce::Slider::TextEntryBoxPosition::TextBoxBelow),
      reverbAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::ReverbAmount>().data(), reverbSlider),

      bitCrusherSlider(juce::Slider::LinearVertical, juce::Slider::TextBoxBelow),
      bitCrusherAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::BitCrusherRate>().data(), bitCrusherSlider),

      // Initialize preset buttons
      toadPreset1Button("Toad"),
      toadPreset2Button("Jerod"),
      toadPreset3Button("John"),
      toadPreset4Button("Dinkelberg"),

      // Initialize interactive components
      keyboardComponent(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      waveformComponent(p.circularBuffer.getBuffer(), p.bufferWritePos),
      vuMeterComponent() {

    juce::ignoreUnused(processorRef);

    // Set look and feel
    setLookAndFeel(&customLookAndFeel);

    // Configure vertical sliders
    reverbSlider.setRange(0.0, 1.0, 0.01);
    reverbSlider.setValue(0.0);
    reverbSlider.setSliderStyle(juce::Slider::LinearVertical);
    reverbSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

    bitCrusherSlider.setRange(0.01, 1.0, 0.01);
    bitCrusherSlider.setValue(0.01);
    bitCrusherSlider.setSliderStyle(juce::Slider::LinearVertical);
    bitCrusherSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    bitCrusherSlider.setLookAndFeel(&customLookAndFeel);

    // Configure labels
    setupLabels();

    // Setup preset buttons
    setupPresetButtons();

    // Setup ADSR component
    setupADSRComponent();

    // Setup ComboBox with oscillator choices
    setupOscillatorComboBox();

    // Add listeners
    oscTypeComboBox.addListener(this);

    // Set initial color theme and image
    currentOscType = oscTypeComboBox.getSelectedItemIndex();
    updateColorTheme(currentOscType);
    updateOscImage(currentOscType);

    // Add all components to the editor
    addAndMakeVisibleComponents();

    // Start timer for UI updates
    startTimer(TIMER_INTERVAL_MS);

    // Set initial size
    setSize(650, 720);
    setResizable(true, true);
}

AvSynthAudioProcessorEditor::~AvSynthAudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

//==============================================================================
// Component Setup Methods

void AvSynthAudioProcessorEditor::setupLabels() {
    // Configure labels with proper text and styling
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    vowelMorphLabel.setText("Vowel (A-E-I-O-U)", juce::dontSendNotification);
    vowelMorphLabel.setJustificationType(juce::Justification::centred);
    vowelMorphLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    reverbLabel.setText("Reverb", juce::dontSendNotification);
    reverbLabel.setJustificationType(juce::Justification::centred);
    reverbLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    bitCrusherLabel.setText("BitCrusher", juce::dontSendNotification);
    bitCrusherLabel.setJustificationType(juce::Justification::centred);
    bitCrusherLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    presetLabel.setText("=== Toad Presets ===", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    presetLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
}

void AvSynthAudioProcessorEditor::setupPresetButtons() {
    // Configure preset buttons
    toadPreset1Button.addListener(this);
    toadPreset2Button.addListener(this);
    toadPreset3Button.addListener(this);
    toadPreset4Button.addListener(this);

    // Button styling
    auto buttonColor = juce::Colours::black.withAlpha(0.7f);
    toadPreset1Button.setColour(juce::TextButton::buttonColourId, buttonColor);
    toadPreset2Button.setColour(juce::TextButton::buttonColourId, buttonColor);
    toadPreset3Button.setColour(juce::TextButton::buttonColourId, buttonColor);
    toadPreset4Button.setColour(juce::TextButton::buttonColourId, buttonColor);
}

void AvSynthAudioProcessorEditor::setupADSRComponent() {
    // Set up ADSR component callback
    adsrComponent.onParameterChanged = [this](float attack, float decay, float sustain, float release) {
        // Set parameter values directly in the ValueTreeState
        auto* attackParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Attack>().data());
        auto* decayParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Decay>().data());
        auto* sustainParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Sustain>().data());
        auto* releaseParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Release>().data());

        // Set parameters with normalized values
        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(attackParam)) {
            float normalizedValue = floatParam->convertTo0to1(attack);
            floatParam->setValueNotifyingHost(normalizedValue);
        }

        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(decayParam)) {
            float normalizedValue = floatParam->convertTo0to1(decay);
            floatParam->setValueNotifyingHost(normalizedValue);
        }

        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(sustainParam)) {
            float normalizedValue = floatParam->convertTo0to1(sustain);
            floatParam->setValueNotifyingHost(normalizedValue);
        }

        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(releaseParam)) {
            float normalizedValue = floatParam->convertTo0to1(release);
            floatParam->setValueNotifyingHost(normalizedValue);
        }
    };

    // Initialize ADSR component with current values
    updateUIFromParameters();
}

void AvSynthAudioProcessorEditor::setupOscillatorComboBox() {
    // Setup ComboBox with oscillator choices
    auto* oscTypeParam = dynamic_cast<juce::AudioParameterChoice*>(
        processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::OscType>().data()));

    if (oscTypeParam != nullptr) {
        oscTypeComboBox.clear();
        const auto& choices = oscTypeParam->choices;
        for (int i = 0; i < choices.size(); ++i) {
            oscTypeComboBox.addItem(choices[i], i + 1);
        }
        oscTypeComboBox.setSelectedId(oscTypeParam->getIndex() + 1, juce::dontSendNotification);
    }
}

void AvSynthAudioProcessorEditor::addAndMakeVisibleComponents() {
    // Add all components and make them visible
    for (auto* component : getComponents()) {
        addAndMakeVisible(component);
    }

    addAndMakeVisible(oscImage);
    addAndMakeVisible(reverbLabel);
    addAndMakeVisible(bitCrusherSlider);
    addAndMakeVisible(bitCrusherLabel);
    addAndMakeVisible(gainLabel);
    addAndMakeVisible(vowelMorphLabel);
    addAndMakeVisible(presetLabel);
    addAndMakeVisible(toadPreset1Button);
    addAndMakeVisible(toadPreset2Button);
    addAndMakeVisible(toadPreset3Button);
    addAndMakeVisible(toadPreset4Button);
    addAndMakeVisible(vuMeterComponent);
}

//==============================================================================
// Component Overrides

void AvSynthAudioProcessorEditor::paint(juce::Graphics &g) {
    // Dynamic gradient based on current oscillator type
    juce::ColourGradient gradient(
        primaryColor.withAlpha(0.8f),
        getLocalBounds().getTopLeft().toFloat(),
        secondaryColor.withAlpha(0.6f),
        getLocalBounds().getBottomRight().toFloat(),
        false);

    // Additional color stops for more depth
    gradient.addColour(0.3, primaryColor.withAlpha(0.4f));
    gradient.addColour(0.7, secondaryColor.withAlpha(0.8f));

    g.setGradientFill(gradient);
    g.fillAll();

    // Subtle overlay texture
    g.setColour(juce::Colours::black.withAlpha(0.1f));
    g.fillAll();
}

void AvSynthAudioProcessorEditor::resized() {
    auto bounds = getLocalBounds().reduced(10);

    // Right slider area
    auto rightSliderArea = bounds.removeFromRight(160);

    // Preset area at top
    auto presetArea = bounds.removeFromTop(80);
    presetLabel.setBounds(presetArea.removeFromTop(25));

    // Preset buttons in a row
    auto buttonWidth = presetArea.getWidth() / 4;
    toadPreset1Button.setBounds(presetArea.removeFromLeft(buttonWidth).reduced(2));
    toadPreset2Button.setBounds(presetArea.removeFromLeft(buttonWidth).reduced(2));
    toadPreset3Button.setBounds(presetArea.removeFromLeft(buttonWidth).reduced(2));
    toadPreset4Button.setBounds(presetArea.reduced(2));

    // Reserve lower areas for VU meter, keyboard and ADSR
    auto vuMeterArea = bounds.removeFromBottom(60);
    auto keyboardArea = bounds.removeFromBottom(80);
    auto adsrArea = bounds.removeFromBottom(180);

    // Small spacing between areas
    bounds.removeFromBottom(10);
    adsrArea.removeFromBottom(10);
    keyboardArea.removeFromBottom(10);

    // Layout remaining area: left controls, right visualizations
    auto leftColumn = bounds.removeFromLeft(bounds.getWidth() / 2);
    auto rightColumn = bounds;

    // Right slider areas
    auto reverbArea = rightSliderArea.removeFromLeft(80);
    auto bitCrusherArea = rightSliderArea;

    // Reverb slider (vertical, right side)
    auto reverbLabelArea = reverbArea.removeFromTop(20);
    reverbLabel.setBounds(reverbLabelArea);
    reverbSlider.setBounds(reverbArea.reduced(10));

    auto bitCrusherLabelArea = bitCrusherArea.removeFromTop(20);
    bitCrusherLabel.setBounds(bitCrusherLabelArea);
    bitCrusherSlider.setBounds(bitCrusherArea.reduced(10));

    // Left column: controls stacked vertically
    auto controlHeight = 40;
    gainSlider.setBounds(leftColumn.removeFromTop(controlHeight + 20));
    gainLabel.setBounds(gainSlider.getX(), gainSlider.getY(), gainSlider.getWidth(), 20);

    oscTypeComboBox.setBounds(leftColumn.removeFromTop(controlHeight + 10));

    // Vowel morph lower
    leftColumn.removeFromTop(20); // Additional spacing
    vowelMorphSlider.setBounds(leftColumn.removeFromTop(controlHeight + 20));
    vowelMorphLabel.setBounds(vowelMorphSlider.getX(), vowelMorphSlider.getY() - 20,
                             vowelMorphSlider.getWidth(), 20);

    // Right column: image at top, waveform below
    auto imageArea = rightColumn.removeFromTop(100);
    oscImage.setBounds(imageArea.reduced(10));
    waveformComponent.setBounds(rightColumn.reduced(10));

    // Components that take full width
    adsrComponent.setBounds(adsrArea.reduced(10, 5));
    keyboardComponent.setBounds(keyboardArea);
    vuMeterComponent.setBounds(vuMeterArea.reduced(5));
}

//==============================================================================
// Listener Implementations

void AvSynthAudioProcessorEditor::comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged) {
    if (comboBoxThatHasChanged == &oscTypeComboBox) {
        int newOscType = oscTypeComboBox.getSelectedItemIndex();
        updateColorTheme(newOscType);
        updateOscImage(newOscType);
    }
}

void AvSynthAudioProcessorEditor::buttonClicked(juce::Button* button) {
    if (button == &toadPreset1Button) {
        loadToadPreset(0);
    } else if (button == &toadPreset2Button) {
        loadToadPreset(1);
    } else if (button == &toadPreset3Button) {
        loadToadPreset(2);
    } else if (button == &toadPreset4Button) {
        loadToadPreset(3);
    }
}

void AvSynthAudioProcessorEditor::timerCallback() {
    // Update ADSR plotter with current values
    float currentValue = processorRef.getCurrentEnvelopeValue();
    bool isActive = processorRef.isEnvelopeActive();
    int state = processorRef.getADSRState();

    adsrComponent.updateEnvelopeValue(currentValue, isActive);
    adsrComponent.setADSRState(state);

    updateVUMeter();

    // Synchronize ADSR component with current parameter values
    updateUIFromParameters();
}

//==============================================================================
// Utility Methods

void AvSynthAudioProcessorEditor::updateVUMeter() {
    // Get current audio levels from processor
    auto& buffer = processorRef.circularBuffer.getBuffer();
    if (buffer.getNumChannels() > 0 && buffer.getNumSamples() > 0) {
        // Calculate RMS levels for left and right channels
        float leftLevel = 0.0f;
        float rightLevel = 0.0f;

        const int numSamples = juce::jmin(buffer.getNumSamples(), 512); // Limit for performance

        // Left channel
        if (buffer.getNumChannels() >= 1) {
            const float* leftData = buffer.getReadPointer(0);
            float sum = 0.0f;
            for (int i = 0; i < numSamples; ++i) {
                sum += leftData[i] * leftData[i];
            }
            leftLevel = std::sqrt(sum / numSamples);
        }

        // Right channel (use left if mono)
        if (buffer.getNumChannels() >= 2) {
            const float* rightData = buffer.getReadPointer(1);
            float sum = 0.0f;
            for (int i = 0; i < numSamples; ++i) {
                sum += rightData[i] * rightData[i];
            }
            rightLevel = std::sqrt(sum / numSamples);
        } else {
            rightLevel = leftLevel; // Mono signal
        }

        vuMeterComponent.updateLevels(leftLevel, rightLevel);
    }
}

void AvSynthAudioProcessorEditor::updateColorTheme(int oscTypeIndex) {
    currentOscType = oscTypeIndex;

    // Color scheme based on oscillator type
    switch (oscTypeIndex) {
        case 0: // Sine - Red
            primaryColor = juce::Colour(220, 50, 50);      // Bright red
            secondaryColor = juce::Colour(120, 20, 20);    // Dark red
            break;
        case 1: // Square - Blue
            primaryColor = juce::Colour(50, 120, 220);     // Bright blue
            secondaryColor = juce::Colour(20, 60, 120);    // Dark blue
            break;
        case 2: // Saw - Green
            primaryColor = juce::Colour(50, 200, 80);      // Bright green
            secondaryColor = juce::Colour(20, 100, 40);    // Dark green
            break;
        case 3: // Triangle - Yellow
            primaryColor = juce::Colour(220, 200, 50);     // Bright yellow
            secondaryColor = juce::Colour(150, 120, 20);   // Dark yellow/orange
            break;
        default:
            primaryColor = juce::Colours::orange;
            secondaryColor = juce::Colours::darkorange;
            break;
    }

    customLookAndFeel.updateColors(primaryColor, secondaryColor);

    vuMeterComponent.setColorScheme(primaryColor, secondaryColor);

    // Update label colors
    reverbLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    vowelMorphLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    presetLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    repaint();

    // Repaint all sliders
    for (auto* comp : getComponents()) {
        comp->repaint();
    }

    toadPreset1Button.repaint();
    toadPreset2Button.repaint();
    toadPreset3Button.repaint();
    toadPreset4Button.repaint();
}

void AvSynthAudioProcessorEditor::updateOscImage(int oscTypeIndex) {
    const void* imageData = nullptr;
    int imageSize = 0;

    switch (oscTypeIndex) {
        case 0:
            imageData = ToadyAssets::sine_wave_png;
            imageSize = ToadyAssets::sine_wave_pngSize;
            break;
        case 1:
            imageData = ToadyAssets::square_wave_png;
            imageSize = ToadyAssets::square_wave_pngSize;
            break;
        case 2:
            imageData = ToadyAssets::sawtooth_wave_png;
            imageSize = ToadyAssets::sawtooth_wave_pngSize;
            break;
        case 3:
            imageData = ToadyAssets::triangle_wave_png;
            imageSize = ToadyAssets::triangle_wave_pngSize;
            break;
        default:
            imageData = ToadyAssets::sawtooth_wave_png;
            imageSize = ToadyAssets::sawtooth_wave_pngSize;
            break;
    }

    if (imageData != nullptr && imageSize > 0) {
        auto image = juce::ImageCache::getFromMemory(imageData, imageSize);
        oscImage.setImage(image, juce::RectanglePlacement::centred);
    } else {
        DBG("Error loading image for oscillator type " << oscTypeIndex);
    }
}

void AvSynthAudioProcessorEditor::loadToadPreset(int presetIndex) {
    if (processorRef.loadPreset(presetIndex)) {
        // Update UI to reflect the loaded preset
        updateColorTheme(processorRef.getPresetManager().getPreset(presetIndex)->oscType);
        updateOscImage(processorRef.getPresetManager().getPreset(presetIndex)->oscType);
    }
}

void AvSynthAudioProcessorEditor::updateUIFromParameters() {
    static float lastAttack = -1.0f, lastDecay = -1.0f, lastSustain = -1.0f, lastRelease = -1.0f;

    float currentAttack = processorRef.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Attack>().data())->load();
    float currentDecay = processorRef.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Decay>().data())->load();
    float currentSustain = processorRef.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Sustain>().data())->load();
    float currentRelease = processorRef.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Release>().data())->load();

    // Only update if values have changed
    if (!juce::approximatelyEqual(currentAttack, lastAttack) ||
        !juce::approximatelyEqual(currentDecay, lastDecay) ||
        !juce::approximatelyEqual(currentSustain, lastSustain) ||
        !juce::approximatelyEqual(currentRelease, lastRelease)) {

        adsrComponent.setAttack(currentAttack);
        adsrComponent.setDecay(currentDecay);
        adsrComponent.setSustain(currentSustain);
        adsrComponent.setRelease(currentRelease);

        lastAttack = currentAttack;
        lastDecay = currentDecay;
        lastSustain = currentSustain;
        lastRelease = currentRelease;
    }
}

std::vector<juce::Component*> AvSynthAudioProcessorEditor::getComponents() {
    return {
        &gainSlider, &frequencySlider, &oscTypeComboBox, &vowelMorphSlider,
        &reverbSlider, &bitCrusherSlider, &keyboardComponent,
        &waveformComponent, &adsrComponent, &vuMeterComponent
    };
}