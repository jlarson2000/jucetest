/**
 * The LoopMeter is a rectangular "thermostat" that shows
 * the current playback position in the loop.
 * Undderneath is a set of tick marks representing the position
 * of cycles and subcycles.
 */

#include <JuceHeader.h>

#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"

#include "Colors.h"
#include "StatusArea.h"
#include "LoopMeterElement.h"

const int LoopMeterWidth = 200;
const int LoopMeterHeight = 30;
const int LoopMeterTickHeight = 20;
const int LoopMeterCycleTickHeight = 20;
const int LoopMeterSubcycleTickHeight = 12;
const int LoopMeterEventHeight = 20;

LoopMeterElement::LoopMeterElement(StatusArea* area) :
    StatusElement(area, "LoopMeterElement")
{
}

LoopMeterElement::~LoopMeterElement()
{
}

void LoopMeterElement::configure(UIConfig* config)
{
}

void LoopMeterElement::update(MobiusState* state)
{
    MobiusTrackState* track = &(state->tracks[state->activeTrack]);
    MobiusLoopState* loop = &(track->loops[track->activeLoop]);
}

int LoopMeterElement::getPreferredHeight()
{
    return LoopMeterHeight + LoopMeterTickHeight + LoopMeterEventHeight;
}

int LoopMeterElement::getPreferredWidth()
{
    return LoopMeterWidth;
}

void LoopMeterElement::resized()
{
}

void LoopMeterElement::paint(juce::Graphics& g)
{
    // borders, labels, etc.
    StatusElement::paint(g);

    g.setColour(juce::Colour(MobiusBlue));
    g.drawRect(0, 0, getWidth(), LoopMeterHeight);

    int subcycles = 4;  // need to get this from config
    // todo: cycles
    int subcycleWidth = getWidth() / subcycles;

    g.setColour(juce::Colours::white);
    // iterate one extra to get a line at far right
    for (int i = 0 ; i < (subcycles + 1); i++) {
        int subcycleLeft = i * subcycleWidth;
        int tickHeight = LoopMeterSubcycleTickHeight;
        if (i == 0 || i > subcycles)
          tickHeight = LoopMeterTickHeight;
        g.drawLine((float)subcycleLeft, (float)LoopMeterHeight,
                   (float)subcycleLeft, (float)(LoopMeterHeight + tickHeight));
    }
}

    
