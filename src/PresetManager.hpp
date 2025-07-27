#pragma once
#include "JuceHeader.h"
#include "Oscillator.hpp"
#include <vector>
#include <memory>

/**
 * @file PresetManager.hpp
 * @brief Preset management system for the AvSynth audio plugin
 */

/**
 * @brief Structure representing a complete preset configuration
 */
struct PresetData {
    float gain;              ///< Gain level (0.0 to 1.0)
    int oscType;            ///< Oscillator type (enum index)
    float vowelMorph;       ///< Vowel morphing value (0.0 to 1.0)
    float reverbAmount;     ///< Reverb amount (0.0 to 1.0)
    float bitCrusherRate;   ///< Bit crusher rate (0.01 to 1.0)
    float attack;           ///< ADSR attack time (0.0 to 1.0)
    float decay;            ///< ADSR decay time (0.0 to 1.0)
    float sustain;          ///< ADSR sustain level (0.0 to 1.0)
    float release;          ///< ADSR release time (0.0 to 1.0)
    juce::String name;      ///< Preset name
    juce::String description; ///< Preset description

    /**
     * @brief Constructor with default values
     * @param presetName Name of the preset
     * @param presetDescription Description of the preset
     */
    PresetData(const juce::String& presetName = "Default", 
               const juce::String& presetDescription = "Default preset")
        : gain(0.25f), oscType(0), vowelMorph(0.0f), reverbAmount(0.0f), 
          bitCrusherRate(1.0f), attack(0.1f), decay(0.3f), sustain(0.7f), 
          release(0.5f), name(presetName), description(presetDescription) {}

    /**
     * @brief Constructor with all parameters
     */
    PresetData(float g, int osc, float vowel, float reverb, float bitCrush,
               float att, float dec, float sust, float rel,
               const juce::String& presetName, const juce::String& presetDescription)
        : gain(g), oscType(osc), vowelMorph(vowel), reverbAmount(reverb),
          bitCrusherRate(bitCrush), attack(att), decay(dec), sustain(sust),
          release(rel), name(presetName), description(presetDescription) {}
};

/**
 * @brief Preset management class for storing and loading presets
 */
class PresetManager {
public:
    /**
     * @brief Constructor
     */
    PresetManager();

    /**
     * @brief Destructor
     */
    ~PresetManager() = default;

    /**
     * @brief Initialize the preset manager with built-in presets
     */
    void initializeBuiltInPresets();

    /**
     * @brief Get the number of available presets
     * @return Number of presets
     */
    int getNumPresets() const { return static_cast<int>(presets.size()); }

    /**
     * @brief Get a preset by index
     * @param index Preset index (0-based)
     * @return Preset data, or nullptr if index is invalid
     */
    const PresetData* getPreset(int index) const;

    /**
     * @brief Get preset name by index
     * @param index Preset index
     * @return Preset name or empty string if invalid
     */
    juce::String getPresetName(int index) const;

    /**
     * @brief Add a new preset
     * @param preset Preset data to add
     * @return Index of the added preset
     */
    int addPreset(const PresetData& preset);

    /**
     * @brief Remove a preset by index
     * @param index Preset index to remove
     * @return True if successful, false if index invalid
     */
    bool removePreset(int index);

    /**
     * @brief Get all Toad presets (built-in character presets)
     * @return Vector of Toad preset indices
     */
    std::vector<int> getToadPresetIndices() const;

    /**
     * @brief Check if a preset is a Toad preset
     * @param index Preset index
     * @return True if it's a Toad preset
     */
    bool isToadPreset(int index) const;

    /**
     * @brief Save presets to file
     * @param file Target file
     * @return True if successful
     */
    bool savePresetsToFile(const juce::File& file) const;

    /**
     * @brief Load presets from file
     * @param file Source file
     * @return True if successful
     */
    bool loadPresetsFromFile(const juce::File& file);

    /**
     * @brief Clear all presets
     */
    void clearPresets() { presets.clear(); }

private:
    /**
     * @brief Create built-in Toad presets
     */
    void createToadPresets();

    /**
     * @brief Validate preset data
     * @param preset Preset to validate
     * @return True if valid
     */
    bool validatePreset(const PresetData& preset) const;

    std::vector<std::unique_ptr<PresetData>> presets; ///< Collection of presets
    std::vector<int> toadPresetIndices;               ///< Indices of Toad presets

    static constexpr int MAX_PRESETS = 256;           ///< Maximum number of presets
    static constexpr int NUM_TOAD_PRESETS = 4;       ///< Number of built-in Toad presets
};