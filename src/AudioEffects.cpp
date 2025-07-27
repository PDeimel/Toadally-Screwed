#include "AudioEffects.hpp"

// ReverbEffect Implementation
ReverbEffect::ReverbEffect() {
    // Initialize default parameters
    updateParameters();
}

void ReverbEffect::prepare(double sampleRate, int maximumBlockSize, int numChannels) {
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maximumBlockSize);
    spec.numChannels = static_cast<juce::uint32>(numChannels);

    reverb.prepare(spec);
    isPrepared = true;
}

void ReverbEffect::processBlock(juce::AudioBuffer<float>& buffer) {
    if (!isPrepared || currentAmount <= 0.0f) {
        return; // Skip processing if not prepared or amount is zero
    }

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);
}

void ReverbEffect::setAmount(float amount) {
    currentAmount = juce::jlimit(0.0f, 1.0f, amount);
    updateParameters();
}

void ReverbEffect::reset() {
    if (isPrepared) {
        reverb.reset();
    }
}

void ReverbEffect::updateParameters() {
    juce::dsp::Reverb::Parameters params;

    // Map reverb amount (0.0 to 1.0) to reverb parameters
    params.roomSize = juce::jmap(currentAmount, 0.0f, 0.8f);        // Room size
    params.damping = juce::jmap(currentAmount, 0.2f, 0.6f);         // Damping
    params.wetLevel = juce::jmap(currentAmount, 0.0f, 0.4f);        // Wet signal
    params.dryLevel = 1.0f - (currentAmount * 0.3f);               // Dry signal stays dominant
    params.width = 1.0f;                                            // Stereo width
    params.freezeMode = 0.0f;                                       // No freeze

    reverb.setParameters(params);
}

// BitCrusherEffect Implementation
void BitCrusherEffect::processBlock(juce::AudioBuffer<float>& buffer, float crushRate) {
    if (crushRate >= 1.0f) {
        return; // No effect when rate is 1.0
    }

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    const float crushFactor = juce::jlimit(0.01f, 1.0f, crushRate);
    const float inverse = 1.0f / crushFactor;

    for (int channel = 0; channel < numChannels; ++channel) {
        float* samples = buffer.getWritePointer(channel);
        for (int i = 0; i < numSamples; ++i) {
            samples[i] = processSample(samples[i], crushFactor);
        }
    }
}

float BitCrusherEffect::processSample(float sample, float crushRate) {
    if (crushRate >= 1.0f) {
        return sample;
    }

    const float crushFactor = juce::jlimit(0.01f, 1.0f, crushRate);
    const float inverse = 1.0f / crushFactor;
    return std::round(sample * inverse) * crushFactor;
}

// ADSREnvelope Implementation
ADSREnvelope::ADSREnvelope() {
    updateRates();
}

void ADSREnvelope::setSampleRate(double newSampleRate) {
    sampleRate = newSampleRate;
    internalADSR.setSampleRate(newSampleRate);
    updateRates();
}

void ADSREnvelope::setParameters(const Parameters& newParams) {
    parameters = newParams;
    updateRates();
}

void ADSREnvelope::noteOn() {
    noteIsOn = true;
    currentState = State::Attack;
    internalADSR.noteOn();
}

void ADSREnvelope::noteOff() {
    noteIsOn = false;
    currentState = State::Release;
    internalADSR.noteOff();
}

float ADSREnvelope::getNextSample() {
    float sample = internalADSR.getNextSample();

    // Update state based on envelope behavior
    if (!isActive()) {
        currentState = State::Idle;
    } else if (noteIsOn) {
        // Determine if we're in attack, decay, or sustain
        static float lastSample = 0.0f;
        float delta = sample - lastSample;
        
        if (sample < 0.99f && delta > 0.001f) {
            currentState = State::Attack;
        } else if (delta < -0.001f && sample > parameters.sustain + 0.1f) {
            currentState = State::Decay;
        } else if (std::abs(delta) < 0.001f) {
            currentState = State::Sustain;
        }
        
        lastSample = sample;
    }

    return sample;
}

bool ADSREnvelope::isActive() const {
    return internalADSR.isActive();
}

void ADSREnvelope::reset() {
    internalADSR.reset();
    currentState = State::Idle;
    noteIsOn = false;
}

void ADSREnvelope::updateRates() {
    // Map our parameter range (0.0-1.0) to JUCE ADSR time ranges
    juceParams.attack = juce::jmap(parameters.attack, 0.0f, 1.0f, 0.01f, 3.0f);     // 0.01s to 3s
    juceParams.decay = juce::jmap(parameters.decay, 0.0f, 1.0f, 0.01f, 3.0f);       // 0.01s to 3s
    juceParams.sustain = parameters.sustain;                                         // 0.0 to 1.0 (direct)
    juceParams.release = juce::jmap(parameters.release, 0.0f, 1.0f, 0.01f, 5.0f);   // 0.01s to 5s

    internalADSR.setParameters(juceParams);
}

// EffectsChain Implementation
EffectsChain::EffectsChain() {
    // Constructor - effects are initialized with their defaults
}

void EffectsChain::prepare(double sampleRate, int maximumBlockSize, int numChannels) {
    reverb.prepare(sampleRate, maximumBlockSize, numChannels);
    // BitCrusher doesn't need preparation
    isPrepared = true;
}

void EffectsChain::processBlock(juce::AudioBuffer<float>& buffer, float reverbAmount, float bitCrushRate) {
    if (!isPrepared) {
        return;
    }

    // Process reverb first
    reverb.setAmount(reverbAmount);
    reverb.processBlock(buffer);

    // Then process bit crusher
    bitCrusher.processBlock(buffer, bitCrushRate);
}

void EffectsChain::reset() {
    reverb.reset();
    // BitCrusher doesn't have state to reset
}