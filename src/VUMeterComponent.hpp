#pragma once

#include "JuceHeader.h"
#include <atomic>
#include <array>

/**
 * @brief VU Meter component for displaying audio level visualization
 * 
 * This component provides a professional-looking VU meter with peak and RMS level display,
 * including proper ballistics and color-coded level indicators.
 */
class VUMeterComponent : public juce::Component, public juce::Timer {
public:
    /**
     * @brief Constructor
     */
    VUMeterComponent();
    
    /**
     * @brief Destructor
     */
    ~VUMeterComponent() override;
    
    /**
     * @brief Update the meter with new audio level data
     * @param leftLevel Left channel level (0.0 to 1.0)
     * @param rightLevel Right channel level (0.0 to 1.0)
     */
    void updateLevels(float leftLevel, float rightLevel);
    
    /**
     * @brief Set the color scheme for the meter
     * @param primary Primary color for the meter
     * @param secondary Secondary color for accents
     */
    void setColorScheme(juce::Colour primary, juce::Colour secondary);
    
    /**
     * @brief Reset the meter (clear all levels and peaks)
     */
    void reset();

    //==============================================================================
    // Component overrides
    
    /**
     * @brief Paint the component
     * @param g Graphics context
     */
    void paint(juce::Graphics& g) override;
    
    /**
     * @brief Handle component resizing
     */
    void resized() override;
    
    /**
     * @brief Timer callback for updating meter ballistics
     */
    void timerCallback() override;

private:
    /**
     * @brief Calculate decibel value from linear level
     * @param level Linear level (0.0 to 1.0)
     * @return Decibel value
     */
    float levelToDb(float level) const;
    
    /**
     * @brief Convert decibel value to meter position
     * @param db Decibel value
     * @return Normalized position (0.0 to 1.0)
     */
    float dbToMeterPosition(float db) const;
    
    /**
     * @brief Get color for given level
     * @param db Level in decibels
     * @return Color for the level
     */
    juce::Colour getColorForLevel(float db) const;
    
    /**
     * @brief Draw a single channel meter
     * @param g Graphics context
     * @param bounds Bounds for the meter
     * @param level Current level (0.0 to 1.0)
     * @param peak Peak level (0.0 to 1.0)
     * @param channelName Name of the channel
     */
    void drawChannelMeter(juce::Graphics& g, juce::Rectangle<float> bounds, 
                         float level, float peak, const juce::String& channelName);

    //==============================================================================
    // Constants
    
    static constexpr int UPDATE_RATE_HZ = 60;           ///< Update rate in Hz
    static constexpr int TIMER_INTERVAL_MS = 1000 / UPDATE_RATE_HZ; ///< Timer interval
    static constexpr float PEAK_HOLD_TIME_MS = 1500.0f; ///< Peak hold time in milliseconds
    static constexpr float PEAK_DECAY_RATE = 0.99f;     ///< Peak decay rate per update
    static constexpr float LEVEL_DECAY_RATE = 0.95f;    ///< Level decay rate per update
    static constexpr float MIN_DB = -60.0f;             ///< Minimum dB value to display
    static constexpr float MAX_DB = 6.0f;               ///< Maximum dB value to display
    static constexpr int NUM_SEGMENTS = 20;             ///< Number of LED segments per channel
    
    //==============================================================================
    // Member variables
    
    // Level data (thread-safe)
    std::atomic<float> leftLevelAtomic{0.0f};           ///< Left channel level (atomic)
    std::atomic<float> rightLevelAtomic{0.0f};          ///< Right channel level (atomic)
    
    // Display levels (accessed from timer thread)
    float leftLevel{0.0f};                              ///< Left channel display level
    float rightLevel{0.0f};                             ///< Right channel display level
    float leftPeak{0.0f};                               ///< Left channel peak level
    float rightPeak{0.0f};                              ///< Right channel peak level
    
    // Peak hold timing
    int leftPeakHoldCounter{0};                         ///< Left peak hold counter
    int rightPeakHoldCounter{0};                        ///< Right peak hold counter
    
    // Colors
    juce::Colour primaryColor{juce::Colours::green};    ///< Primary meter color
    juce::Colour secondaryColor{juce::Colours::darkgreen}; ///< Secondary meter color
    
    // Gradient colors for different levels
    juce::Colour greenColor{0xFF00FF00};                ///< Green level color
    juce::Colour yellowColor{0xFFFFFF00};               ///< Yellow level color
    juce::Colour orangeColor{0xFFFF8000};               ///< Orange level color
    juce::Colour redColor{0xFFFF0000};                  ///< Red level color
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeterComponent)
};