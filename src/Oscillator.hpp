#pragma once
#include "JuceHeader.h"
#include <cmath>

/**
 * @file Oscillator.hpp
 * @brief Toad-voice enhanced oscillator implementations for the AvSynth audio plugin
 */

/**
 * @brief Enumeration of available oscillator types
 */
enum class OscType {
    Sine,      ///< Sine wave oscillator with Toad-like nasality
    Square,    ///< Square wave oscillator with characteristic roughness
    Saw,       ///< Sawtooth wave oscillator with harmonic distortion
    Triangle,  ///< Triangle wave oscillator with subtle formant emphasis
    NumTypes   ///< Total number of oscillator types
};

/**
 * @brief Base oscillator class providing common functionality with Toad voice characteristics
 */
class BaseOscillator {
public:
    /**
     * @brief Constructor
     */
    BaseOscillator() = default;

    /**
     * @brief Virtual destructor
     */
    virtual ~BaseOscillator() = default;

    /**
     * @brief Set the sample rate
     * @param sampleRate Sample rate in Hz
     */
    virtual void setSampleRate(double sampleRate) {
        this->sampleRate = sampleRate;
        updateAngleDelta();
        updateToadParameters();
    }

    /**
     * @brief Set the frequency
     * @param frequency Frequency in Hz
     */
    virtual void setFrequency(float frequency) {
        this->frequency = frequency;
        updateAngleDelta();
        updateToadParameters();
    }

    /**
     * @brief Get the next sample from the oscillator
     * @return Audio sample value with Toad voice characteristics
     */
    virtual float getNextSample() = 0;

    /**
     * @brief Reset the oscillator phase
     */
    virtual void reset() {
        currentAngle = 0.0;
        toadPhase = 0.0;
        nasalPhase = 0.0;
    }

protected:
    /**
     * @brief Update the angle delta based on current frequency and sample rate
     */
    void updateAngleDelta() {
        if (sampleRate > 0.0) {
            auto cyclesPerSample = frequency / sampleRate;
            angleDelta = cyclesPerSample * juce::MathConstants<double>::twoPi;
        } else {
            angleDelta = 0.0;
        }
    }

    /**
     * @brief Update Toad-specific voice parameters based on frequency
     */
    void updateToadParameters() {
        if (sampleRate > 0.0) {
            // Toad's characteristic nasal resonance around 800-1200 Hz
            toadNasalFreq = frequency * 2.7f;  // Creates nasal formant
            toadNasalDelta = (toadNasalFreq / sampleRate) * juce::MathConstants<double>::twoPi;

            // Higher frequency modulation for squeaky character
            toadVibratoFreq = 4.5f + (frequency / 440.0f) * 2.0f;  // Variable vibrato
            toadVibratoDelta = (toadVibratoFreq / sampleRate) * juce::MathConstants<double>::twoPi;
        }
    }

    /**
     * @brief Apply Toad voice characteristics to a sample
     * @param sample Input sample
     * @return Sample with Toad voice processing applied
     */
    float applyToadCharacteristics(float sample) {
        // Add nasal resonance (characteristic of Toad's voice)
        float nasalComponent = std::sin(nasalPhase) * 0.15f;
        nasalPhase += toadNasalDelta;

        // Add slight vibrato for squeaky character
        float vibratoMod = 1.0f + std::sin(toadPhase) * 0.08f;
        toadPhase += toadVibratoDelta;

        // Apply formant-like filtering for vocal tract simulation
        float formantSample = sample * vibratoMod;
        formantSample += nasalComponent * sample;

        // Add subtle harmonic distortion for roughness
        float distortion = formantSample * 0.3f;
        if (distortion > 0.1f) {
            distortion = 0.1f + (distortion - 0.1f) * 0.7f;  // Soft clipping
        } else if (distortion < -0.1f) {
            distortion = -0.1f + (distortion + 0.1f) * 0.7f;
        }

        formantSample += distortion;

        // Apply frequency-dependent character enhancement
        float frequencyFactor = juce::jlimit(0.5f, 2.0f, frequency / 440.0f);
        float toadIntensity = 0.8f + frequencyFactor * 0.3f;

        return sample * 0.7f + formantSample * 0.3f * toadIntensity;
    }

    double currentAngle = 0.0;      ///< Current phase angle
    double angleDelta = 0.0;        ///< Phase increment per sample
    double sampleRate = 44100.0;    ///< Sample rate in Hz
    float frequency = 440.0f;       ///< Oscillator frequency in Hz

    // Toad voice specific parameters
    double toadPhase = 0.0;         ///< Phase for Toad's vibrato modulation
    double nasalPhase = 0.0;        ///< Phase for nasal resonance
    double toadVibratoDelta = 0.0;  ///< Phase increment for vibrato
    double toadNasalDelta = 0.0;    ///< Phase increment for nasal component
    float toadNasalFreq = 1080.0f;  ///< Nasal formant frequency
    float toadVibratoFreq = 4.5f;   ///< Vibrato frequency
};

/**
 * @brief Toad-voice enhanced sine wave oscillator
 */
class SineOscillator : public BaseOscillator {
public:
    /**
     * @brief Get the next Toad-like sine wave sample
     * @return Sine wave sample with Toad voice characteristics
     */
    float getNextSample() override {
        float baseSample = static_cast<float>(std::sin(currentAngle));
        currentAngle += angleDelta;

        // Add subtle harmonic content for more vocal-like quality
        float harmonic2 = std::sin(currentAngle * 2.0) * 0.1f;
        float harmonic3 = std::sin(currentAngle * 3.0) * 0.05f;

        float sample = baseSample + harmonic2 + harmonic3;
        return applyToadCharacteristics(sample);
    }
};

/**
 * @brief Toad-voice enhanced square wave oscillator
 */
class SquareOscillator : public BaseOscillator {
public:
    /**
     * @brief Get the next Toad-like square wave sample
     * @return Square wave sample with enhanced roughness and nasal quality
     */
    float getNextSample() override {
        float baseSquare = (std::sin(currentAngle) >= 0.0 ? 1.0f : -1.0f);
        currentAngle += angleDelta;

        // Soften the edges slightly for more natural vocal quality
        float softening = std::sin(currentAngle * 5.0) * 0.08f;
        float sample = baseSquare * (0.95f + softening);

        // Add extra roughness characteristic of Toad's voice
        float roughness = std::sin(currentAngle * 7.3) * 0.12f;
        sample += roughness;

        return applyToadCharacteristics(sample);
    }
};

/**
 * @brief Toad-voice enhanced sawtooth wave oscillator
 */
class SawOscillator : public BaseOscillator {
public:
    /**
     * @brief Get the next Toad-like sawtooth wave sample
     * @return Sawtooth wave sample with harmonic enhancement
     */
    float getNextSample() override {
        float baseSaw = static_cast<float>(
            2.0 * (currentAngle / juce::MathConstants<double>::twoPi -
                   std::floor(0.5 + currentAngle / juce::MathConstants<double>::twoPi)));
        currentAngle += angleDelta;

        // Emphasize certain harmonics for more vocal-like timbre
        float harmonic4 = std::sin(currentAngle * 4.0) * 0.08f;
        float harmonic6 = std::sin(currentAngle * 6.0) * 0.04f;

        float sample = baseSaw + harmonic4 + harmonic6;
        return applyToadCharacteristics(sample);
    }
};

/**
 * @brief Toad-voice enhanced triangle wave oscillator
 */
class TriangleOscillator : public BaseOscillator {
public:
    /**
     * @brief Get the next Toad-like triangle wave sample
     * @return Triangle wave sample with subtle formant emphasis
     */
    float getNextSample() override {
        float baseTriangle = 2.0f * static_cast<float>(
            std::abs(2.0 * (currentAngle / juce::MathConstants<double>::twoPi -
                           std::floor(0.5 + currentAngle / juce::MathConstants<double>::twoPi)))) - 1.0f;
        currentAngle += angleDelta;

        // Add subtle formant-like resonances
        float formant1 = std::sin(currentAngle * 2.3) * 0.06f;  // Lower formant
        float formant2 = std::sin(currentAngle * 3.7) * 0.04f;  // Higher formant

        float sample = baseTriangle + formant1 + formant2;
        return applyToadCharacteristics(sample);
    }
};

/**
 * @brief Static utility functions for Toad-enhanced oscillator operations
 */
class OscillatorUtils {
public:
    /**
     * @brief Generate a single Toad-enhanced oscillator sample based on type and angle
     * @param type Oscillator type
     * @param angle Current phase angle
     * @param frequency Current frequency for Toad characteristics
     * @param sampleRate Sample rate for proper scaling
     * @return Generated sample with Toad voice characteristics
     */
    static float getOscSample(OscType type, double angle, float frequency = 440.0f, double sampleRate = 44100.0) {
        float baseSample = 0.0f;

        switch (type) {
        case OscType::Sine:
            baseSample = static_cast<float>(std::sin(angle));
            // Add harmonics for vocal quality
            baseSample += std::sin(angle * 2.0) * 0.1f + std::sin(angle * 3.0) * 0.05f;
            break;
        case OscType::Square:
            baseSample = (std::sin(angle) >= 0.0 ? 1.0f : -1.0f);
            // Soften and add roughness
            baseSample *= (0.95f + std::sin(angle * 5.0) * 0.08f);
            baseSample += std::sin(angle * 7.3) * 0.12f;
            break;
        case OscType::Saw:
            baseSample = static_cast<float>(
                2.0 * (angle / juce::MathConstants<double>::twoPi -
                       std::floor(0.5 + angle / juce::MathConstants<double>::twoPi)));
            // Add harmonic emphasis
            baseSample += std::sin(angle * 4.0) * 0.08f + std::sin(angle * 6.0) * 0.04f;
            break;
        case OscType::Triangle:
            baseSample = 2.0f * static_cast<float>(
                std::abs(2.0 * (angle / juce::MathConstants<double>::twoPi -
                               std::floor(0.5 + angle / juce::MathConstants<double>::twoPi)))) - 1.0f;
            // Add formant-like resonances
            baseSample += std::sin(angle * 2.3) * 0.06f + std::sin(angle * 3.7) * 0.04f;
            break;
        default:
            return 0.0f;
        }

        // Apply simplified Toad characteristics
        float nasalFreq = frequency * 2.7f;
        double nasalAngle = (nasalFreq / sampleRate) * angle;
        float nasalComponent = std::sin(nasalAngle) * 0.15f;

        float vibratoMod = 1.0f + std::sin(angle * 0.01f * frequency / 440.0f) * 0.08f;

        float toadSample = baseSample * vibratoMod + nasalComponent * baseSample;

        // Subtle harmonic distortion
        float distortion = toadSample * 0.3f;
        if (distortion > 0.1f) {
            distortion = 0.1f + (distortion - 0.1f) * 0.7f;
        } else if (distortion < -0.1f) {
            distortion = -0.1f + (distortion + 0.1f) * 0.7f;
        }

        return baseSample * 0.7f + (toadSample + distortion) * 0.3f;
    }

    /**
     * @brief Calculate angle delta for given frequency and sample rate
     * @param frequency Frequency in Hz
     * @param sampleRate Sample rate in Hz
     * @return Angle delta per sample
     */
    static double calculateAngleDelta(float frequency, double sampleRate) {
        if (sampleRate <= 0.0) return 0.0;
        auto cyclesPerSample = frequency / sampleRate;
        return cyclesPerSample * juce::MathConstants<double>::twoPi;
    }
};