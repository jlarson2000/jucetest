
#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../model/MobiusState.h"

#include "Colors.h"
#include "TrackStrip.h"
#include "StripElement.h"
#include "StripElements.h"

//////////////////////////////////////////////////////////////////////
//
// TrackNumber
//
//////////////////////////////////////////////////////////////////////

TrackNumberElement::TrackNumberElement(class TrackStrip* parent) :
    StripElement(parent, StripTrackNumber)
{
}

TrackNumberElement::~TrackNumberElement()
{
}

int TrackNumberElement::getPreferredWidth()
{
    return 30;
}

int TrackNumberElement::getPreferredHeight()
{
    return 30;
}

void TrackNumberElement::paint(juce::Graphics& g)
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
// LoopRadar
//
//////////////////////////////////////////////////////////////////////

LoopRadarElement::LoopRadarElement(class TrackStrip* parent) :
    StripElement(parent, StripLoopRadar)
{
}

LoopRadarElement::~LoopRadarElement()
{
}

int LoopRadarElement::getPreferredWidth()
{
    return 30;
}

int LoopRadarElement::getPreferredHeight()
{
    return 30;
}

void LoopRadarElement::update(MobiusState* state)
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
void LoopRadarElement::paint(juce::Graphics& g)
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

LoopThermometerElement::LoopThermometerElement(class TrackStrip* parent) :
    StripElement(parent, StripLoopThermometer)
{
}

LoopThermometerElement::~LoopThermometerElement()
{
}

int LoopThermometerElement::getPreferredWidth()
{
    return 100;
}

int LoopThermometerElement::getPreferredHeight()
{
    return 10;
}

void LoopThermometerElement::update(MobiusState* state)
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

void LoopThermometerElement::paint(juce::Graphics& g)
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
// OutputLevel
//
//////////////////////////////////////////////////////////////////////

OutputLevelElement::OutputLevelElement(class TrackStrip* parent) :
    StripRotary(parent, StripOutputLevel)
{
    min = 0;
    max = 127;
    slider.setRange((double)min, (double)max, 1);
}

OutputLevelElement::~OutputLevelElement()
{
}

void OutputLevelElement::update(MobiusState* state)
{
    int tracknum = strip->getTrackNumber();
    MobiusTrackState* track = &(state->tracks[tracknum]);

    if (track->outputLevel != value) {
        value = track->outputLevel;
        // testing, max it
        value = 127;
        slider.setValue((double)value);
        slider.repaint();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
