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

      lowCutFreqSlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      lowCutFreqAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::LowPassFreq>().data(), lowCutFreqSlider),

      highCutFreqSlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      highCutFreqAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::HighPassFreq>().data(), highCutFreqSlider),

      vowelMorphSlider(juce::Slider::LinearHorizontal, juce::Slider::TextEntryBoxPosition::TextBoxLeft),
      vowelMorphAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::VowelMorph>().data(), vowelMorphSlider),

      // Reverb-Slider (vertikal)
      reverbSlider(juce::Slider::LinearVertical, juce::Slider::TextEntryBoxPosition::TextBoxBelow),
      reverbAttachment(p.parameters, magic_enum::enum_name<AvSynthAudioProcessor::Parameters::ReverbAmount>().data(), reverbSlider),

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

    setSize(650, 600);
    setResizable(true, true);
}

AvSynthAudioProcessorEditor::~AvSynthAudioProcessorEditor() {
    setLookAndFeel(nullptr);
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

    // Rechten Bereich für Reverb-Slider reservieren
    auto reverbArea = bounds.removeFromRight(80);

    // Verbleibendes Layout: links Bedienelemente, rechts Visualisierungen
    auto leftColumn = bounds.removeFromLeft(bounds.getWidth() / 2);
    auto rightColumn = bounds;

    // Reverb-Slider (vertikal, rechts)
    auto reverbLabelArea = reverbArea.removeFromTop(20);
    reverbLabel.setBounds(reverbLabelArea);
    reverbSlider.setBounds(reverbArea.reduced(10));

    // Linke Spalte: Bedienelemente vertikal stapeln
    auto controlHeight = 40;
    gainSlider.setBounds(leftColumn.removeFromTop(controlHeight));
    oscTypeComboBox.setBounds(leftColumn.removeFromTop(controlHeight));
    lowCutFreqSlider.setBounds(leftColumn.removeFromTop(controlHeight));
    highCutFreqSlider.setBounds(leftColumn.removeFromTop(controlHeight));
    vowelMorphSlider.setBounds(leftColumn.removeFromTop(controlHeight));
    keyboardComponent.setBounds(leftColumn.removeFromTop(80));

    // Rechte Spalte: oben das Bild, darunter die Waveform
    auto imageArea = rightColumn.removeFromTop(100);
    oscImage.setBounds(imageArea.reduced(10));

    waveformComponent.setBounds(rightColumn.reduced(10));
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

    // Reverb-Label-Farbe aktualisieren
    reverbLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // Komponenten zum Neuzeichnen zwingen
    repaint();

    // Alle Slider neu zeichnen
    for (auto* comp : GetComps())
    {
        comp->repaint();
    }
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
        &gainSlider, &frequencySlider, &oscTypeComboBox, &lowCutFreqSlider,
        &highCutFreqSlider, &vowelMorphSlider, &reverbSlider, &keyboardComponent,
        &waveformComponent
    };
}