#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"
#include "Utils.hpp"
#include "magic_enum/magic_enum.hpp"

/**
 * @brief Retrieves the current parameter values from the ValueTreeState
 * This method creates a new ChainSettings object and populates it with current parameter values
 */
AvSynthAudioProcessor::ChainSettings
AvSynthAudioProcessor::ChainSettings::Get(const juce::AudioProcessorValueTreeState &parameters) {
    ChainSettings settings{};

    // Load the gain parameter value using magic_enum to convert enum to string
    settings.gain = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::Gain>().data())->load();
    // Load the frequency parameter value using magic_enum to convert enum to string
    settings.frequency = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::Frequency>().data())->load();
    settings.oscType = static_cast<OscType>(
        static_cast<int>(parameters.getRawParameterValue(magic_enum::enum_name<Parameters::OscType>().data())->load()));
    settings.VowelMorph = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::VowelMorph>().data())->load();
    settings.reverbAmount = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::ReverbAmount>().data())->load();
    settings.bitCrusherRate = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::BitCrusherRate>().data())->load();
    settings.attack = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::Attack>().data())->load();
    settings.decay = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::Decay>().data())->load();
    settings.sustain = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::Sustain>().data())->load();
    settings.release = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::Release>().data())->load();

    return settings;
}

//==============================================================================
AvSynthAudioProcessor::AvSynthAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      ) {
}

AvSynthAudioProcessor::~AvSynthAudioProcessor() {}

//==============================================================================
const juce::String AvSynthAudioProcessor::getName() const { return JucePlugin_Name; }

bool AvSynthAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AvSynthAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AvSynthAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AvSynthAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AvSynthAudioProcessor::getNumPrograms() {
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int AvSynthAudioProcessor::getCurrentProgram() { return 0; }

void AvSynthAudioProcessor::setCurrentProgram(int index) { juce::ignoreUnused(index); }

const juce::String AvSynthAudioProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void AvSynthAudioProcessor::changeProgramName(int index, const juce::String &newName) { juce::ignoreUnused(index, newName); }

//==============================================================================
void AvSynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused(sampleRate);

    previousChainSettings = ChainSettings::Get(parameters);
    circularBuffer.setSize(1, samplesPerBlock);

    updateAngleDelta(previousChainSettings.frequency);

    reverbSpec.sampleRate = sampleRate;
    reverbSpec.maximumBlockSize = samplesPerBlock;
    reverbSpec.numChannels = 2; // Stereo für Reverb

    reverb.prepare(reverbSpec);
    updateReverbParameters(previousChainSettings.reverbAmount);

    adsr.setSampleRate(sampleRate);
    updateADSRParameters(previousChainSettings.attack, previousChainSettings.decay,
                        previousChainSettings.sustain, previousChainSettings.release);

}

void AvSynthAudioProcessor::updateReverbParameters(float reverbAmount) {
    juce::dsp::Reverb::Parameters reverbParams;

    // Reverb-Parameter basierend auf reverbAmount (0.0 bis 1.0) setzen
    reverbParams.roomSize = juce::jmap(reverbAmount, 0.0f, 0.8f);        // Raumgröße
    reverbParams.damping = juce::jmap(reverbAmount, 0.2f, 0.6f);         // Dämpfung
    reverbParams.wetLevel = juce::jmap(reverbAmount, 0.0f, 0.4f);        // Wet-Signal
    reverbParams.dryLevel = 1.0f - (reverbAmount * 0.3f);               // Dry-Signal bleibt dominant
    reverbParams.width = 1.0f;                                          // Stereo-Breite
    reverbParams.freezeMode = 0.0f;                                     // Kein Freeze

    reverb.setParameters(reverbParams);
}

void AvSynthAudioProcessor::updateADSRParameters(float attack, float decay, float sustain, float release) {
    adsrParams.attack = juce::jmap(attack, 0.0f, 1.0f, 0.01f, 3.0f);     // 0.01s bis 3s
    adsrParams.decay = juce::jmap(decay, 0.0f, 1.0f, 0.01f, 3.0f);       // 0.01s bis 3s
    adsrParams.sustain = sustain;                                         // 0.0 bis 1.0 (direkter Wert)
    adsrParams.release = juce::jmap(release, 0.0f, 1.0f, 0.01f, 5.0f);   // 0.01s bis 5s

    adsr.setParameters(adsrParams);
}


void AvSynthAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AvSynthAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

/**
 * @brief Main audio processing function called by the host to process a block of audio data
 * @param buffer Audio buffer containing the input/output audio data
 * @param midiMessages MIDI messages to be processed
 */
void AvSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    // Einstellungen laden
    const auto chainSettings = ChainSettings::Get(parameters);

    // ADSR-Parameter ZUERST aktualisieren (vor MIDI-Verarbeitung)
    if (!juce::approximatelyEqual(chainSettings.attack, previousChainSettings.attack) ||
        !juce::approximatelyEqual(chainSettings.decay, previousChainSettings.decay) ||
        !juce::approximatelyEqual(chainSettings.sustain, previousChainSettings.sustain) ||
        !juce::approximatelyEqual(chainSettings.release, previousChainSettings.release)) {
        updateADSRParameters(chainSettings.attack, chainSettings.decay, chainSettings.sustain, chainSettings.release);
    }

    // Standard-MIDI-Verarbeitung
    keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);

    // MIDI-Note Handling
    for (const auto metadata : midiMessages) {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn()) {
            currentNoteFrequency = msg.getMidiNoteInHertz(msg.getNoteNumber());
            noteIsActive = true;

            adsr.noteOn();

            // Frequenz-Parameter updaten (optional)
            auto* freqParam = parameters.getParameter(magic_enum::enum_name<Parameters::Frequency>().data());
            if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(freqParam)) {
                float normValue = floatParam->convertTo0to1(currentNoteFrequency);
                floatParam->setValueNotifyingHost(normValue);
            }

            updateAngleDelta(currentNoteFrequency);
        }
        else if (msg.isNoteOff()) {
            adsr.noteOff();
        }
    }

    // Gain vorbereiten
    bool gainUnchanged = juce::approximatelyEqual(chainSettings.gain, previousChainSettings.gain);

    // Rest der processBlock-Funktion bleibt unverändert...
    // [Oszillator-Ausgabe, Reverb, BitCrusher, etc.]

    // Oszillator-Ausgabe
    if (noteIsActive || adsr.isActive()) {
        float vowelMorphValue = chainSettings.VowelMorph;

        for (int sample = 0; sample < numSamples; ++sample) {
            float currentSample = getVowelMorphSample(chainSettings.oscType, currentAngle, vowelMorphValue);
            currentAngle += angleDelta;

            // ADSR-Hüllkurve anwenden
            float adsrValue = adsr.getNextSample();
            currentSample *= adsrValue;

            // ADSR-Wert für UI speichern (nur jeden 4. Sample für Performance)
            if (sample % 4 == 0) {
                currentEnvelopeValue.store(adsrValue);

                // ADSR-Zustand bestimmen
                static float lastValue = 0.0f;
                float delta = adsrValue - lastValue;

                if (adsrValue < 0.01f && !noteIsActive) {
                    currentADSRState.store(0); // Idle
                } else if (delta > 0.001f && adsrValue < 0.99f) {
                    currentADSRState.store(1); // Attack
                } else if (delta < -0.001f && adsrValue > adsrParams.sustain + 0.1f) {
                    currentADSRState.store(2); // Decay
                } else if (std::abs(delta) < 0.001f && noteIsActive) {
                    currentADSRState.store(3); // Sustain
                } else if (delta < -0.001f && !noteIsActive) {
                    currentADSRState.store(4); // Release
                }

                lastValue = adsrValue;
            }

            // Wenn ADSR nicht mehr aktiv ist, Note beenden
            if (!adsr.isActive()) {
                noteIsActive = false;
                currentEnvelopeValue.store(0.0f);
                currentADSRState.store(0);
            }

            for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
                buffer.setSample(channel, sample, currentSample);
            }
        }
    }
    else {
        buffer.clear(); // Kein Ton -> Stille ausgeben
        currentEnvelopeValue.store(0.0f);
        currentADSRState.store(0);
    }

    // Reverb-Parameter aktualisieren
    if (!juce::approximatelyEqual(chainSettings.reverbAmount, previousChainSettings.reverbAmount)) {
        updateReverbParameters(chainSettings.reverbAmount);
    }

    // Reverb-Verarbeitung (nur wenn reverbAmount > 0)
    if (chainSettings.reverbAmount > 0.0f) {
        juce::dsp::AudioBlock<float> reverbBlock(buffer);
        juce::dsp::ProcessContextReplacing<float> reverbContext(reverbBlock);
        reverb.process(reverbContext);
    }

    // BitCrusher
    if (chainSettings.bitCrusherRate < 1.0f) {
        const float crushFactor = chainSettings.bitCrusherRate;
        const float inverse = 1.0f / crushFactor;

        for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
            float* samples = buffer.getWritePointer(channel);
            for (int i = 0; i < numSamples; ++i) {
                samples[i] = std::round(samples[i] * inverse) * crushFactor;
            }
        }
    }

    // Lautstärke anwenden
    for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
        if (gainUnchanged) {
            buffer.applyGain(channel, 0, numSamples, chainSettings.gain);
        }
        else {
            buffer.applyGainRamp(channel, 0, numSamples, previousChainSettings.gain, chainSettings.gain);
        }
    }

    // Buffer für Waveform-Visualisierung updaten
    const float* channelData = buffer.getReadPointer(0);
    for (int sample = 0; sample < numSamples; ++sample) {
        circularBuffer.setSample(0, bufferWritePos, channelData[sample]);
        bufferWritePos = (bufferWritePos + 1) % circularBuffer.getNumSamples();
    }

    // Parameter speichern
    previousChainSettings = chainSettings;
}

float AvSynthAudioProcessor::getVowelMorphSample(OscType oscType, float angle, float vowelMorphValue) {
    // Basis-Sample nach OscType generieren
    float baseSample = getOscSample(oscType, angle);

    // Vokal-Formanten-Frequenzen (vereinfacht)
    // Diese Werte sind approximiert für charakteristische Formanten
    struct VowelFormants {
        float f1, f2, f3; // Erste drei Formanten
        float a1, a2, a3;  // Amplituden für die Formanten
    };

    // Vokal-Definitionen (Formanten in Hz, vereinfacht für Synthesizer-Nutzung)
    VowelFormants vowelA = {800.0f, 1200.0f, 2500.0f, 1.0f, 0.7f, 0.3f};  // "A"
    VowelFormants vowelE = {500.0f, 1800.0f, 2500.0f, 1.0f, 0.8f, 0.2f};  // "E"
    VowelFormants vowelI = {300.0f, 2300.0f, 3000.0f, 1.0f, 0.9f, 0.4f};  // "I"
    VowelFormants vowelO = {500.0f, 900.0f, 2200.0f, 1.0f, 0.6f, 0.2f};   // "O"
    VowelFormants vowelU = {300.0f, 700.0f, 2100.0f, 1.0f, 0.5f, 0.1f};   // "U"

    // Interpolation zwischen Vokalen basierend auf vowelMorphValue (0.0 bis 1.0)
    VowelFormants currentVowel;

    if (vowelMorphValue <= 0.25f) {
        // A zu E
        float t = vowelMorphValue * 4.0f;
        currentVowel.f1 = juce::jmap(t, vowelA.f1, vowelE.f1);
        currentVowel.f2 = juce::jmap(t, vowelA.f2, vowelE.f2);
        currentVowel.f3 = juce::jmap(t, vowelA.f3, vowelE.f3);
        currentVowel.a1 = juce::jmap(t, vowelA.a1, vowelE.a1);
        currentVowel.a2 = juce::jmap(t, vowelA.a2, vowelE.a2);
        currentVowel.a3 = juce::jmap(t, vowelA.a3, vowelE.a3);
    }
    else if (vowelMorphValue <= 0.5f) {
        // E zu I
        float t = (vowelMorphValue - 0.25f) * 4.0f;
        currentVowel.f1 = juce::jmap(t, vowelE.f1, vowelI.f1);
        currentVowel.f2 = juce::jmap(t, vowelE.f2, vowelI.f2);
        currentVowel.f3 = juce::jmap(t, vowelE.f3, vowelI.f3);
        currentVowel.a1 = juce::jmap(t, vowelE.a1, vowelI.a1);
        currentVowel.a2 = juce::jmap(t, vowelE.a2, vowelI.a2);
        currentVowel.a3 = juce::jmap(t, vowelE.a3, vowelI.a3);
    }
    else if (vowelMorphValue <= 0.75f) {
        // I zu O
        float t = (vowelMorphValue - 0.5f) * 4.0f;
        currentVowel.f1 = juce::jmap(t, vowelI.f1, vowelO.f1);
        currentVowel.f2 = juce::jmap(t, vowelI.f2, vowelO.f2);
        currentVowel.f3 = juce::jmap(t, vowelI.f3, vowelO.f3);
        currentVowel.a1 = juce::jmap(t, vowelI.a1, vowelO.a1);
        currentVowel.a2 = juce::jmap(t, vowelI.a2, vowelO.a2);
        currentVowel.a3 = juce::jmap(t, vowelI.a3, vowelO.a3);
    }
    else {
        // O zu U
        float t = (vowelMorphValue - 0.75f) * 4.0f;
        currentVowel.f1 = juce::jmap(t, vowelO.f1, vowelU.f1);
        currentVowel.f2 = juce::jmap(t, vowelO.f2, vowelU.f2);
        currentVowel.f3 = juce::jmap(t, vowelO.f3, vowelU.f3);
        currentVowel.a1 = juce::jmap(t, vowelO.a1, vowelU.a1);
        currentVowel.a2 = juce::jmap(t, vowelO.a2, vowelU.a2);
        currentVowel.a3 = juce::jmap(t, vowelO.a3, vowelU.a3);
    }

    // Formant-Filter-Simulation durch Überlagerung harmonischer Komponenten
    // Vereinfachter Ansatz: Moduliere das Signal mit den Formanten
    float formantSample = 0.0f;

    // Erste Formante (stärkste)
    float formant1 = std::sin(angle * currentVowel.f1 / 440.0f) * currentVowel.a1;
    formantSample += formant1 * 0.5f;

    // Zweite Formante
    float formant2 = std::sin(angle * currentVowel.f2 / 440.0f) * currentVowel.a2;
    formantSample += formant2 * 0.3f;

    // Dritte Formante (schwächste)
    float formant3 = std::sin(angle * currentVowel.f3 / 440.0f) * currentVowel.a3;
    formantSample += formant3 * 0.2f;

    // Mische das Original-Signal mit den Formanten
    float morphFactor = vowelMorphValue * 0.8f; // Maximal 80% Vokal-Anteil
    return juce::jmap(morphFactor, baseSample, baseSample * (1.0f + formantSample * 0.5f));
}

//==============================================================================
bool AvSynthAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *AvSynthAudioProcessor::createEditor() {
    return new AvSynthAudioProcessorEditor(*this);
    // return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
// Save plugin state - called by the host when saving project or plugin preset
void AvSynthAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // This method stores parameters in the memory block.
    // ValueTree is used as an intermediate to save complex parameter data
    // Parameters are written to a memory stream which is then stored in the destData block

    // Create output stream and write parameter state to it
    juce::MemoryOutputStream stream(destData, true);
    parameters.state.writeToStream(stream);
}

void AvSynthAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        parameters.replaceState(tree);
    }
}
void AvSynthAudioProcessor::updateAngleDelta(float frequency) {
    auto sampleRate = getSampleRate();
    if (sampleRate <= 0.0) {
        angleDelta = 0.0;
        return;
    }

    auto cyclesPerSample = frequency / sampleRate;
    angleDelta = cyclesPerSample * juce::MathConstants<double>::twoPi;
}

float AvSynthAudioProcessor::getOscSample(OscType type, double angle) {
    switch (type) {
    case OscType::Sine:
        return static_cast<float>(std::sin(angle));
    case OscType::Square:
        return (std::sin(angle) >= 0.0 ? 1.0f : -1.0f);
    case OscType::Saw:
        return static_cast<float>(
            2.0f * (angle / juce::MathConstants<double>::twoPi - std::floor(0.5 + angle / juce::MathConstants<double>::twoPi)));
    case OscType::Triangle:
        return 2.0f * static_cast<float>(std::abs(2.0f * (angle / juce::MathConstants<double>::twoPi -
                                                          std::floor(0.5 + angle / juce::MathConstants<double>::twoPi)))) -
               1.0f;
    default:
        return 0.0f;
    }
}

template <typename ParamT, AvSynthAudioProcessor::Parameters Param, typename... Args>
static std::unique_ptr<ParamT> makeParameter(Args &&...args) {
    return std::make_unique<ParamT>(magic_enum::enum_name<Param>().data(), magic_enum::enum_name<Param>().data(),
                                    std::forward<Args>(args)...);
}

juce::AudioProcessorValueTreeState::ParameterLayout AvSynthAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::Gain>(0.0f, 1.0f, 0.25f));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::Frequency>(
        juce::NormalisableRange(20.0f, 20000.0f, 1.0f, 0.3f), 440.0f));

    layout.add(makeParameter<juce::AudioParameterChoice, Parameters::OscType>(
        juce::StringArray{magic_enum::enum_name<OscType::Sine>().data(), magic_enum::enum_name<OscType::Saw>().data(),
                          magic_enum::enum_name<OscType::Square>().data(), magic_enum::enum_name<OscType::Triangle>().data()},
        0));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::VowelMorph>(
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::ReverbAmount>(
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::BitCrusherRate>(
        juce::NormalisableRange<float>(0.01f, 1.0f, 0.01f), 1.0f));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::Attack>(
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.1f));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::Decay>(
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::Sustain>(
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.7f));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::Release>(
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new AvSynthAudioProcessor(); }