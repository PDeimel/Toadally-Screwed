#include "VowelFilter.hpp"
#include "Utils.hpp"

VowelFilter::VowelFilter() {
    // Constructor implementation if needed
}

float VowelFilter::processSample(float inputSample, OscType oscType, float angle, float morphValue) {
    return getVowelMorphSample(oscType, angle, morphValue);
}

VowelFormants VowelFilter::interpolateVowels(const VowelFormants& vowel1, const VowelFormants& vowel2, float t) {
    VowelFormants result;
    result.f1 = AudioUtils::lerp(vowel1.f1, vowel2.f1, t);
    result.f2 = AudioUtils::lerp(vowel1.f2, vowel2.f2, t);
    result.f3 = AudioUtils::lerp(vowel1.f3, vowel2.f3, t);
    result.a1 = AudioUtils::lerp(vowel1.a1, vowel2.a1, t);
    result.a2 = AudioUtils::lerp(vowel1.a2, vowel2.a2, t);
    result.a3 = AudioUtils::lerp(vowel1.a3, vowel2.a3, t);
    return result;
}

VowelFormants VowelFilter::getVowelFormants(VowelType vowel) {
    switch (vowel) {
        case VowelType::A: return vowelA;
        case VowelType::E: return vowelE;
        case VowelType::I: return vowelI;
        case VowelType::O: return vowelO;
        case VowelType::U: return vowelU;
        default: return vowelA;
    }
}

VowelFormants VowelFilter::getCurrentVowelFormants(float morphValue) {
    // Clamp morph value
    morphValue = juce::jlimit(0.0f, 1.0f, morphValue);

    // Interpolation between vowels based on morphValue (0.0 to 1.0)
    if (morphValue <= 0.25f) {
        // A to E
        float t = morphValue * 4.0f;
        return interpolateVowels(vowelA, vowelE, t);
    }
    else if (morphValue <= 0.5f) {
        // E to I
        float t = (morphValue - 0.25f) * 4.0f;
        return interpolateVowels(vowelE, vowelI, t);
    }
    else if (morphValue <= 0.75f) {
        // I to O
        float t = (morphValue - 0.5f) * 4.0f;
        return interpolateVowels(vowelI, vowelO, t);
    }
    else {
        // O to U
        float t = (morphValue - 0.75f) * 4.0f;
        return interpolateVowels(vowelO, vowelU, t);
    }
}

float VowelFilter::getVowelMorphSample(OscType oscType, float angle, float vowelMorphValue) {
    // Generate base sample based on oscillator type
    float baseSample = OscillatorUtils::getOscSample(oscType, angle);

    // Create temporary vowel filter instance for formant calculation
    VowelFilter tempFilter;
    VowelFormants currentVowel = tempFilter.getCurrentVowelFormants(vowelMorphValue);

    // Formant filter simulation through harmonic component overlay
    // Simplified approach: Modulate the signal with the formants
    float formantSample = 0.0f;

    // First formant (strongest)
    float formant1 = std::sin(angle * currentVowel.f1 / 440.0f) * currentVowel.a1;
    formantSample += formant1 * 0.5f;

    // Second formant
    float formant2 = std::sin(angle * currentVowel.f2 / 440.0f) * currentVowel.a2;
    formantSample += formant2 * 0.3f;

    // Third formant (weakest)
    float formant3 = std::sin(angle * currentVowel.f3 / 440.0f) * currentVowel.a3;
    formantSample += formant3 * 0.2f;

    // Mix the original signal with the formants
    float morphFactor = vowelMorphValue * 0.8f; // Maximum 80% vowel content
    return juce::jmap(morphFactor, baseSample, baseSample * (1.0f + formantSample * 0.5f));
}