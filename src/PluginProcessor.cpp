#include "PluginProcessor.hpp"
#include "PluginEditor.hpp"
#include "magic_enum/magic_enum.hpp"

/**
 * @brief Template helper for creating parameters with magic_enum
 */
template <typename ParamT, AvSynthAudioProcessor::Parameters Param, typename... Args>
static std::unique_ptr<ParamT> makeParameter(Args &&...args) {
    return std::make_unique<ParamT>(magic_enum::enum_name<Param>().data(), magic_enum::enum_name<Param>().data(),
                                    std::forward<Args>(args)...);
}

//==============================================================================
// ChainSettings Implementation

AvSynthAudioProcessor::ChainSettings
AvSynthAudioProcessor::ChainSettings::Get(const juce::AudioProcessorValueTreeState &parameters) {
    ChainSettings settings{};

    // Load parameter values using magic_enum to convert enum to string
    settings.gain = parameters.getRawParameterValue(magic_enum::enum_name<Parameters::Gain>().data())->load();
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
// Constructor/Destructor

AvSynthAudioProcessor::AvSynthAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      ),
      parameters(*this, nullptr, "Parameters", createParameterLayout()),
      circularBuffer(1, 1024) {
}

AvSynthAudioProcessor::~AvSynthAudioProcessor() = default;

//==============================================================================
// AudioProcessor Implementation

const juce::String AvSynthAudioProcessor::getName() const {
    return JucePlugin_Name;
}

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

double AvSynthAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int AvSynthAudioProcessor::getNumPrograms() {
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int AvSynthAudioProcessor::getCurrentProgram() {
    return 0;
}

void AvSynthAudioProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String AvSynthAudioProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void AvSynthAudioProcessor::changeProgramName(int index, const juce::String &newName) {
    juce::ignoreUnused(index, newName);
}

//==============================================================================
// Playback Preparation

void AvSynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::ignoreUnused(sampleRate);

    // Initialize previous settings
    previousChainSettings = ChainSettings::Get(parameters);

    // Setup circular buffer for visualization
    circularBuffer.setSize(1, samplesPerBlock);

    // Initialize oscillator
    updateAngleDelta(previousChainSettings.frequency);

    // Prepare effects chain
    effectsChain.prepare(sampleRate, samplesPerBlock, 2); // Stereo for reverb

    // Setup ADSR envelope
    envelope.setSampleRate(sampleRate);
    ADSREnvelope::Parameters adsrParams(
        previousChainSettings.attack,
        previousChainSettings.decay,
        previousChainSettings.sustain,
        previousChainSettings.release
    );
    envelope.setParameters(adsrParams);
}

void AvSynthAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    effectsChain.reset();
    envelope.reset();
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

//==============================================================================
// Main Audio Processing

void AvSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    const auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    // Get current settings
    const auto chainSettings = ChainSettings::Get(parameters);

    // Update ADSR parameters if changed
    if (!juce::approximatelyEqual(chainSettings.attack, previousChainSettings.attack) ||
        !juce::approximatelyEqual(chainSettings.decay, previousChainSettings.decay) ||
        !juce::approximatelyEqual(chainSettings.sustain, previousChainSettings.sustain) ||
        !juce::approximatelyEqual(chainSettings.release, previousChainSettings.release)) {

        ADSREnvelope::Parameters adsrParams(
            chainSettings.attack, chainSettings.decay,
            chainSettings.sustain, chainSettings.release
        );
        envelope.setParameters(adsrParams);
    }

    // Process MIDI and keyboard state
    keyboardState.processNextMidiBuffer(midiMessages, 0, numSamples, true);
    processMidiMessages(midiMessages, numSamples);

    // Generate audio samples
    generateAudioSamples(buffer, numSamples, chainSettings);

    // Apply effects
    effectsChain.processBlock(buffer, chainSettings.reverbAmount, chainSettings.bitCrusherRate);

    // Apply gain
    bool gainUnchanged = juce::approximatelyEqual(chainSettings.gain, previousChainSettings.gain);
    for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
        if (gainUnchanged) {
            buffer.applyGain(channel, 0, numSamples, chainSettings.gain);
        } else {
            buffer.applyGainRamp(channel, 0, numSamples, previousChainSettings.gain, chainSettings.gain);
        }
    }

    // Update visualization buffer
    updateVisualizationBuffer(buffer, numSamples);

    // Store settings for next block
    previousChainSettings = chainSettings;
}

void AvSynthAudioProcessor::processMidiMessages(const juce::MidiBuffer& midiMessages, int numSamples) {
    for (const auto metadata : midiMessages) {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn()) {
            currentNoteFrequency = msg.getMidiNoteInHertz(msg.getNoteNumber());
            noteIsActive = true;
            envelope.noteOn();

            // Update frequency parameter (optional)
            auto* freqParam = parameters.getParameter(magic_enum::enum_name<Parameters::Frequency>().data());
            if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(freqParam)) {
                float normValue = floatParam->convertTo0to1(currentNoteFrequency);
                floatParam->setValueNotifyingHost(normValue);
            }

            updateAngleDelta(currentNoteFrequency);
        }
        else if (msg.isNoteOff()) {
            envelope.noteOff();
        }
    }
}

void AvSynthAudioProcessor::generateAudioSamples(juce::AudioBuffer<float>& buffer, int numSamples, const ChainSettings& chainSettings) {
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Generate audio if note is active or envelope is still releasing
    if (noteIsActive || envelope.isActive()) {
        for (int sample = 0; sample < numSamples; ++sample) {
            // Generate base oscillator sample with vowel morphing
            float currentSample = VowelFilter::getVowelMorphSample(
                chainSettings.oscType,
                static_cast<float>(currentAngle),
                chainSettings.VowelMorph
            );

            currentAngle += angleDelta;

            // Apply ADSR envelope
            float adsrValue = envelope.getNextSample();
            currentSample *= adsrValue;

            // Update envelope value for UI (every 4th sample for performance)
            if (sample % 4 == 0) {
                currentEnvelopeValue.store(adsrValue);
            }

            // If envelope is no longer active, stop note
            if (!envelope.isActive()) {
                noteIsActive = false;
                currentEnvelopeValue.store(0.0f);
            }

            // Write to all output channels
            for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
                buffer.setSample(channel, sample, currentSample);
            }
        }
    } else {
        // No active note or envelope - output silence
        buffer.clear();
        currentEnvelopeValue.store(0.0f);
    }
}

void AvSynthAudioProcessor::updateVisualizationBuffer(const juce::AudioBuffer<float>& buffer, int numSamples) {
    if (buffer.getNumChannels() > 0) {
        const float* channelData = buffer.getReadPointer(0);
        for (int sample = 0; sample < numSamples; ++sample) {
            circularBuffer.writeSample(0, channelData[sample]);
            circularBuffer.advanceWritePosition();
        }
        bufferWritePos = circularBuffer.getWritePosition();
    }
}

//==============================================================================
// Utility Methods

void AvSynthAudioProcessor::updateAngleDelta(float frequency) {
    auto sampleRate = getSampleRate();
    if (sampleRate <= 0.0) {
        angleDelta = 0.0;
        return;
    }

    angleDelta = OscillatorUtils::calculateAngleDelta(frequency, sampleRate);
}

bool AvSynthAudioProcessor::loadPreset(int presetIndex) {
    const PresetData* preset = presetManager.getPreset(presetIndex);
    if (!preset) {
        return false;
    }

    // Set all parameters
    auto* gainParam = parameters.getParameter(magic_enum::enum_name<Parameters::Gain>().data());
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(gainParam)) {
        floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset->gain));
    }

    auto* oscTypeParam = parameters.getParameter(magic_enum::enum_name<Parameters::OscType>().data());
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(oscTypeParam)) {
        choiceParam->setValueNotifyingHost(choiceParam->convertTo0to1(preset->oscType));
    }

    auto* vowelParam = parameters.getParameter(magic_enum::enum_name<Parameters::VowelMorph>().data());
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(vowelParam)) {
        floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset->vowelMorph));
    }

    auto* reverbParam = parameters.getParameter(magic_enum::enum_name<Parameters::ReverbAmount>().data());
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(reverbParam)) {
        floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset->reverbAmount));
    }

    auto* bitCrusherParam = parameters.getParameter(magic_enum::enum_name<Parameters::BitCrusherRate>().data());
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(bitCrusherParam)) {
        floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset->bitCrusherRate));
    }

    auto* attackParam = parameters.getParameter(magic_enum::enum_name<Parameters::Attack>().data());
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(attackParam)) {
        floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset->attack));
    }

    auto* decayParam = parameters.getParameter(magic_enum::enum_name<Parameters::Decay>().data());
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(decayParam)) {
        floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset->decay));
    }

    auto* sustainParam = parameters.getParameter(magic_enum::enum_name<Parameters::Sustain>().data());
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(sustainParam)) {
        floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset->sustain));
    }

    auto* releaseParam = parameters.getParameter(magic_enum::enum_name<Parameters::Release>().data());
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(releaseParam)) {
        floatParam->setValueNotifyingHost(floatParam->convertTo0to1(preset->release));
    }

    return true;
}

//==============================================================================
// Editor Methods

bool AvSynthAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *AvSynthAudioProcessor::createEditor() {
    return new AvSynthAudioProcessorEditor(*this);
    // return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
// State Save/Load

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

//==============================================================================
// Parameter Layout Creation

juce::AudioProcessorValueTreeState::ParameterLayout AvSynthAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::Gain>(0.0f, 1.0f, 0.25f));

    layout.add(makeParameter<juce::AudioParameterFloat, Parameters::Frequency>(
        juce::NormalisableRange(20.0f, 20000.0f, 1.0f, 0.3f), 440.0f));

    // Oscillator type parameter - corrected order to match actual waveforms
    layout.add(makeParameter<juce::AudioParameterChoice, Parameters::OscType>(
        juce::StringArray{magic_enum::enum_name<OscType::Sine>().data(),
                          magic_enum::enum_name<OscType::Square>().data(),
                          magic_enum::enum_name<OscType::Saw>().data(),
                          magic_enum::enum_name<OscType::Triangle>().data()},
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
// Plugin Entry Point

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new AvSynthAudioProcessor();
}