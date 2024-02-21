/**
 * The LoopMeter is a rectangular "thermostat" that shows
 * the current playback position in the loop.
 * Undderneath is a set of tick marks representing the position
 * of cycles and subcycles.
 */

#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"
#include "../../model/UIEventType.h"

#include "Colors.h"
#include "StatusArea.h"
#include "LoopMeterElement.h"

// width of the colored bar that represents the loop position
const int MeterBarWidth = 200;
const int MeterBarHeight = 30;

// width of a border drawn around the colored bar
const int BorderWidth = 1;

// marker arrow
const int MarkerArrowWidth = 12;
const int MarkerArrowHeight = 10;

// marker text assume same size as arrow for now till
// we can mess with centering
const int MarkerTextHeight = 10;
const int MaxTextStack = 3;

// max width of an event marker, could be using Font size here
const int MaxMarkerWidth = MarkerArrowWidth;

// We center the marker on a point along the loop meter bar
// If this point is at the start or end, the marker needs to overhang
// on the left or right, which adds to the overall component width
const int MarkerOverhang = MaxMarkerWidth / 2;

const int TickHeight = 10;

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

// we do not support resizing larger or smaller, could but don't need to
int LoopMeterElement::getPreferredHeight()
{
    return MeterBarHeight + (BorderWidth * 2) + TickHeight + MarkerArrowHeight +
        (MarkerTextHeight * MaxTextStack);
}

int LoopMeterElement::getPreferredWidth()
{
    return MeterBarWidth + (BorderWidth * 2) + (MarkerOverhang * 2);
}

/**
 * This one is unusual because the event list is complcated
 * and we can't easilly do difference detection to trigger repaing.
 * Instead, let the advance of the play frame trigger repaint,
 * and repaint all the events every time.
 *
 * To avoid copying the event list, remember a pointer into
 * the MobiusState which is known to live between calls.
 */
void LoopMeterElement::update(MobiusState* state)
{
    int tracknum = state->activeTrack;
    MobiusTrackState* track = &(state->tracks[tracknum]);
    MobiusLoopState* activeLoop = &(track->loops[track->activeLoop]);

    // full repaint if we changed to another loop since the last one
    if (loop != activeLoop) {
        loop = activeLoop;
        repaint();
    }
    else if (loop->frame != savedFrame ||
             loop->frames != savedFrames) {
        // same loop different size or location
        savedFrame = loop->frame;
        savedFrames = loop->frames;
        repaint();
    }
}

void LoopMeterElement::resized()
{
}

/**
 * Don't need to repaint the whole thing if only the
 * meter bar and event list changes, but it seems fast enough.
 * Could break this down into subcomponents for the progress bar
 * and events.  Will want a verbose event list too.
 */
void LoopMeterElement::paint(juce::Graphics& g)
{
    // borders, labels, etc.
    StatusElement::paint(g);

    // outer border around the meter bar
    int leftOffset = MarkerOverhang;
    g.setColour(juce::Colour(MobiusBlue));
    g.drawRect(leftOffset, 0, MeterBarWidth + (BorderWidth*2), MeterBarHeight + (BorderWidth*2));

    // ticks subcycle ticks are yellow, start/end/cycle are white
    int subcycles = 4;  // need to get this from config
    int cycles = 2;  // until this is reliable, simulation assumes this
    int totalSubcycles = subcycles * cycles;
    int subcycleWidth = MeterBarWidth / totalSubcycles;
    int ticksToDraw = totalSubcycles + 1;
    int tickTop = MeterBarHeight + (BorderWidth * 2);
    int subcycleCount = 0;
    leftOffset = MarkerOverhang + BorderWidth;
    for (int i = 0 ; i < ticksToDraw ; i++) {
        if (subcycleCount > 0) {
            g.setColour(juce::Colours::yellow);
        }
        else {
            g.setColour(juce::Colours::white);
        }
        subcycleCount++;
        if (subcycleCount == subcycles)
          subcycleCount = 0;
        
        g.drawLine((float)leftOffset, (float)tickTop, (float)leftOffset,
                   (float)(tickTop + TickHeight));

        leftOffset += subcycleWidth;
    }

    // meter bar
    leftOffset = MarkerOverhang + BorderWidth;
    if (loop->frames > 0) {
        int width = getMeterOffset(loop->frame);
        g.setColour(juce::Colour(MobiusRed));
        g.fillRect((float)leftOffset, (float)BorderWidth,
                   (float)width, (float)MeterBarHeight);
    }

    // events
    // clear it out first?
    juce::Font font(MarkerTextHeight);
    g.setFont(font);
    int eventInfoLeft = MarkerOverhang + BorderWidth;
    int eventInfoTop = (BorderWidth * 2) + MeterBarHeight + TickHeight;
    if (loop->eventCount > 0) {
        int lastEventFrame = -1;
        int stackCount = 0;
        for (int i = 0 ; i < loop->eventCount ; i++) {
            MobiusEventState* ev = &(loop->events[i]);
            int eventOffset = getMeterOffset(ev->frame);
            int arrowLeft = eventInfoLeft + eventOffset - (MarkerArrowWidth / 2);
            // should also stack if "close enough"
            // should really be testing the scaled location of the markers
            // the loop frame
            if (ev->frame != lastEventFrame) {
                // reset this if we were stacking
                stackCount = 0;
                g.setColour(juce::Colours::red);
                // todo: should be a triangle
                g.fillRect(arrowLeft, eventInfoTop, MarkerArrowWidth, MarkerArrowHeight);
            }
            
            g.setColour(juce::Colours::white);
            const char* symbol = ev->type->timelineSymbol;
            if (symbol == nullptr) {
                symbol = "?";
                trace("Event with no symbol %s\n", ev->type->name);
            }
            
            int textTop = eventInfoTop + MarkerArrowHeight + (MarkerTextHeight * stackCount);

            g.drawText(juce::String(symbol),arrowLeft, textTop,
                       MarkerArrowWidth, MarkerTextHeight,
                       juce::Justification::centred);
            stackCount++;
            lastEventFrame = ev->frame;
        }
    }
}

/**
 * Common calculation for paint
 * Convert a loop location expressed in frames into the
 * corresponding X coordinate of the visible meter.
 * We're insetting the colored meter bar by 2 to give it a border.
 * Event markers need to track that too.
 */
int LoopMeterElement::getMeterOffset(long frame)
{
    int offset = 0;

    if (loop->frames == 0) {
        // happened during testing, might happen if
        // we pre-schedule events before recording,
        // should push them to the end
    }
    else {
        // the percentage of the frame within the loop
        float fraction = (float)frame / (float)(loop->frames);

        // the width we have available minus insets
        int width = MeterBarWidth;

        // offset within the meter of that frame
        offset = (int)((float)width * fraction);
    }
    
    return offset;
}

    
