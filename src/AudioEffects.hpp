#pragma once
#include "JuceHeader.h"
#include "juce_dsp/juce_dsp.h"

/**
 * @file AudioEffects.hpp
 * @brief Audio effects implementations for the AvSynth audio plugin
 */

/**
 * @brief Reverb effect wrapper with simplified controls
 */
class ReverbEffect {
public:
    /**
     * @brief Constructor
     */
    ReverbEffect();

    /**
     * @brief Destructor
     */
    ~ReverbEffect() = default;

    /**
     * @brief Prepare the reverb for processing
     * @param sampleRate Sample rate in Hz
     * @param maximumBlockSize Maximum expected block size
     * @param numChannels Number of audio channels
     */
    void prepare(double sampleRate, int maximumBlockSize, int numChannels);

    /**
     * @brief Process audio block with reverb
     * @param buffer Audio buffer to process
     */
    void processBlock(juce::AudioBuffer<float>& buffer);

    /**
     * @brief Set reverb amount
     * @param amount Reverb amount (0.0 to 1.0)
     */
    void setAmount(float amount);

    /**
     * @brief Get current reverb amount
     * @return Current reverb amount
     */
    float getAmount() const { return currentAmount; }

    /**
     * @brief Reset the reverb state
     */
    void reset();

private:
    /**
     * @brief Update internal reverb parameters based on amount
     */
    void updateParameters();

    juce::dsp::Reverb reverb;           ///< Internal JUCE reverb processor
    juce::dsp::ProcessSpec spec;        ///< Processing specification
    float currentAmount = 0.0f;         ///< Current reverb amount
    bool isPrepared = false;            ///< Preparation state flag
};

/**
 * @brief Bit crusher effect for digital distortion
 */
class BitCrusherEffect {
public:
    /**
     * @brief Constructor
     */
    BitCrusherEffect() = default;

    /**
     * @brief Destructor
     */
    ~BitCrusherEffect() = default;

    /**
     * @brief Process audio buffer with bit crushing
     * @param buffer Audio buffer to process
     * @param rate Bit crush rate (0.01 to 1.0, where 1.0 = no effect)
     */
    void processBlock(juce::AudioBuffer<float>& buffer, float rate);

    /**
     * @brief Process a single sample with bit crushing
     * @param sample Input sample
     * @param rate Bit crush rate (0.01 to 1.0)
     * @return Processed sample
     */
    float processSample(float sample, float rate);

    /**
     * @brief Set the bit crush rate
     * @param rate Bit crush rate (0.01 to 1.0)
     */
    void setRate(float rate) { this->rate = juce::jlimit(0.01f, 1.0f, rate); }

    /**
     * @brief Get current bit crush rate
     * @return Current rate
     */
    float getRate() const { return rate; }

private:
    float rate = 1.0f; ///< Current bit crush rate
};

/**
 * @brief ADSR envelope generator
 */
class ADSREnvelope {
public:
    /**
     * @brief ADSR parameters structure
     */
    struct Parameters {
        float attack = 0.1f;   ///< Attack time in seconds
        float decay = 0.3f;    ///< Decay time in seconds
        float sustain = 0.7f;  ///< Sustain level (0.0 to 1.0)
        float release = 0.5f;  ///< Release time in seconds

        /**
         * @brief Constructor with default values
         */
        Parameters() = default;

        /**
         * @brief Constructor with specific values
         * @param att Attack time
         * @param dec Decay time
         * @param sust Sustain level
         * @param rel Release time
         */
        Parameters(float att, float dec, float sust, float rel)
            : attack(att), decay(dec), sustain(sust), release(rel) {}
    };

    /**
     * @brief ADSR state enumeration
     */
    enum class State {
        Idle = 0,    ///< No note active
        Attack = 1,  ///< Attack phase
        Decay = 2,   ///< Decay phase
        Sustain = 3, ///< Sustain phase
        Release = 4  ///< Release phase
    };

    /**
     * @brief Constructor
     */
    ADSREnvelope();

    /**
     * @brief Destructor
     */
    ~ADSREnvelope() = default;

    /**
     * @brief Set sample rate
     * @param sampleRate Sample rate in Hz
     */
    void setSampleRate(double sampleRate);

    /**
     * @brief Set ADSR parameters
     * @param params ADSR parameters
     */
    void setParameters(const Parameters& params);

    /**
     * @brief Get current parameters
     * @return Current ADSR parameters
     */
    const Parameters& getParameters() const { return parameters; }

    /**
     * @brief Trigger note on (start attack phase)
     */
    void noteOn();

    /**
     * @brief Trigger note off (start release phase)
     */
    void noteOff();

    /**
     * @brief Get next envelope sample
     * @return Envelope value (0.0 to 1.0)
     */
    float getNextSample();

    /**
     * @brief Check if envelope is active
     * @return True if envelope is producing output
     */
    bool isActive() const;

    /**
     * @brief Get current envelope state
     * @return Current ADSR state
     */
    State getState() const { return currentState; }

    /**
     * @brief Reset envelope to idle state
     */
    void reset();

private:
    /**
     * @brief Update internal rate calculations when parameters change
     */
    void updateRates();

    juce::ADSR internalADSR;        ///< Internal JUCE ADSR
    juce::ADSR::Parameters juceParams; ///< JUCE ADSR parameters
    Parameters parameters;          ///< Our parameter structure
    State currentState = State::Idle; ///< Current envelope state
    double sampleRate = 44100.0;    ///< Current sample rate
    bool noteIsOn = false;          ///< Note on/off state
};

/**
 * @brief Audio effects chain for combining multiple effects
 */
class EffectsChain {
public:
    /**
     * @brief Constructor
     */
    EffectsChain();

    /**
     * @brief Destructor
     */
    ~EffectsChain() = default;

    /**
     * @brief Prepare all effects for processing
     * @param sampleRate Sample rate in Hz
     * @param maximumBlockSize Maximum expected block size
     * @param numChannels Number of audio channels
     */
    void prepare(double sampleRate, int maximumBlockSize, int numChannels);

    /**
     * @brief Process audio buffer through the effects chain
     * @param buffer Audio buffer to process
     * @param reverbAmount Reverb effect amount
     * @param bitCrushRate Bit crusher rate
     */
    void processBlock(juce::AudioBuffer<float>& buffer, float reverbAmount, float bitCrushRate);

    /**
     * @brief Get reverb effect reference
     * @return Reference to reverb effect
     */
    ReverbEffect& getReverb() { return reverb; }

    /**
     * @brief Get bit crusher effect reference
     * @return Reference to bit crusher effect
     */
    BitCrusherEffect& getBitCrusher() { return bitCrusher; }

    /**
     * @brief Reset all effects
     */
    void reset();

private:
    ReverbEffect reverb;           ///< Reverb effect
    BitCrusherEffect bitCrusher;   ///< Bit crusher effect
    bool isPrepared = false;       ///< Preparation state flag
};