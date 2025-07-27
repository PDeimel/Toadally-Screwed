#pragma once
#include <type_traits>
#include <atomic>
#include "JuceHeader.h"

/**
 * @file Utils.hpp
 * @brief Utility classes and functions for the AvSynth audio plugin
 */

/**
 * @brief A simple linear ramp generator for smooth transitions between two values without any branching
 * @tparam T Type that supports arithmetic operations
 */
template <typename T>
    requires std::is_arithmetic_v<T> || requires(T a, T b, float f) {
        { a + b };
        { a - b };
        { a *f };
        { a / f };
    }
class LinearRamp {
public:
    /**
     * @brief Reset the ramp with new start and end values
     * @param start Starting value of the ramp
     * @param end Target value of the ramp
     * @param steps Number of steps to reach the target
     */
    void reset(T start, T end, int steps) {
        current = start;
        increment = (steps > 0) ? (end - start) / static_cast<std::common_type_t<T, float>>(steps) : T{};
        remainingSteps = steps;
    }

    /**
     * @brief Get the next value in the ramp sequence
     * @return Current value and advance to next step
     */
    T getNext() {
        T value = current;

        // Active flag: 0 or 1
        int active = (remainingSteps > 0);
        auto fActive = static_cast<typename std::common_type<T, float>::type>(active);

        // Only accumulate if active
        current += increment * fActive;
        remainingSteps -= active;

        return value;
    }

private:
    T current{};           ///< Current value in the ramp
    T increment{};         ///< Increment per step
    int remainingSteps = 0; ///< Remaining steps in the ramp
};

/**
 * @brief Thread-safe circular buffer for audio data visualization
 */
class CircularAudioBuffer {
public:
    /**
     * @brief Constructor
     * @param channels Number of audio channels
     * @param size Buffer size in samples
     */
    CircularAudioBuffer(int channels = 1, int size = 1024)
        : buffer(channels, size), writePosition(0) {}

    /**
     * @brief Write a sample to the buffer
     * @param channel Channel index
     * @param sample Audio sample value
     */
    void writeSample(int channel, float sample) {
        if (channel < buffer.getNumChannels()) {
            buffer.setSample(channel, writePosition.load(), sample);
        }
    }

    /**
     * @brief Advance the write position
     */
    void advanceWritePosition() {
        writePosition = (writePosition.load() + 1) % buffer.getNumSamples();
    }

    /**
     * @brief Get read access to the buffer
     * @return Reference to the internal audio buffer
     */
    juce::AudioSampleBuffer &getBuffer() { return buffer; }

    const juce::AudioSampleBuffer &getBuffer() const { return buffer; }

    /**
     * @brief Get current write position
     * @return Current write position index
     */
    int getWritePosition() const { return writePosition.load(); }

    /**
     * @brief Resize the buffer
     * @param channels Number of channels
     * @param size Buffer size in samples
     */
    void setSize(int channels, int size) {
        buffer.setSize(channels, size);
        writePosition = 0;
    }

private:
    juce::AudioBuffer<float> buffer;     ///< Internal audio buffer
    std::atomic<int> writePosition;      ///< Thread-safe write position
};

/**
 * @brief Utility functions for audio processing
 */
namespace AudioUtils {
    /**
     * @brief Convert MIDI note number to frequency in Hz
     * @param noteNumber MIDI note number (0-127)
     * @return Frequency in Hz
     */
    inline float midiNoteToFrequency(int noteNumber) {
        return 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);
    }

    /**
     * @brief Apply soft clipping to prevent harsh distortion
     * @param sample Input sample
     * @param threshold Clipping threshold (0.0 to 1.0)
     * @return Clipped sample
     */
    inline float softClip(float sample, float threshold = 0.7f) {
        float abs_sample = std::abs(sample);
        if (abs_sample <= threshold) {
            return sample;
        }
        float sign = (sample > 0.0f) ? 1.0f : -1.0f;
        return sign * (threshold + (1.0f - threshold) * std::tanh((abs_sample - threshold) / (1.0f - threshold)));
    }

    /**
     * @brief Linear interpolation between two values
     * @param a First value
     * @param b Second value
     * @param t Interpolation factor (0.0 to 1.0)
     * @return Interpolated value
     */
    template<typename T>
    inline T lerp(T a, T b, float t) {
        return a + t * (b - a);
    }
}