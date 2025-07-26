#include "ADSRComponent.hpp"

ADSRComponent::ADSRComponent()
{
    setSize(400, 150);
    setInterceptsMouseClicks(true, false);

    // Plot-Buffer initialisieren
    plotBuffer.fill(0.0f);

    // Timer für Echtzeit-Updates starten (60 FPS)
    startTimer(16);
}

ADSRComponent::~ADSRComponent()
{
    stopTimer();
}

void ADSRComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(10);

    // Hintergrund
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(bounds, 5.0f);

    // Umrandung
    g.setColour(primaryColor.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 5.0f, 2.0f);

    // Grid-Linien
    g.setColour(secondaryColor.withAlpha(0.2f));
    for (int i = 1; i < 4; ++i)
    {
        float y = bounds.getY() + (bounds.getHeight() / 4.0f) * i;
        g.drawLine(bounds.getX(), y, bounds.getRight(), y, 1.0f);
    }

    // ADSR-Kurve zeichnen
    auto path = createADSRPath();

    // Gefüllte Fläche unter der Kurve
    auto filledPath = path;
    filledPath.lineTo(bounds.getRight(), bounds.getBottom());
    filledPath.lineTo(bounds.getX(), bounds.getBottom());
    filledPath.closeSubPath();

    g.setColour(primaryColor.withAlpha(0.2f));
    g.fillPath(filledPath);

    // ADSR-Linie
    g.setColour(primaryColor);
    g.strokePath(path, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved));

    // Echtzeit-Plot zeichnen
    drawEnvelopePlot(g);

    // Kontrollpunkte zeichnen
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

    // Aktueller Envelope-Wert als großer Punkt
    if (envelopeActive.load())
    {
        auto currentPos = getEnvelopePositionOnCurve(currentEnvelopeValue.load());

        // Pulsierender Effekt
        float pulseSize = 8.0f + 4.0f * std::sin(animationPhase);

        // Farbbasiert auf ADSR-Phase
        juce::Colour phaseColor;
        switch (adsrState.load())
        {
            case 1: phaseColor = juce::Colours::red; break;      // Attack
            case 2: phaseColor = juce::Colours::yellow; break;   // Decay
            case 3: phaseColor = juce::Colours::green; break;    // Sustain
            case 4: phaseColor = juce::Colours::blue; break;     // Release
            default: phaseColor = juce::Colours::white; break;
        }

        // Glowing-Effekt
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

    // ADSR-Phase anzeigen
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
    // Wird aufgerufen wenn die Komponente ihre Größe ändert
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

void ADSRComponent::updateEnvelopeValue(float currentValue, bool isActive)
{
    currentEnvelopeValue.store(currentValue);
    envelopeActive.store(isActive);

    // Wert in Plot-Buffer speichern
    if (isActive)
    {
        int writeIndex = plotWriteIndex.load();
        plotBuffer[writeIndex] = currentValue;
        plotWriteIndex.store((writeIndex + 1) % plotBufferSize);
    }
}

void ADSRComponent::setADSRState(int state)
{
    adsrState.store(state);
}

void ADSRComponent::timerCallback()
{
    // Animation-Phase für Pulsieren aktualisieren
    animationPhase += 0.2f;
    if (animationPhase > juce::MathConstants<float>::twoPi)
        animationPhase -= juce::MathConstants<float>::twoPi;

    // Nur neu zeichnen wenn Envelope aktiv ist
    if (envelopeActive.load())
    {
        repaint();
    }
}

void ADSRComponent::drawEnvelopePlot(juce::Graphics& g)
{
    if (!envelopeActive.load()) return;

    auto bounds = getLocalBounds().toFloat().reduced(10);

    // Trail-Effekt: vorherige Werte als verlaufende Linie
    juce::Path trailPath;
    bool firstPoint = true;

    int currentWrite = plotWriteIndex.load();

    for (int i = 0; i < plotBufferSize; ++i)
    {
        int index = (currentWrite - plotBufferSize + i + plotBufferSize) % plotBufferSize;
        float value = plotBuffer[index];

        if (value > 0.0f)
        {
            auto pos = getEnvelopePositionOnCurve(value);

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

    // Trail mit Verlauf zeichnen
    if (!trailPath.isEmpty())
    {
        g.setColour(primaryColor.withAlpha(0.4f));
        g.strokePath(trailPath, juce::PathStrokeType(2.0f));
    }
}

juce::Point<float> ADSRComponent::getEnvelopePositionOnCurve(float value) const
{
    auto bounds = getLocalBounds().toFloat().reduced(10);

    float x, y;

    int state = adsrState.load();
    switch (state)
    {
        case 1: // Attack
        {
            auto attackPoint = getAttackPoint();
            x = juce::jmap(value, 0.0f, 1.0f, bounds.getX(), attackPoint.x);
            y = juce::jmap(value, 0.0f, 1.0f, bounds.getBottom(), bounds.getY());
            break;
        }
        case 2: // Decay
        {
            auto attackPoint = getAttackPoint();
            auto decayPoint = getDecayPoint();
            x = juce::jmap(value, 1.0f, sustainValue, attackPoint.x, decayPoint.x);
            y = juce::jmap(value, 1.0f, sustainValue, bounds.getY(), decayPoint.y);
            break;
        }
        case 3: // Sustain
        {
            auto decayPoint = getDecayPoint();
            auto sustainPoint = getSustainPoint();
            x = juce::jmap(0.5f, 0.0f, 1.0f, decayPoint.x, sustainPoint.x);
            y = bounds.getY() + (bounds.getHeight() * (1.0f - value));
            break;
        }
        case 4: // Release
        {
            auto sustainPoint = getSustainPoint();
            auto releasePoint = getReleasePoint();
            x = juce::jmap(1.0f - value, 0.0f, sustainValue, sustainPoint.x, releasePoint.x);
            y = juce::jmap(value, sustainValue, 0.0f, sustainPoint.y, bounds.getBottom());
            break;
        }
        default:
            x = bounds.getX();
            y = bounds.getBottom();
            break;
    }

    return {x, y};
}

juce::Path ADSRComponent::createADSRPath() const
{
    auto bounds = getLocalBounds().toFloat().reduced(10);
    juce::Path path;
    
    // Startpunkt (0, unten)
    float startX = bounds.getX();
    float startY = bounds.getBottom();
    path.startNewSubPath(startX, startY);
    
    // Attack-Phase
    auto attackPoint = getAttackPoint();
    path.lineTo(attackPoint);
    
    // Decay-Phase
    auto decayPoint = getDecayPoint();
    path.lineTo(decayPoint);
    
    // Sustain-Phase (horizontale Linie)
    auto sustainPoint = getSustainPoint();
    path.lineTo(sustainPoint);
    
    // Release-Phase
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
    float y = bounds.getY(); // Peak (oben)
    
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
    
    float x = bounds.getX() + bounds.getWidth() * 0.7f; // Feste Position
    float y = decayPoint.y; // Gleiche Höhe wie Decay-Ende
    
    return {x, y};
}

juce::Point<float> ADSRComponent::getReleasePoint() const
{
    auto bounds = getLocalBounds().toFloat().reduced(10);
    auto sustainPoint = getSustainPoint();
    
    float x = sustainPoint.x + (bounds.getWidth() * 0.3f * releaseValue);
    float y = bounds.getBottom(); // Zurück auf null
    
    return {x, y};
}

float ADSRComponent::mapToADSRValue(float screenValue, bool isTime)
{
    // Einfache lineare Abbildung für diese Implementierung
    return juce::jlimit(0.0f, 1.0f, screenValue);
}

float ADSRComponent::mapFromADSRValue(float adsrValue, bool isTime)
{
    // Einfache lineare Abbildung für diese Implementierung
    return juce::jlimit(0.0f, 1.0f, adsrValue);
}