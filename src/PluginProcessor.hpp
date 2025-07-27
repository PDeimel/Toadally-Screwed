#pragma once

#include "JuceHeader.h"
#include "juce_dsp/juce_dsp.h"

//==============================================================================
class AvSynthAudioProcessor final : public juce::AudioProcessor {
    friend class AvSynthAudioProcessorEditor;

  public:
    enum class Parameters { Gain, Frequency, OscType, VowelMorph, ReverbAmount, BitCrusherRate, Attack, Decay, Sustain, Release, NumParameters};
    // KORRIGIERT: Die Namen entsprechen jetzt den tatsächlichen Wellenformen
    enum class OscType { Sine, Square, Saw, Triangle, NumTypes };

    struct ChainSettings {
        float gain = 0.25f;
        float frequency = 440.0f;
        OscType oscType = OscType::Sine;
        float VowelMorph = 0.0f; // 0.0 = "A", 1.0 = "U"
        float reverbAmount = 0.0f;
        float bitCrusherRate = 0.01f;
        float attack = 0.1f;
        float decay = 0.3f;
        float sustain = 0.7f;
        float release = 0.5f;

        static forcedinline ChainSettings Get(const juce::AudioProcessorValueTreeState &parameters);
    };

  public:
    //==============================================================================
    AvSynthAudioProcessor();
    ~AvSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    void updateAngleDelta(float frequency);

    static float getOscSample(OscType type, double angle);
    static float getVowelMorphSample(OscType oscType, float angle, float vowelMorphValue);

    void updateReverbParameters(float reverbAmount);
    void updateADSRParameters(float attack, float decay, float sustain, float release);

    float getCurrentEnvelopeValue() const { return currentEnvelopeValue.load(); }
    bool isEnvelopeActive() const { return adsr.isActive(); }
    int getADSRState() const { return currentADSRState.load(); }

  private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

  public:
    juce::AudioProcessorValueTreeState parameters{*this, nullptr, "Parameters", createParameterLayout()};
    juce::MidiKeyboardState keyboardState;

  private:
    juce::Random random;
    ChainSettings previousChainSettings;
    juce::AudioSampleBuffer circularBuffer;
    int bufferWritePos = 0;

    bool noteIsActive = false;
    float currentNoteFrequency = 0.0f;

  private:
    // Reverb-Komponenten
    juce::dsp::Reverb reverb;
    juce::dsp::ProcessSpec reverbSpec;

    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;

    double currentAngle = 0.0, angleDelta = 0.0;

    std::atomic<float> currentEnvelopeValue{0.0f};
    std::atomic<int> currentADSRState{0}; // 0=idle, 1=attack, 2=decay, 3=sustain, 4=release

  private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AvSynthAudioProcessor)
};