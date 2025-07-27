#pragma once

#include "JuceHeader.h"
#include "juce_dsp/juce_dsp.h"
#include "Oscillator.hpp"
#include "VowelFilter.hpp"
#include "AudioEffects.hpp"
#include "PresetManager.hpp"
#include "Utils.hpp"

/**
 * @file PluginProcessor.hpp
 * @brief Main audio processor class for the AvSynth audio plugin
 */

/**
 * @brief Main audio processor class implementing the AvSynth synthesizer
 *
 * This class handles all audio processing, parameter management, and MIDI input
 * for the AvSynth audio plugin. It integrates oscillators, effects, and preset management.
 */
class AvSynthAudioProcessor final : public juce::AudioProcessor {
    friend class AvSynthAudioProcessorEditor;

public:
    /**
     * @brief Enumeration of all plugin parameters
     */
    enum class Parameters {
        Gain,           ///< Main output gain
        Frequency,      ///< Oscillator frequency
        OscType,        ///< Oscillator type selection
        VowelMorph,     ///< Vowel morphing amount
        ReverbAmount,   ///< Reverb effect amount
        BitCrusherRate, ///< Bit crusher rate
        Attack,         ///< ADSR attack time
        Decay,          ///< ADSR decay time
        Sustain,        ///< ADSR sustain level
        Release,        ///< ADSR release time
        NumParameters   ///< Total number of parameters
    };

    /**
     * @brief Structure containing all chain settings derived from parameters
     */
    struct ChainSettings {
        float gain = 0.25f;                 ///< Output gain (0.0 to 1.0)
        float frequency = 440.0f;           ///< Oscillator frequency in Hz
        OscType oscType = OscType::Sine;    ///< Selected oscillator type
        float VowelMorph = 0.0f;           ///< Vowel morphing value (0.0 = "A", 1.0 = "U")
        float reverbAmount = 0.0f;         ///< Reverb amount (0.0 to 1.0)
        float bitCrusherRate = 0.01f;      ///< Bit crusher rate (0.01 to 1.0)
        float attack = 0.1f;               ///< ADSR attack (0.0 to 1.0)
        float decay = 0.3f;                ///< ADSR decay (0.0 to 1.0)
        float sustain = 0.7f;              ///< ADSR sustain (0.0 to 1.0)
        float release = 0.5f;              ///< ADSR release (0.0 to 1.0)

        /**
         * @brief Create ChainSettings from current parameter values
         * @param parameters Reference to the parameter tree state
         * @return ChainSettings structure with current values
         */
        static forcedinline ChainSettings Get(const juce::AudioProcessorValueTreeState &parameters);
    };

public:
    /**
     * @brief Constructor
     */
    AvSynthAudioProcessor();

    /**
     * @brief Destructor
     */
    ~AvSynthAudioProcessor() override;

    //==============================================================================
    // AudioProcessor overrides

    /**
     * @brief Prepare the processor for playback
     * @param sampleRate Sample rate in Hz
     * @param samplesPerBlock Expected maximum samples per block
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    /**
     * @brief Release resources when playback stops
     */
    void releaseResources() override;

    /**
     * @brief Check if a bus layout is supported
     * @param layouts The bus layout to check
     * @return True if layout is supported
     */
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    /**
     * @brief Process a block of audio data
     * @param buffer Audio buffer containing input/output audio data
     * @param midiMessages MIDI messages to be processed
     */
    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    // Editor methods

    /**
     * @brief Create the plugin editor
     * @return Pointer to the created editor
     */
    juce::AudioProcessorEditor *createEditor() override;

    /**
     * @brief Check if the processor has an editor
     * @return True if editor is available
     */
    bool hasEditor() const override;

    //==============================================================================
    // Plugin info methods

    /**
     * @brief Get the plugin name
     * @return Plugin name string
     */
    const juce::String getName() const override;

    /**
     * @brief Check if the plugin accepts MIDI input
     * @return True if MIDI input is accepted
     */
    bool acceptsMidi() const override;

    /**
     * @brief Check if the plugin produces MIDI output
     * @return True if MIDI output is produced
     */
    bool producesMidi() const override;

    /**
     * @brief Check if this is a MIDI effect
     * @return True if this is a MIDI effect
     */
    bool isMidiEffect() const override;

    /**
     * @brief Get the tail length in seconds
     * @return Tail length in seconds
     */
    double getTailLengthSeconds() const override;

    //==============================================================================
    // Program/preset methods

    /**
     * @brief Get the number of programs
     * @return Number of programs
     */
    int getNumPrograms() override;

    /**
     * @brief Get the current program index
     * @return Current program index
     */
    int getCurrentProgram() override;

    /**
     * @brief Set the current program
     * @param index Program index to set
     */
    void setCurrentProgram(int index) override;

    /**
     * @brief Get program name by index
     * @param index Program index
     * @return Program name
     */
    const juce::String getProgramName(int index) override;

    /**
     * @brief Change program name
     * @param index Program index
     * @param newName New program name
     */
    void changeProgramName(int index, const juce::String &newName) override;

    //==============================================================================
    // State save/load methods

    /**
     * @brief Save plugin state to memory block
     * @param destData Memory block to store state data
     */
    void getStateInformation(juce::MemoryBlock &destData) override;

    /**
     * @brief Restore plugin state from memory block
     * @param data Pointer to state data
     * @param sizeInBytes Size of state data in bytes
     */
    void setStateInformation(const void *data, int sizeInBytes) override;

    //==============================================================================
    // Public utility methods

    /**
     * @brief Update oscillator angle delta for given frequency
     * @param frequency Frequency in Hz
     */
    void updateAngleDelta(float frequency);

    /**
     * @brief Get current envelope value for UI display
     * @return Current envelope value (0.0 to 1.0)
     */
    float getCurrentEnvelopeValue() const { return currentEnvelopeValue.load(); }

    /**
     * @brief Check if envelope is currently active
     * @return True if envelope is active
     */
    bool isEnvelopeActive() const { return envelope.isActive(); }

    /**
     * @brief Get current ADSR state for UI display
     * @return Current ADSR state (0=idle, 1=attack, 2=decay, 3=sustain, 4=release)
     */
    int getADSRState() const { return static_cast<int>(envelope.getState()); }

    /**
     * @brief Get reference to preset manager
     * @return Reference to preset manager
     */
    PresetManager& getPresetManager() { return presetManager; }

    /**
     * @brief Get const reference to preset manager
     * @return Const reference to preset manager
     */
    const PresetManager& getPresetManager() const { return presetManager; }

    /**
     * @brief Load a preset by index
     * @param presetIndex Index of preset to load
     * @return True if preset was loaded successfully
     */
    bool loadPreset(int presetIndex);

private:
    /**
     * @brief Create the parameter layout for the value tree state
     * @return Parameter layout structure
     */
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    /**
     * @brief Process MIDI messages and update synthesizer state
     * @param midiMessages MIDI buffer to process
     * @param numSamples Number of samples in current block
     */
    void processMidiMessages(const juce::MidiBuffer& midiMessages, int numSamples);

    /**
     * @brief Generate audio samples using current oscillator and settings
     * @param buffer Audio buffer to fill
     * @param numSamples Number of samples to generate
     * @param chainSettings Current parameter settings
     */
    void generateAudioSamples(juce::AudioBuffer<float>& buffer, int numSamples, const ChainSettings& chainSettings);

    /**
     * @brief Update the circular buffer for waveform visualization
     * @param buffer Source audio buffer
     * @param numSamples Number of samples to copy
     */
    void updateVisualizationBuffer(const juce::AudioBuffer<float>& buffer, int numSamples);

public:
    //==============================================================================
    // Public member variables (for editor access)

    juce::AudioProcessorValueTreeState parameters; ///< Parameter tree state
    juce::MidiKeyboardState keyboardState;         ///< MIDI keyboard state
    CircularAudioBuffer circularBuffer;            ///< Buffer for waveform visualization
    int bufferWritePos{0};             ///< Write position for circular buffer

private:
    //==============================================================================
    // Private member variables

    // Core synthesis components
    VowelFilter vowelFilter;                        ///< Vowel morphing filter
    EffectsChain effectsChain;                      ///< Audio effects chain
    ADSREnvelope envelope;                          ///< ADSR envelope generator
    PresetManager presetManager;                    ///< Preset management system

    // Synthesis state
    ChainSettings previousChainSettings;            ///< Previous parameter settings for change detection
    double currentAngle = 0.0;                      ///< Current oscillator phase angle
    double angleDelta = 0.0;                        ///< Phase increment per sample
    bool noteIsActive = false;                      ///< Current note activity state
    float currentNoteFrequency = 0.0f;              ///< Current note frequency in Hz

    // Thread-safe UI communication
    std::atomic<float> currentEnvelopeValue{0.0f};  ///< Current envelope value for UI

    // Utility objects
    juce::Random random;                            ///< Random number generator

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AvSynthAudioProcessor)
};