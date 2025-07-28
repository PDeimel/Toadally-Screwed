#include "VUMeterComponent.hpp"
#include <cmath>

//==============================================================================
VUMeterComponent::VUMeterComponent() {
    // Start the timer for regular updates
    startTimer(TIMER_INTERVAL_MS);
}

VUMeterComponent::~VUMeterComponent() {
    stopTimer();
}

//==============================================================================
void VUMeterComponent::updateLevels(float leftLevel, float rightLevel) {
    // Store levels atomically for thread safety
    leftLevelAtomic.store(juce::jlimit(0.0f, 1.0f, leftLevel));
    rightLevelAtomic.store(juce::jlimit(0.0f, 1.0f, rightLevel));
}

void VUMeterComponent::setColorScheme(juce::Colour primary, juce::Colour secondary) {
    primaryColor = primary;
    secondaryColor = secondary;
    repaint();
}

void VUMeterComponent::reset() {
    leftLevelAtomic.store(0.0f);
    rightLevelAtomic.store(0.0f);
    leftLevel = 0.0f;
    rightLevel = 0.0f;
    leftPeak = 0.0f;
    rightPeak = 0.0f;
    leftPeakHoldCounter = 0;
    rightPeakHoldCounter = 0;
    repaint();
}

//==============================================================================
void VUMeterComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.fillRoundedRectangle(bounds, 5.0f);

    // Border
    g.setColour(primaryColor.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 5.0f, 2.0f);

    // Calculate meter areas
    auto meterBounds = bounds.reduced(10.0f);
    auto channelWidth = (meterBounds.getWidth() - 10.0f) * 0.5f;

    // Left channel
    auto leftChannelBounds = juce::Rectangle<float>(
        meterBounds.getX(),
        meterBounds.getY(),
        channelWidth,
        meterBounds.getHeight()
    );

    // Right channel
    auto rightChannelBounds = juce::Rectangle<float>(
        leftChannelBounds.getRight() + 10.0f,
        meterBounds.getY(),
        channelWidth,
        meterBounds.getHeight()
    );

    // Draw channel meters
    drawChannelMeter(g, leftChannelBounds, leftLevel, leftPeak, "L");
    drawChannelMeter(g, rightChannelBounds, rightLevel, rightPeak, "R");
}

void VUMeterComponent::resized() {
}

void VUMeterComponent::timerCallback() {
    // Get atomic values
    float newLeftLevel = leftLevelAtomic.load();
    float newRightLevel = rightLevelAtomic.load();


    // Update left channel
    if (newLeftLevel > leftLevel) {
        leftLevel = newLeftLevel;
    } else {
        leftLevel *= LEVEL_DECAY_RATE;
        if (leftLevel < 0.001f) leftLevel = 0.0f;
    }

    // Update left peak
    if (newLeftLevel > leftPeak) {
        leftPeak = newLeftLevel;
        leftPeakHoldCounter = static_cast<int>(PEAK_HOLD_TIME_MS / TIMER_INTERVAL_MS);
    } else if (leftPeakHoldCounter > 0) {
        leftPeakHoldCounter--;
    } else {
        leftPeak *= PEAK_DECAY_RATE;
        if (leftPeak < 0.001f) leftPeak = 0.0f;
    }

    // Update right channel
    if (newRightLevel > rightLevel) {
        rightLevel = newRightLevel;
    } else {
        rightLevel *= LEVEL_DECAY_RATE;
        if (rightLevel < 0.001f) rightLevel = 0.0f;
    }

    // Update right peak
    if (newRightLevel > rightPeak) {
        rightPeak = newRightLevel;
        rightPeakHoldCounter = static_cast<int>(PEAK_HOLD_TIME_MS / TIMER_INTERVAL_MS);
    } else if (rightPeakHoldCounter > 0) {
        rightPeakHoldCounter--;
    } else {
        rightPeak *= PEAK_DECAY_RATE;
        if (rightPeak < 0.001f) rightPeak = 0.0f;
    }
    repaint();
}

//==============================================================================
float VUMeterComponent::levelToDb(float level) const {
    if (level <= 0.0f) {
        return MIN_DB;
    }

    float db = 20.0f * std::log10(level);
    return juce::jlimit(MIN_DB, MAX_DB, db);
}

float VUMeterComponent::dbToMeterPosition(float db) const {
    return (db - MIN_DB) / (MAX_DB - MIN_DB);
}

juce::Colour VUMeterComponent::getColorForLevel(float db) const {
    if (db >= 0.0f) {
        return redColor;
    } else if (db >= -6.0f) {
        return orangeColor;
    } else if (db >= -18.0f) {
        return yellowColor;
    } else {
        return greenColor;
    }
}

void VUMeterComponent::drawChannelMeter(juce::Graphics& g, juce::Rectangle<float> bounds,
                                       float level, float peak, const juce::String& channelName) {
    // Draw channel label
    auto labelBounds = bounds.removeFromTop(20.0f);
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.drawText(channelName, labelBounds, juce::Justification::centred);

    // Meter background
    auto meterBounds = bounds.reduced(2.0f);
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(meterBounds, 2.0f);

    // Calculate segment dimensions
    float segmentHeight = (meterBounds.getHeight() - (NUM_SEGMENTS - 1) * 1.0f) / NUM_SEGMENTS;
    float meterWidth = meterBounds.getWidth();

    // Convert levels to dB and positions
    float levelDb = levelToDb(level);
    float peakDb = levelToDb(peak);
    float levelPosition = dbToMeterPosition(levelDb);
    float peakPosition = dbToMeterPosition(peakDb);

    // Draw segments from bottom to top
    for (int i = 0; i < NUM_SEGMENTS; ++i) {
        float segmentTop = meterBounds.getBottom() - (i + 1) * (segmentHeight + 1.0f);
        float segmentNormalizedPosition = static_cast<float>(i) / static_cast<float>(NUM_SEGMENTS - 1);

        juce::Rectangle<float> segmentBounds(
            meterBounds.getX() + 1.0f,
            segmentTop,
            meterWidth - 2.0f,
            segmentHeight
        );

        // Determine segment color based on level
        float segmentDb = MIN_DB + segmentNormalizedPosition * (MAX_DB - MIN_DB);
        juce::Colour segmentColor = getColorForLevel(segmentDb);

        // Draw segment based on current level
        if (segmentNormalizedPosition <= levelPosition) {
            // Active segment
            g.setColour(segmentColor);
            g.fillRoundedRectangle(segmentBounds, 1.0f);
        } else {
            // Inactive segment
            g.setColour(segmentColor.withAlpha(0.2f));
            g.fillRoundedRectangle(segmentBounds, 1.0f);
        }

        // Draw peak indicator
        if (std::abs(segmentNormalizedPosition - peakPosition) < (1.0f / NUM_SEGMENTS)) {
            g.setColour(juce::Colours::white);
            g.fillRoundedRectangle(segmentBounds.reduced(0.5f), 1.0f);
        }
    }

    // Draw dB scale markings on the right side
    if (channelName == "R") {
        g.setColour(juce::Colours::white.withAlpha(0.7f));
        g.setFont(juce::FontOptions(9.0f));

        // Mark important dB levels
        std::array<float, 6> dbMarks = {0.0f, -6.0f, -12.0f, -18.0f, -24.0f, -30.0f};

        for (float dbMark : dbMarks) {
            if (dbMark >= MIN_DB && dbMark <= MAX_DB) {
                float markPosition = dbToMeterPosition(dbMark);
                float yPos = meterBounds.getBottom() - (markPosition * meterBounds.getHeight());

                g.drawText(juce::String(static_cast<int>(dbMark)),
                          bounds.getRight() + 5.0f, yPos - 6.0f, 30.0f, 12.0f,
                          juce::Justification::centredLeft);

                // Draw tick mark
                g.drawLine(bounds.getRight(), yPos, bounds.getRight() + 3.0f, yPos, 1.0f);
            }
        }
    }
}