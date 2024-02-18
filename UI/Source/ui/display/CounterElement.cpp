/**
 * The old counter has these numbers from left to right modeled after the EDP
 *
 *  current loop number
 *  loop position seconds as a two digit floating point number
 *    displayed in a larger font
 *  cycle indicator
 *
 * The cycle indicator was two numbers with a slash in between
 *    current cycle / total cycles
 *
 */

#include <JuceHeader.h>

#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"
#include "../../model/ModeDefinition.h"

#include "Colors.h"
#include "StatusArea.h"
#include "CounterElement.h"

const int CounterHeight = 30;

// see notes in paint on total digits
const int CounterDigits = 13;

const int BorderGap = 1;

CounterElement::CounterElement(StatusArea* area) :
    StatusElement(area, "CounterElement")
{
    // not sure if we should cache this
    font = juce::Font(CounterHeight - (BorderGap * 2));
    digitWidth = font.getStringWidth("M");
}

CounterElement::~CounterElement()
{
}

void CounterElement::configure(UIConfig* config)
{
}

int CounterElement::getPreferredHeight()
{
    return CounterHeight;
}

int CounterElement::getPreferredWidth()
{
    return (digitWidth * CounterDigits) + (BorderGap * 2);
}

void CounterElement::resized()
{
}

void CounterElement::update(MobiusState* state)
{
    MobiusTrackState* track = &(state->tracks[state->activeTrack]);
    MobiusLoopState* loop = &(track->loops[track->activeLoop]);

    // has anything changed?
    if (loopNumber != loop->number ||
        loopFrame != loop->frame ||
        cycle != loop->cycle ||
        cycles != loop->cycles) {

        loopNumber = loop->number;
        loopFrame = loop->frame;
        cycle = loop->cycle;
        cycles = loop->cycles;

        repaint();
    }
}

void CounterElement::paint(juce::Graphics& g)
{
    // borders, labels, etc.
    StatusElement::paint(g);

    // clear the background, inset by one
    juce::Rectangle<int> area = getLocalBounds();
    int borderAdjust = (BorderGap * 2);
    // note that this returns, not modifies like removeFromTop
    area = area.withSizeKeepingCentre(area.getWidth() - borderAdjust, area.getHeight() - borderAdjust);

    g.setColour(juce::Colours::black);
    g.fillRect(area);

    // Juce makes this easier by handling justification of multiple digits
    // loop number (1), gap (1), loop seconds (3) right, dot (1), loop tenths (1) left, gap (1),
    // cycle (2) right, slash (1), cycles (2) left
    // total 13
    g.setFont(font);
    g.setColour(juce::Colour(MobiusBlue));
    
    // loop number
    g.drawText(juce::String(loopNumber + 1), area.removeFromLeft(digitWidth),
               juce::Justification::centredLeft);

    // gap
    area.removeFromLeft(digitWidth);

    // seconds . tenths
    int sampleRate = 44100;
    int totalTenths = loopFrame / (sampleRate / 10);
    int tenths = totalTenths % 10;
    int seconds = totalTenths / 10;
    
    g.drawText(juce::String(seconds), area.removeFromLeft(digitWidth * 3),
               juce::Justification::centredRight);

    g.drawText(juce::String("."), area.removeFromLeft(digitWidth),
               juce::Justification::centred);

    g.drawText(juce::String(tenths), area.removeFromLeft(digitWidth),
               juce::Justification::centredLeft);
               
    // gap
    area.removeFromLeft(digitWidth);

    // cycle/cycles
    // cycle is zero based
    // cycles is 1 based, zero means empty
    // old dispaly always showed 1 if empty
    
    int cycleToDraw = cycle + 1;
    int cyclesToDraw = (cycles > 0) ? cycles : 1;
    
    g.drawText(juce::String(cycleToDraw), area.removeFromLeft(digitWidth * 2),
               juce::Justification::centredRight);

    g.drawText(juce::String("/"), area.removeFromLeft(digitWidth),
               juce::Justification::centred);

    g.drawText(juce::String(cyclesToDraw), area.removeFromLeft(digitWidth * 2),
               juce::Justification::centredLeft);

    // so...
    // 1 loop number, 1 gap, 3 seconds, 1 dot, 1 tenths, 1 gap, 2 cycle, 1 slash, 2 cycles
    // total 13
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
