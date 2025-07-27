#pragma once
#include "JuceHeader.h"
#include <cmath>

/**
 * @file Oscillator.hpp
 * @brief Oscillator implementations for the AvSynth audio plugin
 */

/**
 * @brief Enumeration of available oscillator types
 */
enum class OscType {
    Sine,      ///< Sine wave oscillator
    Square,    ///< Square wave oscillator
    Saw,       ///< Sawtooth wave oscillator
    Triangle,  ///< Triangle wave oscillator
    NumTypes   ///< Total number of oscillator types
};

/**
 * @brief Base oscillator class providing common functionality
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
    }

    /**
     * @brief Set the frequency
     * @param frequency Frequency in Hz
     */
    virtual void setFrequency(float frequency) {
        this->frequency = frequency;
        updateAngleDelta();
    }

    /**
     * @brief Get the next sample from the oscillator
     * @return Audio sample value
     */
    virtual float getNextSample() = 0;

    /**
     * @brief Reset the oscillator phase
     */
    virtual void reset() {
        currentAngle = 0.0;
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

    double currentAngle = 0.0;  ///< Current phase angle
    double angleDelta = 0.0;    ///< Phase increment per sample
    double sampleRate = 44100.0; ///< Sample rate in Hz
    float frequency = 440.0f;   ///< Oscillator frequency in Hz
};

/**
 * @brief Sine wave oscillator
 */
class SineOscillator : public BaseOscillator {
public:
    /**
     * @brief Get the next sine wave sample
     * @return Sine wave sample
     */
    float getNextSample() override {
        float sample = static_cast<float>(std::sin(currentAngle));
        currentAngle += angleDelta;
        return sample;
    }
};

/**
 * @brief Square wave oscillator
 */
class SquareOscillator : public BaseOscillator {
public:
    /**
     * @brief Get the next square wave sample
     * @return Square wave sample
     */
    float getNextSample() override {
        float sample = (std::sin(currentAngle) >= 0.0 ? 1.0f : -1.0f);
        currentAngle += angleDelta;
        return sample;
    }
};

/**
 * @brief Sawtooth wave oscillator
 */
class SawOscillator : public BaseOscillator {
public:
    /**
     * @brief Get the next sawtooth wave sample
     * @return Sawtooth wave sample
     */
    float getNextSample() override {
        float sample = static_cast<float>(
            2.0 * (currentAngle / juce::MathConstants<double>::twoPi -
                   std::floor(0.5 + currentAngle / juce::MathConstants<double>::twoPi)));
        currentAngle += angleDelta;
        return sample;
    }
};

/**
 * @brief Triangle wave oscillator
 */
class TriangleOscillator : public BaseOscillator {
public:
    /**
     * @brief Get the next triangle wave sample
     * @return Triangle wave sample
     */
    float getNextSample() override {
        float sample = 2.0f * static_cast<float>(
            std::abs(2.0 * (currentAngle / juce::MathConstants<double>::twoPi -
                           std::floor(0.5 + currentAngle / juce::MathConstants<double>::twoPi)))) - 1.0f;
        currentAngle += angleDelta;
        return sample;
    }
};

/**
 * @brief Static utility functions for oscillator operations
 */
class OscillatorUtils {
public:
    /**
     * @brief Generate a single oscillator sample based on type and angle
     * @param type Oscillator type
     * @param angle Current phase angle
     * @return Generated sample
     */
    static float getOscSample(OscType type, double angle) {
        switch (type) {
        case OscType::Sine:
            return static_cast<float>(std::sin(angle));
        case OscType::Square:
            return (std::sin(angle) >= 0.0 ? 1.0f : -1.0f);
        case OscType::Saw:
            return static_cast<float>(
                2.0 * (angle / juce::MathConstants<double>::twoPi -
                       std::floor(0.5 + angle / juce::MathConstants<double>::twoPi)));
        case OscType::Triangle:
            return 2.0f * static_cast<float>(
                std::abs(2.0 * (angle / juce::MathConstants<double>::twoPi -
                               std::floor(0.5 + angle / juce::MathConstants<double>::twoPi)))) - 1.0f;
        default:
            return 0.0f;
        }
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