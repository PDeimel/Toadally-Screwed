#include "PresetManager.hpp"

PresetManager::PresetManager() {
    initializeBuiltInPresets();
}

void PresetManager::initializeBuiltInPresets() {
    createToadPresets();
}

void PresetManager::createToadPresets() {
    // Clear existing Toad preset indices
    toadPresetIndices.clear();

    // Toad-like presets for each oscillator type

    auto toadSine = std::make_unique<PresetData>(
        0.25f, 0, 0.15f, 0.25f, 0.8f, 0.05f, 0.2f, 0.8f, 0.3f,
        "Toad", "Soft and melodic like Toad's higher tones");
    toadPresetIndices.push_back(static_cast<int>(presets.size()));
    presets.push_back(std::move(toadSine));

    auto toadSquare = std::make_unique<PresetData>(
        0.25f, 1, 0.45f, 0.2f, 0.4f, 0.08f, 0.25f, 0.75f, 0.4f,
        "Jerod", "Retro and characteristic like classic Mario sounds");
    toadPresetIndices.push_back(static_cast<int>(presets.size()));
    presets.push_back(std::move(toadSquare));

    auto toadSaw = std::make_unique<PresetData>(
        0.25f, 2, 0.3f, 0.15f, 0.6f, 0.02f, 0.15f, 0.7f, 0.25f,
        "John", "Scratchy and excited like Toad's \"Wahoo!\"");
    toadPresetIndices.push_back(static_cast<int>(presets.size()));
    presets.push_back(std::move(toadSaw));

    auto toadTriangle = std::make_unique<PresetData>(
        0.25f, 3, 0.2f, 0.3f, 0.9f, 0.1f, 0.3f, 0.85f, 0.5f,
        "Dinkelberg", "Soft but distinctive, like Toad's calmer voice");
    toadPresetIndices.push_back(static_cast<int>(presets.size()));
    presets.push_back(std::move(toadTriangle));
}

const PresetData* PresetManager::getPreset(int index) const {
    if (index >= 0 && index < static_cast<int>(presets.size())) {
        return presets[index].get();
    }
    return nullptr;
}

juce::String PresetManager::getPresetName(int index) const {
    const auto* preset = getPreset(index);
    return preset ? preset->name : juce::String();
}

int PresetManager::addPreset(const PresetData& preset) {
    if (presets.size() >= MAX_PRESETS) {
        return -1; // Maximum presets reached
    }

    if (!validatePreset(preset)) {
        return -1; // Invalid preset data
    }

    presets.push_back(std::make_unique<PresetData>(preset));
    return static_cast<int>(presets.size()) - 1;
}

bool PresetManager::removePreset(int index) {
    if (index < 0 || index >= static_cast<int>(presets.size())) {
        return false;
    }

    // Don't allow removal of Toad presets
    if (isToadPreset(index)) {
        return false;
    }

    presets.erase(presets.begin() + index);

    // Update Toad preset indices
    for (auto& toadIndex : toadPresetIndices) {
        if (toadIndex > index) {
            toadIndex--;
        }
    }

    return true;
}

std::vector<int> PresetManager::getToadPresetIndices() const {
    return toadPresetIndices;
}

bool PresetManager::isToadPreset(int index) const {
    return std::find(toadPresetIndices.begin(), toadPresetIndices.end(), index) != toadPresetIndices.end();
}

bool PresetManager::savePresetsToFile(const juce::File& file) const {
    juce::ValueTree presetTree("Presets");

    for (size_t i = 0; i < presets.size(); ++i) {
        const auto& preset = presets[i];
        juce::ValueTree presetNode("Preset");
        
        presetNode.setProperty("name", preset->name, nullptr);
        presetNode.setProperty("description", preset->description, nullptr);
        presetNode.setProperty("gain", preset->gain, nullptr);
        presetNode.setProperty("oscType", preset->oscType, nullptr);
        presetNode.setProperty("vowelMorph", preset->vowelMorph, nullptr);
        presetNode.setProperty("reverbAmount", preset->reverbAmount, nullptr);
        presetNode.setProperty("bitCrusherRate", preset->bitCrusherRate, nullptr);
        presetNode.setProperty("attack", preset->attack, nullptr);
        presetNode.setProperty("decay", preset->decay, nullptr);
        presetNode.setProperty("sustain", preset->sustain, nullptr);
        presetNode.setProperty("release", preset->release, nullptr);
        presetNode.setProperty("isToadPreset", isToadPreset(static_cast<int>(i)), nullptr);

        presetTree.appendChild(presetNode, nullptr);
    }

    std::unique_ptr<juce::XmlElement> xml(presetTree.createXml());
    return xml->writeTo(file);
}

bool PresetManager::loadPresetsFromFile(const juce::File& file) {
    if (!file.existsAsFile()) {
        return false;
    }

    std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(file));
    if (!xml) {
        return false;
    }

    juce::ValueTree presetTree = juce::ValueTree::fromXml(*xml);
    if (!presetTree.isValid() || presetTree.getType() != juce::Identifier("Presets")) {
        return false;
    }

    // Clear existing presets (except Toad presets)
    std::vector<std::unique_ptr<PresetData>> toadPresets;
    for (int i : toadPresetIndices) {
        if (i < static_cast<int>(presets.size())) {
            toadPresets.push_back(std::move(presets[i]));
        }
    }

    presets.clear();
    toadPresetIndices.clear();

    // Re-add Toad presets
    for (auto& toadPreset : toadPresets) {
        toadPresetIndices.push_back(static_cast<int>(presets.size()));
        presets.push_back(std::move(toadPreset));
    }

    // Load presets from file
    for (int i = 0; i < presetTree.getNumChildren(); ++i) {
        juce::ValueTree presetNode = presetTree.getChild(i);
        
        if (presetNode.getType() == juce::Identifier("Preset")) {
            PresetData preset;
            preset.name = presetNode.getProperty("name", "Unnamed");
            preset.description = presetNode.getProperty("description", "");
            preset.gain = presetNode.getProperty("gain", 0.25f);
            preset.oscType = presetNode.getProperty("oscType", 0);
            preset.vowelMorph = presetNode.getProperty("vowelMorph", 0.0f);
            preset.reverbAmount = presetNode.getProperty("reverbAmount", 0.0f);
            preset.bitCrusherRate = presetNode.getProperty("bitCrusherRate", 1.0f);
            preset.attack = presetNode.getProperty("attack", 0.1f);
            preset.decay = presetNode.getProperty("decay", 0.3f);
            preset.sustain = presetNode.getProperty("sustain", 0.7f);
            preset.release = presetNode.getProperty("release", 0.5f);

            bool isToad = presetNode.getProperty("isToadPreset", false);
            
            if (validatePreset(preset)) {
                int index = static_cast<int>(presets.size());
                presets.push_back(std::make_unique<PresetData>(preset));
                
                if (isToad) {
                    toadPresetIndices.push_back(index);
                }
            }
        }
    }

    return true;
}

bool PresetManager::validatePreset(const PresetData& preset) const {
    // Validate parameter ranges
    if (preset.gain < 0.0f || preset.gain > 1.0f) return false;
    if (preset.oscType < 0 || preset.oscType >= static_cast<int>(OscType::NumTypes)) return false;
    if (preset.vowelMorph < 0.0f || preset.vowelMorph > 1.0f) return false;
    if (preset.reverbAmount < 0.0f || preset.reverbAmount > 1.0f) return false;
    if (preset.bitCrusherRate < 0.01f || preset.bitCrusherRate > 1.0f) return false;
    if (preset.attack < 0.0f || preset.attack > 1.0f) return false;
    if (preset.decay < 0.0f || preset.decay > 1.0f) return false;
    if (preset.sustain < 0.0f || preset.sustain > 1.0f) return false;
    if (preset.release < 0.0f || preset.release > 1.0f) return false;

    // Validate name
    if (preset.name.isEmpty()) return false;

    return true;
}