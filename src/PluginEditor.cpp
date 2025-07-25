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

      keyboardComponent(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      waveformComponent(p.circularBuffer, p.bufferWritePos)
{
    juce::ignoreUnused(processorRef);

    setLookAndFeel(&customLookAndFeel);

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

    updateOscImage(oscTypeComboBox.getSelectedItemIndex()); // Initial image set

    for (auto *component : GetComps())
        addAndMakeVisible(component);

    addAndMakeVisible(oscImage); // Add image component

    setSize(600, 600);
    setResizable(true, true);
}

AvSynthAudioProcessorEditor::~AvSynthAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

//==============================================================================
void AvSynthAudioProcessorEditor::paint(juce::Graphics &g)
{
    juce::ColourGradient gradient(
        juce::Colours::darkslateblue,
        getLocalBounds().getTopLeft().toFloat(),
        juce::Colours::black,
        getLocalBounds().getBottomRight().toFloat(),
        false);

    g.setGradientFill(gradient);
    g.fillAll();
}

void AvSynthAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(10);

    // Linke und rechte Spalte aufteilen
    auto leftColumn = bounds.removeFromLeft(bounds.getWidth() / 2); // 50/50
    auto rightColumn = bounds;

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
        updateOscImage(oscTypeComboBox.getSelectedItemIndex());
}

std::vector<juce::Component *> AvSynthAudioProcessorEditor::GetComps()
{
    return {
        &gainSlider, &frequencySlider, &oscTypeComboBox, &lowCutFreqSlider,
        &highCutFreqSlider, &vowelMorphSlider, &keyboardComponent,
        &waveformComponent
    };
}
