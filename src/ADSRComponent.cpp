/**
 * @file ADSRComponent.cpp
 * @brief Implementation of interactive ADSR envelope component
 */

#include "ADSRComponent.hpp"

ADSRComponent::ADSRComponent()
{
    setSize(400, 150);
    setInterceptsMouseClicks(true, false);

    // Initialize plot buffers
    plotBuffer.fill(0.0f);
    timeBuffer.fill(0.0f);

    // Start timer for real-time updates (60 FPS)
    startTimer(16);
}

ADSRComponent::~ADSRComponent()
{
    stopTimer();
}

void ADSRComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(10);

    // Background
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(bounds, 5.0f);

    // Border
    g.setColour(primaryColor.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 5.0f, 2.0f);

    // Grid lines
    g.setColour(secondaryColor.withAlpha(0.2f));
    for (int i = 1; i < 4; ++i)
    {
        float y = bounds.getY() + (bounds.getHeight() / 4.0f) * i;
        g.drawLine(bounds.getX(), y, bounds.getRight(), y, 1.0f);
    }

    // Draw ADSR curve
    auto path = createADSRPath();

    // Filled area under the curve
    auto filledPath = path;
    filledPath.lineTo(bounds.getRight(), bounds.getBottom());
    filledPath.lineTo(bounds.getX(), bounds.getBottom());
    filledPath.closeSubPath();

    g.setColour(primaryColor.withAlpha(0.2f));
    g.fillPath(filledPath);

    // ADSR line
    g.setColour(primaryColor);
    g.strokePath(path, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved));

    // Draw real-time envelope plot
    drawEnvelopePlot(g);

    // Draw control points
    auto attackPoint = getAttackPoint();
    auto decayPoint = getDecayPoint();
    auto sustainPoint = getSustainPoint();
    auto releasePoint = getReleasePoint();

    auto drawControlPoint = [&](juce::Point<float> point, bool isActive) {
        float radius = isActive ? 8.0f : 6.0f;
        g.setColour(isActive ? primaryColor : primaryColor.withAlpha(0.8f));
        g.fillEllipse(point.x - radius/2, point.y - radius/2, radius, radius);

        g.setColour(juce::Colours::white);
        g.drawEllipse(point.x - radius/2, point.y - radius/2, radius, radius, 2.0f);
    };

    drawControlPoint(attackPoint, currentDragMode == DragMode::Attack);
    drawControlPoint(decayPoint, currentDragMode == DragMode::Decay);
    drawControlPoint(sustainPoint, currentDragMode == DragMode::Sustain);
    drawControlPoint(releasePoint, currentDragMode == DragMode::Release);

    // Current envelope value as large point
    if (envelopeActive.load())
    {
        auto currentPos = getEnvelopePositionOnCurve(currentEnvelopeValue.load(), currentEnvelopeTime.load());

        // Pulsing effect
        float pulseSize = 8.0f + 4.0f * std::sin(animationPhase);

        // Color based on ADSR phase
        juce::Colour phaseColor;
        switch (adsrState.load())
        {
            case 1: phaseColor = juce::Colours::red; break;      // Attack
            case 2: phaseColor = juce::Colours::yellow; break;   // Decay
            case 3: phaseColor = juce::Colours::green; break;    // Sustain
            case 4: phaseColor = juce::Colours::blue; break;     // Release
            default: phaseColor = juce::Colours::white; break;
        }

        // Glowing effect
        g.setColour(phaseColor.withAlpha(0.3f));
        g.fillEllipse(currentPos.x - pulseSize, currentPos.y - pulseSize,
                     pulseSize * 2, pulseSize * 2);

        g.setColour(phaseColor);
        g.fillEllipse(currentPos.x - 6, currentPos.y - 6, 12, 12);

        g.setColour(juce::Colours::white);
        g.drawEllipse(currentPos.x - 6, currentPos.y - 6, 12, 12, 2.0f);
    }

    // Labels
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);

    g.drawText("A", static_cast<int>(attackPoint.x - 10), static_cast<int>(bounds.getBottom() + 5), 20, 15, juce::Justification::centred);
    g.drawText("D", static_cast<int>(decayPoint.x - 10), static_cast<int>(bounds.getBottom() + 5), 20, 15, juce::Justification::centred);
    g.drawText("S", static_cast<int>(sustainPoint.x - 10), static_cast<int>(bounds.getBottom() + 5), 20, 15, juce::Justification::centred);
    g.drawText("R", static_cast<int>(releasePoint.x - 10), static_cast<int>(bounds.getBottom() + 5), 20, 15, juce::Justification::centred);

    // Display ADSR phase
    if (envelopeActive.load())
    {
        juce::String phaseText;
        switch (adsrState.load())
        {
            case 1: phaseText = "ATTACK"; break;
            case 2: phaseText = "DECAY"; break;
            case 3: phaseText = "SUSTAIN"; break;
            case 4: phaseText = "RELEASE"; break;
            default: phaseText = "IDLE"; break;
        }

        g.setColour(juce::Colours::white);
        g.setFont(14.0f);
        g.drawText(phaseText, static_cast<int>(bounds.getX()), static_cast<int>(bounds.getY() - 20), 100, 20, juce::Justification::left);
    }
}

void ADSRComponent::resized()
{
    // Called when the component changes size
}

void ADSRComponent::mouseDown(const juce::MouseEvent& event)
{
    lastMousePos = event.position;
    currentDragMode = getHitTest(event.position);

    if (currentDragMode != DragMode::None)
    {
        repaint();
    }
}

void ADSRComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (currentDragMode == DragMode::None)
        return;

    auto bounds = getLocalBounds().toFloat().reduced(10);
    auto delta = event.position - lastMousePos;

    const float sensitivity = 0.005f;

    float oldAttack = attackValue;
    float oldDecay = decayValue;
    float oldSustain = sustainValue;
    float oldRelease = releaseValue;

    switch (currentDragMode)
    {
        case DragMode::Attack:
            attackValue = juce::jlimit(0.01f, 1.0f, attackValue + delta.x * sensitivity);
            break;

        case DragMode::Decay:
            decayValue = juce::jlimit(0.01f, 1.0f, decayValue + delta.x * sensitivity);
            break;

        case DragMode::Sustain:
            sustainValue = juce::jlimit(0.0f, 1.0f, sustainValue - delta.y * sensitivity);
            break;

        case DragMode::Release:
            releaseValue = juce::jlimit(0.01f, 1.0f, releaseValue + delta.x * sensitivity);
            break;

        default:
            break;
    }

    lastMousePos = event.position;

    bool valuesChanged = !juce::approximatelyEqual(attackValue, oldAttack) ||
                        !juce::approximatelyEqual(decayValue, oldDecay) ||
                        !juce::approximatelyEqual(sustainValue, oldSustain) ||
                        !juce::approximatelyEqual(releaseValue, oldRelease);

    if (valuesChanged && onParameterChanged)
    {
        onParameterChanged(attackValue, decayValue, sustainValue, releaseValue);
    }

    repaint();
}

void ADSRComponent::mouseUp(const juce::MouseEvent& event)
{
    currentDragMode = DragMode::None;
    repaint();
}

void ADSRComponent::mouseMove(const juce::MouseEvent& event)
{
    auto hitTest = getHitTest(event.position);

    if (hitTest != DragMode::None)
    {
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
    }
    else
    {
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
}

void ADSRComponent::setAttack(float attack)
{
    attackValue = juce::jlimit(0.01f, 1.0f, attack);
    repaint();
}

void ADSRComponent::setDecay(float decay)
{
    decayValue = juce::jlimit(0.01f, 1.0f, decay);
    repaint();
}

void ADSRComponent::setSustain(float sustain)
{
    sustainValue = juce::jlimit(0.0f, 1.0f, sustain);
    repaint();
}

void ADSRComponent::setRelease(float release)
{
    releaseValue = juce::jlimit(0.01f, 1.0f, release);
    repaint();
}

void ADSRComponent::updateColors(juce::Colour primary, juce::Colour secondary)
{
    primaryColor = primary;
    secondaryColor = secondary;
    repaint();
}

void ADSRComponent::updateEnvelopeValue(float currentValue, bool isActive, float timeInEnvelope)
{
    // Validate values before storing them
    float validValue = juce::jlimit(0.0f, 1.0f, currentValue);
    float validTime = juce::jmax(0.0f, timeInEnvelope);

    currentEnvelopeValue.store(validValue);
    currentEnvelopeTime.store(validTime);
    envelopeActive.store(isActive);

    // Store both value AND time in plot buffers
    if (isActive)
    {
        int writeIndex = plotWriteIndex.load();
        plotBuffer[writeIndex] = validValue;
        timeBuffer[writeIndex] = validTime;  // Also store time!
        plotWriteIndex.store((writeIndex + 1) % plotBufferSize);
    }
}

void ADSRComponent::setADSRState(int state)
{
    adsrState.store(state);
}

void ADSRComponent::timerCallback()
{
    // Update animation phase for pulsing
    animationPhase += 0.2f;
    if (animationPhase > juce::MathConstants<float>::twoPi)
        animationPhase -= juce::MathConstants<float>::twoPi;

    // Only repaint when envelope is active
    if (envelopeActive.load())
    {
        repaint();
    }
}

void ADSRComponent::drawEnvelopePlot(juce::Graphics& g)
{
    if (!envelopeActive.load()) return;

    auto bounds = getLocalBounds().toFloat().reduced(10);

    // Trail effect: previous values as fading line
    juce::Path trailPath;
    bool firstPoint = true;

    int currentWrite = plotWriteIndex.load();

    for (int i = 0; i < plotBufferSize; ++i)
    {
        int index = (currentWrite - plotBufferSize + i + plotBufferSize) % plotBufferSize;
        float value = plotBuffer[index];
        float time = timeBuffer[index];  // Use time!

        // Additional validation
        if (value > 0.0f && time >= 0.0f && std::isfinite(value) && std::isfinite(time))
        {
            auto pos = getEnvelopePositionOnCurve(value, time);  // With time!

            // Validate position
            if (std::isfinite(pos.x) && std::isfinite(pos.y))
            {
                if (firstPoint)
                {
                    trailPath.startNewSubPath(pos);
                    firstPoint = false;
                }
                else
                {
                    trailPath.lineTo(pos);
                }
            }
        }
    }

    // Draw trail with fade
    if (!trailPath.isEmpty())
    {
        g.setColour(primaryColor.withAlpha(0.4f));
        g.strokePath(trailPath, juce::PathStrokeType(2.0f));
    }
}

juce::Point<float> ADSRComponent::getEnvelopePositionOnCurve(float value, float timeInEnvelope) const
{
    auto bounds = getLocalBounds().toFloat().reduced(10);

    // Calculate total ADSR time
    float totalAttackTime = juce::jmax(0.01f, attackValue);
    float totalDecayTime = juce::jmax(0.01f, decayValue);
    float totalSustainTime = 0.5f; // Sustain has no fixed time, but for plotting we use 0.5
    float totalReleaseTime = juce::jmax(0.01f, releaseValue);

    float totalTime = totalAttackTime + totalDecayTime + totalSustainTime + totalReleaseTime;

    // Safety check against division by zero
    if (totalTime <= 0.0f)
        totalTime = 1.0f;

    // Normalized time (0.0 to 1.0) over the entire ADSR curve
    float normalizedTime = juce::jlimit(0.0f, 1.0f, timeInEnvelope / totalTime);

    // X position based on time
    float x = bounds.getX() + (bounds.getWidth() * normalizedTime);

    // Y position based on envelope value
    float y = bounds.getY() + (bounds.getHeight() * (1.0f - juce::jlimit(0.0f, 1.0f, value)));

    return {x, y};
}

juce::Path ADSRComponent::createADSRPath() const
{
    auto bounds = getLocalBounds().toFloat().reduced(10);
    juce::Path path;

    // Start point (0, bottom)
    float startX = bounds.getX();
    float startY = bounds.getBottom();
    path.startNewSubPath(startX, startY);

    // Attack phase
    auto attackPoint = getAttackPoint();
    path.lineTo(attackPoint);

    // Decay phase
    auto decayPoint = getDecayPoint();
    path.lineTo(decayPoint);

    // Sustain phase (horizontal line)
    auto sustainPoint = getSustainPoint();
    path.lineTo(sustainPoint);

    // Release phase
    auto releasePoint = getReleasePoint();
    path.lineTo(releasePoint);

    return path;
}

ADSRComponent::DragMode ADSRComponent::getHitTest(juce::Point<float> position) const
{
    const float hitRadius = 10.0f;

    if (getAttackPoint().getDistanceFrom(position) < hitRadius)
        return DragMode::Attack;
    if (getDecayPoint().getDistanceFrom(position) < hitRadius)
        return DragMode::Decay;
    if (getSustainPoint().getDistanceFrom(position) < hitRadius)
        return DragMode::Sustain;
    if (getReleasePoint().getDistanceFrom(position) < hitRadius)
        return DragMode::Release;

    return DragMode::None;
}

juce::Point<float> ADSRComponent::getAttackPoint() const
{
    auto bounds = getLocalBounds().toFloat().reduced(10);

    float x = bounds.getX() + (bounds.getWidth() * 0.25f * attackValue);
    float y = bounds.getY(); // Peak (top)

    return {x, y};
}

juce::Point<float> ADSRComponent::getDecayPoint() const
{
    auto bounds = getLocalBounds().toFloat().reduced(10);
    auto attackPoint = getAttackPoint();

    float x = attackPoint.x + (bounds.getWidth() * 0.25f * decayValue);
    float y = bounds.getY() + (bounds.getHeight() * (1.0f - sustainValue));

    return {x, y};
}

juce::Point<float> ADSRComponent::getSustainPoint() const
{
    auto bounds = getLocalBounds().toFloat().reduced(10);
    auto decayPoint = getDecayPoint();

    float x = bounds.getX() + bounds.getWidth() * 0.7f; // Fixed position
    float y = decayPoint.y; // Same height as decay end

    return {x, y};
}

juce::Point<float> ADSRComponent::getReleasePoint() const
{
    auto bounds = getLocalBounds().toFloat().reduced(10);
    auto sustainPoint = getSustainPoint();

    float x = sustainPoint.x + (bounds.getWidth() * 0.3f * releaseValue);
    float y = bounds.getBottom(); // Back to zero

    return {x, y};
}

float ADSRComponent::mapToADSRValue(float screenValue, bool isTime)
{
    // Simple linear mapping for this implementation
    return juce::jlimit(0.0f, 1.0f, screenValue);
}

float ADSRComponent::mapFromADSRValue(float adsrValue, bool isTime)
{
    // Simple linear mapping for this implementation
    return juce::jlimit(0.0f, 1.0f, adsrValue);
}