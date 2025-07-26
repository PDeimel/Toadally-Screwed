// 1. Erweiterte ADSRComponent.hpp
#pragma once

#include "JuceHeader.h"
#include <array>
#include <atomic>

class ADSRComponent : public juce::Component, private juce::Timer
{
public:
    ADSRComponent();
    ~ADSRComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;

    // ADSR-Parameter setzen/abrufen (0.0 bis 1.0)
    void setAttack(float attack);
    void setDecay(float decay);
    void setSustain(float sustain);
    void setRelease(float release);
    
    float getAttack() const { return attackValue; }
    float getDecay() const { return decayValue; }
    float getSustain() const { return sustainValue; }
    float getRelease() const { return releaseValue; }
    
    // Callback für Parameteränderungen
    std::function<void(float attack, float decay, float sustain, float release)> onParameterChanged;
    
    // Farbschema updaten
    void updateColors(juce::Colour primary, juce::Colour secondary);
    
    // Echtzeit-Plotting
    void updateEnvelopeValue(float currentValue, bool isActive);
    void setADSRState(int state); // 0=idle, 1=attack, 2=decay, 3=sustain, 4=release

private:
    // Timer callback für Animation
    void timerCallback() override;
    
    // ADSR-Werte (0.0 bis 1.0)
    float attackValue = 0.1f;
    float decayValue = 0.3f;
    float sustainValue = 0.7f;
    float releaseValue = 0.5f;
    
    // UI-Zustand
    enum class DragMode {
        None,
        Attack,
        Decay, 
        Sustain,
        Release
    };
    
    DragMode currentDragMode = DragMode::None;
    juce::Point<float> lastMousePos;
    
    // Visuelle Eigenschaften
    juce::Colour primaryColor = juce::Colours::orange;
    juce::Colour secondaryColor = juce::Colours::darkorange;
    
    // Echtzeit-Plotting Daten
    static constexpr int plotBufferSize = 200;
    std::array<float, plotBufferSize> plotBuffer{};
    std::atomic<int> plotWriteIndex{0};
    std::atomic<float> currentEnvelopeValue{0.0f};
    std::atomic<bool> envelopeActive{false};
    std::atomic<int> adsrState{0}; // 0=idle, 1=attack, 2=decay, 3=sustain, 4=release
    
    // Animation
    float animationPhase = 0.0f;
    
    // Hilfsmethoden
    juce::Path createADSRPath() const;
    void drawEnvelopePlot(juce::Graphics& g);
    juce::Point<float> getEnvelopePositionOnCurve(float value) const;
    DragMode getHitTest(juce::Point<float> position) const;
    juce::Point<float> getAttackPoint() const;
    juce::Point<float> getDecayPoint() const;
    juce::Point<float> getSustainPoint() const;
    juce::Point<float> getReleasePoint() const;
    
    static float mapToADSRValue(float screenValue, bool isTime) ;
    static float mapFromADSRValue(float adsrValue, bool isTime) ;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ADSRComponent)
};