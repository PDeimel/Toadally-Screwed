#pragma once
#include "JuceHeader.h"
#include "Oscillator.hpp"
#include <array>

/**
 * @file VowelFilter.hpp
 * @brief Vowel morphing filter implementation for formant synthesis
 */

/**
 * @brief Structure representing vowel formant characteristics
 */
struct VowelFormants {
    float f1, f2, f3; ///< First three formant frequencies in Hz
    float a1, a2, a3; ///< Amplitudes for the formants (0.0 to 1.0)

    /**
     * @brief Constructor with default values
     * @param formant1 First formant frequency
     * @param formant2 Second formant frequency
     * @param formant3 Third formant frequency
     * @param amp1 First formant amplitude
     * @param amp2 Second formant amplitude
     * @param amp3 Third formant amplitude
     */
    constexpr VowelFormants(float formant1 = 500.0f, float formant2 = 1500.0f, float formant3 = 2500.0f,
                  float amp1 = 1.0f, float amp2 = 0.7f, float amp3 = 0.3f)
        : f1(formant1), f2(formant2), f3(formant3), a1(amp1), a2(amp2), a3(amp3) {}
};

/**
 * @brief Enumeration of available vowel types
 */
enum class VowelType {
    A,  ///< Vowel "A"
    E,  ///< Vowel "E"
    I,  ///< Vowel "I"
    O,  ///< Vowel "O"
    U   ///< Vowel "U"
};

/**
 * @brief Vowel morphing filter for formant synthesis
 *
 * This class implements vowel formant filtering by modulating the input signal
 * with characteristic formant frequencies of different vowels.
 */
class VowelFilter {
public:
    /**
     * @brief Constructor
     */
    VowelFilter();

    /**
     * @brief Destructor
     */
    ~VowelFilter() = default;

    /**
     * @brief Process a sample with vowel morphing
     * @param inputSample Input audio sample
     * @param oscType Oscillator type for base waveform
     * @param angle Current oscillator phase angle
     * @param morphValue Morphing value (0.0 = A, 1.0 = U)
     * @return Processed sample with vowel characteristics
     */
    float processSample(float inputSample, OscType oscType, float angle, float morphValue);

    /**
     * @brief Get vowel morphed sample (static utility function)
     * @param oscType Oscillator type
     * @param angle Current phase angle
     * @param vowelMorphValue Morphing value (0.0 to 1.0)
     * @return Processed sample
     */
    static float getVowelMorphSample(OscType oscType, float angle, float vowelMorphValue);

    /**
     * @brief Set the intensity of the vowel effect
     * @param intensity Effect intensity (0.0 to 1.0)
     */
    void setIntensity(float intensity) { this->intensity = juce::jlimit(0.0f, 1.0f, intensity); }

    /**
     * @brief Get current effect intensity
     * @return Current intensity value
     */
    float getIntensity() const { return intensity; }

private:
    /**
     * @brief Interpolate between two vowel formants
     * @param vowel1 First vowel formants
     * @param vowel2 Second vowel formants
     * @param t Interpolation factor (0.0 to 1.0)
     * @return Interpolated vowel formants
     */
    VowelFormants interpolateVowels(const VowelFormants& vowel1, const VowelFormants& vowel2, float t);

    /**
     * @brief Get vowel formants for specific vowel type
     * @param vowel Vowel type
     * @return Vowel formants structure
     */
    VowelFormants getVowelFormants(VowelType vowel);

    /**
     * @brief Calculate current vowel formants based on morph value
     * @param morphValue Morphing value (0.0 to 1.0)
     * @return Current vowel formants
     */
    VowelFormants getCurrentVowelFormants(float morphValue);

    // Predefined vowel formants (approximated for synthesizer use)
    static constexpr VowelFormants vowelA{800.0f, 1200.0f, 2500.0f, 1.0f, 0.7f, 0.3f};  ///< Vowel "A" formants
    static constexpr VowelFormants vowelE{500.0f, 1800.0f, 2500.0f, 1.0f, 0.8f, 0.2f};  ///< Vowel "E" formants
    static constexpr VowelFormants vowelI{300.0f, 2300.0f, 3000.0f, 1.0f, 0.9f, 0.4f};  ///< Vowel "I" formants
    static constexpr VowelFormants vowelO{500.0f, 900.0f, 2200.0f, 1.0f, 0.6f, 0.2f};   ///< Vowel "O" formants
    static constexpr VowelFormants vowelU{300.0f, 700.0f, 2100.0f, 1.0f, 0.5f, 0.1f};   ///< Vowel "U" formants

    float intensity = 0.8f; ///< Effect intensity (0.0 to 1.0)
};