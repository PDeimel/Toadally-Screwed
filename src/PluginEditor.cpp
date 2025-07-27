#include "PluginEditor.hpp"
#include "PluginProcessor.hpp"

#include <magic_enum/magic_enum.hpp>

//==============================================================================
AvSynthAudioProcessorEditor::AvSynthAudioProcessorEditor(AvSynthAudioProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p),
      gainSlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      gainAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Gain>().data(), gainSlider),

      frequencySlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      frequencyAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Frequency>().data(), frequencySlider),

      oscTypeComboBox(),
      oscTypeAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::OscType>().data(), oscTypeComboBox),

      vowelMorphSlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      vowelMorphAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::VowelMorph>().data(), vowelMorphSlider),

      // Reverb-Slider (vertikal)
      reverbSlider(juce::Slider::LinearVertical, juce::Slider::TextEntryBoxPosition::TextBoxBelow),
      reverbAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::ReverbAmount>().data(), reverbSlider),

      bitCrusherSlider(juce::Slider::LinearHorizontal, juce::Slider::TextBoxBelow),
      bitCrusherAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::BitCrusherRate>().data(), bitCrusherSlider),

      // Toad Preset Buttons (KORRIGIERTE NAMEN)
      toadPreset1Button("Toad Sine"),
      toadPreset2Button("Toad Square"),
      toadPreset3Button("Toad Saw"),
      toadPreset4Button("Toad Tri"),

      keyboardComponent(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      waveformComponent(p.circularBuffer, p.bufferWritePos)
{
    juce::ignoreUnused(processorRef);

    setLookAndFeel(&customLookAndFeel);

    // Reverb-Slider konfigurieren
    reverbSlider.setRange(0.0, 1.0, 0.01);
    reverbSlider.setValue(0.0);
    reverbSlider.setSliderStyle(juce::Slider::LinearVertical);
    reverbSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);

    // Reverb-Label konfigurieren
    reverbLabel.setText("Reverb", juce::dontSendNotification);
    reverbLabel.setJustificationType(juce::Justification::centred);
    reverbLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // BitCrusher-Slider konfigurieren
    bitCrusherSlider.setRange(0.01, 1.0, 0.01);
    bitCrusherSlider.setValue(0.01); // Standard = keine Veränderung
    bitCrusherSlider.setSliderStyle(juce::Slider::LinearVertical);
    bitCrusherSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    bitCrusherSlider.setLookAndFeel(&customLookAndFeel);

    // BitCrusher-Label konfigurieren
    bitCrusherLabel.setText("BitCrusher", juce::dontSendNotification);
    bitCrusherLabel.setJustificationType(juce::Justification::centred);
    bitCrusherLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Gain-Label konfigurieren
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Vowel-Morph-Label konfigurieren
    vowelMorphLabel.setText("Vowel (A-E-I-O-U)", juce::dontSendNotification);
    vowelMorphLabel.setJustificationType(juce::Justification::centred);
    vowelMorphLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Preset-Label konfigurieren
    presetLabel.setText("=== Toad Presets ===", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centred);
    presetLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    presetLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));

    // Preset-Buttons konfigurieren
    setupPresetButtons();

    adsrComponent.onParameterChanged = [this, &p](float attack, float decay, float sustain, float release) {
        // Parameter direkt in der ValueTreeState setzen
        auto* attackParam = p.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Attack>().data());
        auto* decayParam = p.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Decay>().data());
        auto* sustainParam = p.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Sustain>().data());
        auto* releaseParam = p.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Release>().data());

        // Parameter mit normalisierten Werten setzen (wichtig!)
        if (attackParam) {
            auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(attackParam);
            if (floatParam) {
                float normalizedValue = floatParam->convertTo0to1(attack);
                floatParam->setValueNotifyingHost(normalizedValue);
            }
        }

        if (decayParam) {
            auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(decayParam);
            if (floatParam) {
                float normalizedValue = floatParam->convertTo0to1(decay);
                floatParam->setValueNotifyingHost(normalizedValue);
            }
        }

        if (sustainParam) {
            auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(sustainParam);
            if (floatParam) {
                float normalizedValue = floatParam->convertTo0to1(sustain);
                floatParam->setValueNotifyingHost(normalizedValue);
            }
        }

        if (releaseParam) {
            auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(releaseParam);
            if (floatParam) {
                float normalizedValue = floatParam->convertTo0to1(release);
                floatParam->setValueNotifyingHost(normalizedValue);
            }
        }
    };

    // ADSR Component mit aktuellen Werten initialisieren
    adsrComponent.setAttack(p.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Attack>().data())->load());
    adsrComponent.setDecay(p.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Decay>().data())->load());
    adsrComponent.setSustain(p.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Sustain>().data())->load());
    adsrComponent.setRelease(p.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Release>().data())->load());

    startTimer(33); // ~30 FPS für UI-Updates

    // Setup ComboBox with oscillator choices
    auto *oscTypeParam = dynamic_cast<juce::AudioParameterChoice *>(
        p.parameters.getParameter(magic_enum::enum_name<AvSynthAudioProcessor::Parameters::OscType>().data()));

    if (oscTypeParam != nullptr)
    {
        oscTypeComboBox.clear();
        auto &choices = oscTypeParam->choices;
        for (int i = 0; i < choices.size(); ++i)
        {
            oscTypeComboBox.addItem(choices[i], i + 1);
        }
        oscTypeComboBox.setSelectedId(oscTypeParam->getIndex() + 1, juce::dontSendNotification);
    }

    oscTypeComboBox.addListener(this); // Add ComboBox listener

    // Initiales Farbschema und Bild setzen
    currentOscType = oscTypeComboBox.getSelectedItemIndex();
    updateColorTheme(currentOscType);
    updateOscImage(currentOscType);

    for (auto *component : GetComps())
        addAndMakeVisible(component);

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

    setSize(650, 650); // Größer für Preset-Bereich
    setResizable(true, true);
}

AvSynthAudioProcessorEditor::~AvSynthAudioProcessorEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void AvSynthAudioProcessorEditor::setupPresetButtons() {
    // Preset-Buttons konfigurieren
    toadPreset1Button.addListener(this);
    toadPreset2Button.addListener(this);
    toadPreset3Button.addListener(this);
    toadPreset4Button.addListener(this);

    // Button-Styling
    toadPreset1Button.setColour(juce::TextButton::buttonColourId, juce::Colours::black.withAlpha(0.7f));
    toadPreset2Button.setColour(juce::TextButton::buttonColourId, juce::Colours::black.withAlpha(0.7f));
    toadPreset3Button.setColour(juce::TextButton::buttonColourId, juce::Colours::black.withAlpha(0.7f));
    toadPreset4Button.setColour(juce::TextButton::buttonColourId, juce::Colours::black.withAlpha(0.7f));
}

void AvSynthAudioProcessorEditor::loadToadPreset(int presetIndex) {
    // Toad-artige Presets für jeden Oszillator-Typ
    struct ToadPreset {
        float gain;
        int oscType;
        float vowelMorph;
        float reverbAmount;
        float bitCrusherRate;
        float attack;
        float decay;
        float sustain;
        float release;
    };

    ToadPreset presets[4] = {
        // Preset 1: Toad Sine - Sanft und melodisch wie Toads höhere Töne
        {0.25f, 0, 0.15f, 0.25f, 0.8f, 0.05f, 0.2f, 0.8f, 0.3f},

        // Preset 2: Toad Square - Retro und charakteristisch wie klassische Mario-Sounds
        {0.25f, 1, 0.45f, 0.2f, 0.4f, 0.08f, 0.25f, 0.75f, 0.4f},

        // Preset 3: Toad Saw - Kratzig und aufgeregt wie Toads "Wahoo!"
        {0.25f, 2, 0.3f, 0.15f, 0.6f, 0.02f, 0.15f, 0.7f, 0.25f},

        // Preset 4: Toad Triangle - Weich aber markant, wie Toads ruhigere Stimme
        {0.25f, 3, 0.2f, 0.3f, 0.9f, 0.1f, 0.3f, 0.85f, 0.5f}
    };

    if (presetIndex >= 0 && presetIndex < 4) {
        auto& preset = presets[presetIndex];

        // Parameter setzen
        auto* gainParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Gain>().data());
        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(gainParam)) {
            floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset.gain));
        }

        auto* oscTypeParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::OscType>().data());
        if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(oscTypeParam)) {
            choiceParam->setValueNotifyingHost(choiceParam->convertTo0to1(preset.oscType));
        }

        auto* vowelParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::VowelMorph>().data());
        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(vowelParam)) {
            floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset.vowelMorph));
        }

        auto* reverbParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::ReverbAmount>().data());
        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(reverbParam)) {
            floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset.reverbAmount));
        }

        auto* bitCrusherParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::BitCrusherRate>().data());
        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(bitCrusherParam)) {
            floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset.bitCrusherRate));
        }

        auto* attackParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Attack>().data());
        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(attackParam)) {
            floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset.attack));
        }

        auto* decayParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Decay>().data());
        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(decayParam)) {
            floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset.decay));
        }

        auto* sustainParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Sustain>().data());
        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(sustainParam)) {
            floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset.sustain));
        }

        auto* releaseParam = processorRef.parameters.getParameter(
            magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Release>().data());
        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(releaseParam)) {
            floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset.release));
        }

        // UI updaten
        updateColorTheme(preset.oscType);
        updateOscImage(preset.oscType);
    }
}

void AvSynthAudioProcessorEditor::buttonClicked(juce::Button* button) {
    if (button == &toadPreset1Button) {
        loadToadPreset(0);
    }
    else if (button == &toadPreset2Button) {
        loadToadPreset(1);
    }
    else if (button == &toadPreset3Button) {
        loadToadPreset(2);
    }
    else if (button == &toadPreset4Button) {
        loadToadPreset(3);
    }
}

//==============================================================================
void AvSynthAudioProcessorEditor::paint(juce::Graphics &g)
{
    // Dynamischer Gradient basierend auf dem aktuellen Oszillator-Typ
    juce::ColourGradient gradient(
        primaryColor.withAlpha(0.8f),
        getLocalBounds().getTopLeft().toFloat(),
        secondaryColor.withAlpha(0.6f),
        getLocalBounds().getBottomRight().toFloat(),
        false);

    // Zusätzliche Farb-Stops für mehr Tiefe
    gradient.addColour(0.3, primaryColor.withAlpha(0.4f));
    gradient.addColour(0.7, secondaryColor.withAlpha(0.8f));

    g.setGradientFill(gradient);
    g.fillAll();

    // Subtile Overlay-Textur
    g.setColour(juce::Colours::black.withAlpha(0.1f));
    g.fillAll();
}

void AvSynthAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    auto rightSliderArea = bounds.removeFromRight(160);

    // Preset-Bereich oben
    auto presetArea = bounds.removeFromTop(80);
    presetLabel.setBounds(presetArea.removeFromTop(25));

    // Preset-Buttons in einer Zeile
    auto buttonWidth = presetArea.getWidth() / 4;
    toadPreset1Button.setBounds(presetArea.removeFromLeft(buttonWidth).reduced(2));
    toadPreset2Button.setBounds(presetArea.removeFromLeft(buttonWidth).reduced(2));
    toadPreset3Button.setBounds(presetArea.removeFromLeft(buttonWidth).reduced(2));
    toadPreset4Button.setBounds(presetArea.reduced(2));

    // Untere Bereiche für Keyboard und ADSR reservieren (bevor die Spalten erstellt werden)
    auto keyboardArea = bounds.removeFromBottom(80);  // Keyboard unten
    auto adsrArea = bounds.removeFromBottom(180);     // ADSR darüber

    // Kleiner Abstand zwischen den Bereichen
    bounds.removeFromBottom(10); // Abstand zum ADSR
    adsrArea.removeFromBottom(10); // Abstand zwischen ADSR und Keyboard

    // Verbleibendes Layout: links Bedienelemente, rechts Visualisierungen
    auto leftColumn = bounds.removeFromLeft(bounds.getWidth() / 2);
    auto rightColumn = bounds;

    // Rechten Bereich für Reverb-Slider reservieren
    auto reverbArea = rightSliderArea.removeFromLeft(80);
    auto bitCrusherArea = rightSliderArea;

    // Reverb-Slider (vertikal, rechts)
    auto reverbLabelArea = reverbArea.removeFromTop(20);
    reverbLabel.setBounds(reverbLabelArea);
    reverbSlider.setBounds(reverbArea.reduced(10));

    auto bitCrusherLabelArea = bitCrusherArea.removeFromTop(20);
    bitCrusherLabel.setBounds(bitCrusherLabelArea);
    bitCrusherSlider.setBounds(bitCrusherArea.reduced(10));

    // Linke Spalte: Bedienelemente vertikal stapeln
    auto controlHeight = 40;
    gainSlider.setBounds(leftColumn.removeFromTop(controlHeight + 20));
    gainLabel.setBounds(gainSlider.getX(), gainSlider.getY(), gainSlider.getWidth(), 20);

    oscTypeComboBox.setBounds(leftColumn.removeFromTop(controlHeight + 10)); // Mehr Abstand für ComboBox

    // Vowel-Morph weiter unten platzieren
    leftColumn.removeFromTop(20); // Zusätzlicher Abstand
    vowelMorphSlider.setBounds(leftColumn.removeFromTop(controlHeight + 20));
    vowelMorphLabel.setBounds(vowelMorphSlider.getX(), vowelMorphSlider.getY() - 20, vowelMorphSlider.getWidth(), 20);

    // Rechte Spalte: oben das Bild, darunter die Waveform
    auto imageArea = rightColumn.removeFromTop(100);
    oscImage.setBounds(imageArea.reduced(10));
    waveformComponent.setBounds(rightColumn.reduced(10));

    // Komponenten die die volle Breite einnehmen (unterhalb der Spalten)
    adsrComponent.setBounds(adsrArea.reduced(10, 5)); // Horizontal 10px, vertikal 5px Padding
    keyboardComponent.setBounds(keyboardArea);
}

void AvSynthAudioProcessorEditor::updateColorTheme(int oscTypeIndex)
{
    currentOscType = oscTypeIndex;

    // Farbschema je nach Oszillator-Typ
    switch (oscTypeIndex)
    {
        case 0: // Sine - Rot
            primaryColor = juce::Colour(220, 50, 50);      // Helles Rot
            secondaryColor = juce::Colour(120, 20, 20);    // Dunkles Rot
            break;
        case 1: // Saw - Blau
            primaryColor = juce::Colour(50, 120, 220);     // Helles Blau
            secondaryColor = juce::Colour(20, 60, 120);    // Dunkles Blau
            break;
        case 2: // Square - Grün
            primaryColor = juce::Colour(50, 200, 80);      // Helles Grün
            secondaryColor = juce::Colour(20, 100, 40);    // Dunkles Grün
            break;
        case 3: // Triangle - Gelb
            primaryColor = juce::Colour(220, 200, 50);     // Helles Gelb
            secondaryColor = juce::Colour(150, 120, 20);   // Dunkles Gelb/Orange
            break;
        default:
            primaryColor = juce::Colours::orange;
            secondaryColor = juce::Colours::darkorange;
            break;
    }

    // LookAndFeel mit neuen Farben aktualisieren
    customLookAndFeel.updateColors(primaryColor, secondaryColor);

    // Label-Farben aktualisieren
    reverbLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    vowelMorphLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    presetLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Komponenten zum Neuzeichnen zwingen
    repaint();

    // Alle Slider neu zeichnen
    for (auto* comp : GetComps())
    {
        comp->repaint();
    }

    // Preset-Buttons neu zeichnen
    toadPreset1Button.repaint();
    toadPreset2Button.repaint();
    toadPreset3Button.repaint();
    toadPreset4Button.repaint();
}

void AvSynthAudioProcessorEditor::updateOscImage(int oscTypeIndex)
{
    const void* imageData = nullptr;
    int imageSize = 0;

    switch (oscTypeIndex)
    {
        case 0:
            imageData = ToadyAssets::sine_wave_png;
            imageSize = ToadyAssets::sine_wave_pngSize;
            break;
        case 1:
            imageData = ToadyAssets::sawtooth_wave_png;
            imageSize = ToadyAssets::sawtooth_wave_pngSize;
            break;
        case 2:
            imageData = ToadyAssets::square_wave_png;
            imageSize = ToadyAssets::square_wave_pngSize;
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

    if (imageData != nullptr && imageSize > 0)
    {
        auto image = juce::ImageCache::getFromMemory(imageData, imageSize);
        oscImage.setImage(image, juce::RectanglePlacement::centred);
    }
    else
    {
        DBG("Fehler beim Laden des Bildes für Oscillator-Typ " << oscTypeIndex);
    }
}

void AvSynthAudioProcessorEditor::timerCallback()
{
    // ADSR-Plotter mit aktuellen Werten updaten
    float currentValue = processorRef.getCurrentEnvelopeValue();
    bool isActive = processorRef.isEnvelopeActive();
    int state = processorRef.getADSRState();

    adsrComponent.updateEnvelopeValue(currentValue, isActive);
    adsrComponent.setADSRState(state);

    // WICHTIG: ADSR Component mit aktuellen Parameter-Werten synchronisieren
    // Dies stellt sicher, dass externe Parameter-Änderungen (z.B. Host-Automation)
    // in der GUI reflektiert werden
    static float lastAttack = -1.0f, lastDecay = -1.0f, lastSustain = -1.0f, lastRelease = -1.0f;

    float currentAttack = processorRef.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Attack>().data())->load();
    float currentDecay = processorRef.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Decay>().data())->load();
    float currentSustain = processorRef.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Sustain>().data())->load();
    float currentRelease = processorRef.parameters.getRawParameterValue(
        magic_enum::enum_name<AvSynthAudioProcessor::Parameters::Release>().data())->load();

    // Nur updaten wenn sich Werte geändert haben (Performance)
    if (!juce::approximatelyEqual(currentAttack, lastAttack) ||
        !juce::approximatelyEqual(currentDecay, lastDecay) ||
        !juce::approximatelyEqual(currentSustain, lastSustain) ||
        !juce::approximatelyEqual(currentRelease, lastRelease))
    {
        adsrComponent.setAttack(currentAttack);
        adsrComponent.setDecay(currentDecay);
        adsrComponent.setSustain(currentSustain);
        adsrComponent.setRelease(currentRelease);

        lastAttack = currentAttack;
        lastDecay = lastDecay;
        lastSustain = currentSustain;
        lastRelease = currentRelease;
    }
}

void AvSynthAudioProcessorEditor::comboBoxChanged(juce::ComboBox *comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &oscTypeComboBox)
    {
        int newOscType = oscTypeComboBox.getSelectedItemIndex();
        updateColorTheme(newOscType);
        updateOscImage(newOscType);
    }
}

juce::Colour AvSynthAudioProcessorEditor::getCurrentPrimaryColor() const
{
    return primaryColor;
}

juce::Colour AvSynthAudioProcessorEditor::getCurrentSecondaryColor() const
{
    return secondaryColor;
}

std::vector<juce::Component *> AvSynthAudioProcessorEditor::GetComps()
{
    return {
        &gainSlider, &frequencySlider, &oscTypeComboBox, &vowelMorphSlider,
        &reverbSlider, &bitCrusherSlider, &keyboardComponent,
        &waveformComponent, &adsrComponent
    };
}