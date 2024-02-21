
#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../model/MobiusConfig.h"
#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"
#include "../../model/Parameter.h"

#include "Colors.h"
#include "TrackStrip.h"
#include "StripElement.h"
#include "StripElements.h"

//////////////////////////////////////////////////////////////////////
//
// TrackNumber
//
//////////////////////////////////////////////////////////////////////

StripTrackNumber::StripTrackNumber(class TrackStrip* parent) :
    StripElement(parent, StripDefinitionTrackNumber)
{
}

StripTrackNumber::~StripTrackNumber()
{
}

int StripTrackNumber::getPreferredWidth()
{
    return 30;
}

int StripTrackNumber::getPreferredHeight()
{
    return 30;
}

void StripTrackNumber::paint(juce::Graphics& g)
{
    juce::Font font(getHeight());

    g.setFont(font);
    g.setColour(juce::Colour(MobiusGreen));

    // if we're docked, the TrackStrip has the number
    // otherwise update must have remembered the active track

    g.drawText(juce::String(strip->getTrackNumber() + 1), 0, 0, getWidth(), getHeight(),
               juce::Justification::centred);
}

//////////////////////////////////////////////////////////////////////
//
// FocusLock
//
//////////////////////////////////////////////////////////////////////

StripFocusLock::StripFocusLock(class TrackStrip* parent) :
    StripElement(parent, StripDefinitionFocusLock)
{
}

StripFocusLock::~StripFocusLock()
{
}

int StripFocusLock::getPreferredWidth()
{
    return 20;
}

int StripFocusLock::getPreferredHeight()
{
    return 20;
}

void StripFocusLock::update(MobiusState* state)
{
    int tracknum = strip->getTrackNumber();
    MobiusTrackState* track = &(state->tracks[tracknum]);

    if (track->focusLock != focusLock) {
        focusLock = track->focusLock;
        repaint();
    }
}

void StripFocusLock::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.drawEllipse((float)getX(), (float)getY(), (float)getWidth(), (float)getHeight(), 2.0f);

    if (focusLock) {
        g.setColour(juce::Colour(MobiusRed));
    }
    else {
        g.setColour(juce::Colours::black);
    }

    g.drawEllipse((float)getX() + 2, (float)getY() + 2, (float)getWidth() - 4, (float)getHeight() - 4, 2.0f);
}

//////////////////////////////////////////////////////////////////////
//
// LoopRadar
//
//////////////////////////////////////////////////////////////////////

StripLoopRadar::StripLoopRadar(class TrackStrip* parent) :
    StripElement(parent, StripDefinitionLoopRadar)
{
}

StripLoopRadar::~StripLoopRadar()
{
}

int StripLoopRadar::getPreferredWidth()
{
    return 30;
}

int StripLoopRadar::getPreferredHeight()
{
    return 30;
}

void StripLoopRadar::update(MobiusState* state)
{
    int tracknum = strip->getTrackNumber();
    MobiusTrackState* track = &(state->tracks[tracknum]);
    MobiusLoopState* loop = &(track->loops[track->activeLoop]);

    if (loop->frame != loopFrame ||
        loop->frames != loopFrames) {

        loopFrame = loop->frame;
        loopFrames = loop->frames;
        repaint();
    }
}

/**
 * Radians docs:
 * "the angle (clockwise) in radians at which to start the arc segment where
 * zero is the top center of the ellipse"
 *
 * "The radius is the distance from the center of a circle to its perimeter.
 * A radian is an angle whose corresponding arc in a circle is equal to the
 * radius of the circle"
 *
 * pi radians is 180 degrees so a full filled circle is 2pi
 *
 * For radians proportional to the position within a loop first get the loop
 * position as a fraction of the total loop:
 *
 *   float loopFraction = loopFrames / loopFrame;
 *
 * Then multiply that by 2pi
 *    
 */
void StripLoopRadar::paint(juce::Graphics& g)
{
    float twopi = 6.28318;

    // StripElement::paint(g);

    // start by redrawing the pie every time, can get smarter later
    g.setColour(juce::Colours::black);
    g.fillRect(0.0f, 0.0f, (float)getWidth(), (float)getHeight());

    if (loopFrames > 0) {
        float frames = (float)loopFrames;
        float frame = (float)loopFrame;
        float fraction = frame / frames;
        float startrad = 0.0f;
        float endrad = twopi * fraction;

        juce::Path path;
        int innerCircle = 0;

        // start radians, end radians, inner circle 
        path.addPieSegment(0.0f, 0.0f, (float)getWidth(), (float)getHeight(),
                           startrad, endrad, innerCircle);

        g.setColour(juce::Colour(MobiusRed));
        g.fillPath(path);
    }
}

//////////////////////////////////////////////////////////////////////
//
// LoopThermometer
//
// Alternative to radar that takes up less vertical space but more horizontal
//
//////////////////////////////////////////////////////////////////////

StripLoopThermometer::StripLoopThermometer(class TrackStrip* parent) :
    StripElement(parent, StripDefinitionLoopThermometer)
{
}

StripLoopThermometer::~StripLoopThermometer()
{
}

int StripLoopThermometer::getPreferredWidth()
{
    return 100;
}

int StripLoopThermometer::getPreferredHeight()
{
    return 10;
}

void StripLoopThermometer::update(MobiusState* state)
{
    int tracknum = strip->getTrackNumber();
    MobiusTrackState* track = &(state->tracks[tracknum]);
    MobiusLoopState* loop = &(track->loops[track->activeLoop]);

    if (loop->frame != loopFrame ||
        loop->frames != loopFrames) {

        loopFrame = loop->frame;
        loopFrames = loop->frames;
        repaint();
    }
}

void StripLoopThermometer::paint(juce::Graphics& g)
{
    float twopi = 6.28318;

    // StripElement::paint(g);

    // start by redrawing the pie every time, can get smarter later
    g.setColour(juce::Colours::black);
    g.fillRect(0.0f, 0.0f, (float)getWidth(), (float)getHeight());

    if (loopFrames > 0) {
        float frames = (float)loopFrames;
        float frame = (float)loopFrame;
        float fraction = frame / frames;
        float width = (float)getWidth() * fraction;
        
        g.setColour(juce::Colour(MobiusRed));
        g.fillRect(0.0f, 0.0f, width, (float)getHeight());
    }
}

//////////////////////////////////////////////////////////////////////
//
// LoopStack
//
// Displays brief information about all loops in a track
//
//////////////////////////////////////////////////////////////////////

const int LoopStackRowHeight = 12;
const int LoopStackNumberWidth = 12;
const int LoopStackHorizontalGap = 10;
const int LoopStackVerticalGap = 1;
const int LoopStackRectangleWidth = 60;
const int LoopStackBorderWidth = 1;

StripLoopStack::StripLoopStack(class TrackStrip* parent) :
    StripElement(parent, StripDefinitionLoopStack)
{
}

StripLoopStack::~StripLoopStack()
{
}

int StripLoopStack::getPreferredWidth()
{
    // needs to be called after configuration
    if (maxLoops == 0) {
        trace("LoopStack: maxLoops not configured!\n");
        maxLoops = 4;
    }

    return LoopStackNumberWidth + LoopStackHorizontalGap + LoopStackRectangleWidth;
}

/**
 * todo: to prevent this from becomming excessively large, should support
 * a maximum number of displayable loops and scroll within that.
 */
int StripLoopStack::getPreferredHeight()
{
    // needs to be called after configuration
    if (maxLoops == 0) {
        trace("LoopStack: maxLoops not configured!\n");
        maxLoops = 4;
    }

    return (LoopStackRowHeight + LoopStackVerticalGap) * maxLoops;
}

/**
 * Changing the number of loops effects the size of this component
 * so this needs to trigger a resize of the containing TrackStrip
 * and TrackStrips.  Until that is worked out, we require a restart.
 * I think the containers just needs to expect that when
 * configure is called anything inside can change size and it
 * needs to go through the layout process again
 */
void StripLoopStack::configure(MobiusConfig* config)
{
    // only the first time, then require restart to resize
    if (maxLoops == 0) {
        maxLoops = config->getMaxLoops();
    }
}

void StripLoopStack::configure(UIConfig* config)
{
}

/**
 * Like LoopMeter, we've got a more complex than usual substructure
 * so it is harder to do difference detection.
 * Update whenever the loop is moving for now.  This also gives us
 * a way to do progress bars if desired.
 */
void StripLoopStack::update(MobiusState* state)
{
    int tracknum = strip->getTrackNumber();

    // paint needs the entire track so save it locally
    track = &(state->tracks[tracknum]);
    MobiusLoopState* activeLoop = &(track->loops[track->activeLoop]);

    if (lastActive != track->activeLoop ||
        activeLoop->frame != lastFrame) {

        lastActive = track->activeLoop;
        lastFrame = activeLoop->frame;
        repaint();
    }
}

/**
 row for each loop with a filed rectangle representing loop state.
 * Old code was pretty basic, we could do a lot more now.
 */
void StripLoopStack::paint(juce::Graphics& g)
{
    // must have saved this in update
    if (track == nullptr) {
        trace("LoopStack: track not set\n");
        return;
    }

    // in theory the number of loops in the track could be different
    // than our original configuration, need to adapt to this?
    int trackLoops = maxLoops;
    if (track->loopCount < trackLoops)
      trackLoops = track->loopCount;
    
    for (int i = 0 ; i < trackLoops ; i++) {
        MobiusLoopState* loop = &(track->loops[i]);

        int rowTop = (LoopStackRowHeight + LoopStackVerticalGap) * i;
        
        // loop number
        if (i == track->activeLoop)
          g.setColour(juce::Colours::white);
        else
          g.setColour(juce::Colours::green);    // was a darker green

        g.drawText(juce::String(i+1), 0, rowTop, LoopStackNumberWidth, LoopStackRowHeight,
                   juce::Justification::centred);
        
        // border: white=active, black=inactive, yellow=switching, green=switchDestination
        // if we're recording and switching yellow may not stand out enough

        // determine switch destination before iteration since we can go up
        // although it is really a property of an event, summaraizer puts
        // it here to make it easier to find, should really be on the track
        // not sure why we made a distinction between switch and return, I guess
        // there was a little "R" somewhere, here we don't care
        int switchDestination = -1;
        if (loop->nextLoop >= 0)
          switchDestination = loop->nextLoop;
        else if (loop->returnLoop >= 0)
          switchDestination = loop->returnLoop;
        
        if (i == track->activeLoop) {
            // It's possible to switch to the same loop, an alternate
            // way to stack events, need a third color for this?
            if (switchDestination >= 0)
              g.setColour(juce::Colours::yellow);
            else
              g.setColour(juce::Colours::white);
        }
        else if (i == switchDestination) {
            g.setColour(juce::Colours::green);
        }
        else {
            // empty, leave it black, or just don't draw it
            g.setColour(juce::Colours::black);
        }

        int rectLeft = LoopStackNumberWidth + LoopStackHorizontalGap;
        // adjust for available size or keep it fixed?
        int rectWidth = getWidth() - rectLeft;
        g.drawRect(rectLeft, rowTop, rectWidth, LoopStackRowHeight);

        // border inset
        int blockLeft = rectLeft + LoopStackBorderWidth;
        int blockTop = rowTop + LoopStackBorderWidth;
        int blockWidth = rectWidth - (LoopStackBorderWidth * 2);
        int blockHeight = LoopStackRowHeight - (LoopStackBorderWidth * 2);

        // block: black=empty, grey=full, green=play, red=record, blue=mute
        // old code used grey to mean 1/2 speed

        if (loop->frames > 0) {
            juce::Colour color;
            if (i != track->activeLoop)
              color = juce::Colours::grey;
            // why have both flags?
            else if (loop->recording || loop->overdub)
              color = juce::Colours::red;
            if (loop->mute)
              color = juce::Colours::blue;
            else
              color = juce::Colours::green;
            g.setColour(color);

            g.drawRect(blockLeft, blockTop, blockWidth, blockHeight);
        }
        // else empty, leave it black
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
