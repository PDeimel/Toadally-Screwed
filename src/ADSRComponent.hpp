/**
 * @file ADSRComponent.hpp
 * @brief Interactive ADSR envelope component with real-time visualization
 */

#pragma once

#include "JuceHeader.h"
#include <array>
#include <atomic>

/**
 * @brief Interactive ADSR envelope component with real-time visualization
 *
 * This component provides a visual representation of an ADSR envelope with
 * interactive control points for adjusting attack, decay, sustain, and release
 * parameters. It also displays real-time envelope values and phase information.
 */
class ADSRComponent : public juce::Component, private juce::Timer
{
public:
    /**
     * @brief Constructor
     */
    ADSRComponent();

    /**
     * @brief Destructor
     */
    ~ADSRComponent() override;

    /**
     * @brief Paint the component
     * @param g Graphics context for drawing
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Handle component resize
     */
    void resized() override;

    /**
     * @brief Handle mouse down events
     * @param event Mouse event information
     */
    void mouseDown(const juce::MouseEvent& event) override;

    /**
     * @brief Handle mouse drag events
     * @param event Mouse event information
     */
    void mouseDrag(const juce::MouseEvent& event) override;

    /**
     * @brief Handle mouse up events
     * @param event Mouse event information
     */
    void mouseUp(const juce::MouseEvent& event) override;

    /**
     * @brief Handle mouse move events
     * @param event Mouse event information
     */
    void mouseMove(const juce::MouseEvent& event) override;

    /**
     * @brief Set attack time parameter
     * @param attack Attack time (0.0 to 1.0)
     */
    void setAttack(float attack);

    /**
     * @brief Set decay time parameter
     * @param decay Decay time (0.0 to 1.0)
     */
    void setDecay(float decay);

    /**
     * @brief Set sustain level parameter
     * @param sustain Sustain level (0.0 to 1.0)
     */
    void setSustain(float sustain);

    /**
     * @brief Set release time parameter
     * @param release Release time (0.0 to 1.0)
     */
    void setRelease(float release);

    /**
     * @brief Get current attack time
     * @return Attack time value (0.0 to 1.0)
     */
    float getAttack() const { return attackValue; }

    /**
     * @brief Get current decay time
     * @return Decay time value (0.0 to 1.0)
     */
    float getDecay() const { return decayValue; }

    /**
     * @brief Get current sustain level
     * @return Sustain level value (0.0 to 1.0)
     */
    float getSustain() const { return sustainValue; }

    /**
     * @brief Get current release time
     * @return Release time value (0.0 to 1.0)
     */
    float getRelease() const { return releaseValue; }

    /**
     * @brief Callback function for parameter changes
     *
     * This function is called whenever ADSR parameters are modified through user interaction.
     * Parameters: attack, decay, sustain, release (all 0.0 to 1.0)
     */
    std::function<void(float attack, float decay, float sustain, float release)> onParameterChanged;

    /**
     * @brief Update component color scheme
     * @param primary Primary color for envelope curve and control points
     * @param secondary Secondary color for grid lines and accents
     */
    void updateColors(juce::Colour primary, juce::Colour secondary);

    /**
     * @brief Update real-time envelope value for visualization
     * @param currentValue Current envelope amplitude (0.0 to 1.0)
     * @param isActive Whether the envelope is currently active
     * @param timeInEnvelope Current time position within the envelope cycle
     */
    void updateEnvelopeValue(float currentValue, bool isActive, float timeInEnvelope = 0.0f);

    /**
     * @brief Set current ADSR phase state
     * @param state Current ADSR state (0=idle, 1=attack, 2=decay, 3=sustain, 4=release)
     */
    void setADSRState(int state);

private:
    /**
     * @brief Timer callback for animation updates
     */
    void timerCallback() override;

    /// ADSR parameter values (0.0 to 1.0)
    float attackValue = 0.1f;   ///< Attack time parameter
    float decayValue = 0.3f;    ///< Decay time parameter
    float sustainValue = 0.7f;  ///< Sustain level parameter
    float releaseValue = 0.5f;  ///< Release time parameter

    /**
     * @brief Enumeration of drag interaction modes
     */
    enum class DragMode {
        None,     ///< No dragging active
        Attack,   ///< Dragging attack control point
        Decay,    ///< Dragging decay control point
        Sustain,  ///< Dragging sustain control point
        Release   ///< Dragging release control point
    };

    DragMode currentDragMode = DragMode::None; ///< Current drag interaction state
    juce::Point<float> lastMousePos;           ///< Last recorded mouse position

    /// Visual properties
    juce::Colour primaryColor = juce::Colours::orange;     ///< Primary color for main elements
    juce::Colour secondaryColor = juce::Colours::darkorange; ///< Secondary color for accents

    /// Real-time plotting data
    static constexpr int plotBufferSize = 200;                    ///< Size of the plotting buffer
    std::array<float, plotBufferSize> plotBuffer{};              ///< Buffer for envelope values
    std::array<float, plotBufferSize> timeBuffer{};              ///< Buffer for time values
    std::atomic<int> plotWriteIndex{0};                          ///< Current write index for plot buffers
    std::atomic<float> currentEnvelopeValue{0.0f};              ///< Current real-time envelope value
    std::atomic<float> currentEnvelopeTime{0.0f};               ///< Current time in envelope cycle
    std::atomic<bool> envelopeActive{false};                    ///< Whether envelope is currently active
    std::atomic<int> adsrState{0};                              ///< Current ADSR phase state

    /// Animation
    float animationPhase = 0.0f; ///< Current animation phase for pulsing effects

    /**
     * @brief Create the main ADSR curve path
     * @return Path representing the ADSR envelope curve
     */
    juce::Path createADSRPath() const;

    /**
     * @brief Draw the real-time envelope plot trail
     * @param g Graphics context for drawing
     */
    void drawEnvelopePlot(juce::Graphics& g);

    /**
     * @brief Get screen position for envelope value at given time
     * @param value Envelope amplitude value (0.0 to 1.0)
     * @param timeInEnvelope Time position within envelope cycle
     * @return Screen coordinates for the envelope position
     */
    juce::Point<float> getEnvelopePositionOnCurve(float value, float timeInEnvelope) const;

    /**
     * @brief Perform hit testing for mouse interactions
     * @param position Mouse position to test
     * @return Drag mode corresponding to the hit element
     */
    DragMode getHitTest(juce::Point<float> position) const;

    /**
     * @brief Get screen position of attack control point
     * @return Screen coordinates of attack point
     */
    juce::Point<float> getAttackPoint() const;

    /**
     * @brief Get screen position of decay control point
     * @return Screen coordinates of decay point
     */
    juce::Point<float> getDecayPoint() const;

    /**
     * @brief Get screen position of sustain control point
     * @return Screen coordinates of sustain point
     */
    juce::Point<float> getSustainPoint() const;

    /**
     * @brief Get screen position of release control point
     * @return Screen coordinates of release point
     */
    juce::Point<float> getReleasePoint() const;

    /**
     * @brief Map screen value to ADSR parameter value
     * @param screenValue Input screen coordinate value
     * @param isTime Whether this represents a time parameter
     * @return Mapped ADSR parameter value
     */
    static float mapToADSRValue(float screenValue, bool isTime);

    /**
     * @brief Map ADSR parameter value to screen value
     * @param adsrValue Input ADSR parameter value
     * @param isTime Whether this represents a time parameter
     * @return Mapped screen coordinate value
     */
    static float mapFromADSRValue(float adsrValue, bool isTime);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ADSRComponent)
};