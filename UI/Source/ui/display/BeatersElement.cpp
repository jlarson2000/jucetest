/**
 * Status area component to display playback beats.
 *
 * BeaterElement manages a set of three Beaters to represent
 * reaching a subcycle, cycle, or loop boundary.
 *
 * When a beater is turned on, it will display highlighted
 * for a period of time then turn off.  This period is determined
 * buy a number of "ticks".  A tick happens when Beaters
 * is informed by something managing a timer.  Exactly what this
 * is is not defined here, but it is expected to be called frequently.
 * Old mobius used a SimpleTimer with a period of 100ms.
 *
 * Under Juce we're not going to implement our own timer, instead
 * it will use the periodic udpateStatus call which is expected to
 * happen around 1/10 second.  If we ever need to change that assumption
 * we'll need to adjust the meaning of a tick here.  So that the beater
 * can remain lit for a visible amount of time regardless of the period
 * of the external timer.
 *
 * Okay, I see the problem here.  The signal that a beater needs to turn
 * on needs to either be synchronous when Mobius sets it in the LoopState,
 * or it needs to latch until the UI gets a chance to respond and then
 * resets the latch.
 *
 * Since there is an undefined time between when Mobius sets loop->beatsubCycle
 * and when the UI gets around to painting, it can't keep setting or clearing
 * that flag on every interrupt. We could schedule a MainThread event for that
 * but that seems like over complicated.  The quick and dirty is to declare
 * that the LoopState beat indicators are only set by Mobius and only cleared
 * by us after we've responded.  The exception is when the loop resets
 * within Mobius.  So loop->beatSomething really means "on until you turn it off
 * or I turn it off for you".  How did it work before?  Simulator will make that
 * assumption for now.
 *
 */

#include <JuceHeader.h>

#include "../../util/Trace.h"
#include "../../model/UIConfig.h"
#include "../../model/MobiusState.h"
#include "../../model/ModeDefinition.h"

#include "Colors.h"
#include "StatusArea.h"
#include "BeatersElement.h"

/**
 * The expected interval between calls to updateState in milliseconds.
 * If that changes, you'll need to adjust that here.
 */
const int BeaterExpectedTickMsec = 100;

/**
 * The number of ticks to keep a beater lit, a multiple of ticks.
 * 
 * This generally does not need to change.  You want it slow enough
 * to be visible but fast enough that adjacent beats don't smear together
 * keeping the beater lit all the time.  That would only be the case
 * for extremely short loops in which case havoc is happening in the
 * UI anyway so it probably doesn't matter.
 *
 * If ticks come in every 100ms then a decay of 5 keeps the beater
 * lit for 1/2 second.
 */
const int BeaterDecay = 4;

//////////////////////////////////////////////////////////////////////
//
// Beaters
//
//////////////////////////////////////////////////////////////////////

BeatersElement::BeatersElement(StatusArea* area) :
    StatusElement(area, "BeatersElement")
{
    addAndMakeVisible(&subcycleBeater);
    addAndMakeVisible(&cycleBeater);
    addAndMakeVisible(&loopBeater);

    // we have child Beater components that are not StatusElements
    // so the default mouse listener doesn't work
    // see if this allows drag
    setInterceptsMouseClicks(true, false);
}

BeatersElement::~BeatersElement()
{
}

void BeatersElement::configure(UIConfig* config)
{
    // todo: could adjust the diameter
}

/**
 * Old default diameter was 20.  We've got three of them.
 * Under Juce 20 feels smaller.
 */
const int BeaterDiameter = 30;

int BeatersElement::getPreferredHeight()
{
    return BeaterDiameter;
}

int BeatersElement::getPreferredWidth()
{
    return (BeaterDiameter * 3);
}

void BeatersElement::resized()
{
    // how much air should we leave around these?
    // might want to give them some padding
    subcycleBeater.setBounds(0, 0, BeaterDiameter, BeaterDiameter);
    cycleBeater.setBounds(BeaterDiameter, 0, BeaterDiameter, BeaterDiameter);
    loopBeater.setBounds(BeaterDiameter * 2, 0, BeaterDiameter, BeaterDiameter);
}

/**
 * If we override paint, does that mean we control painting
 * the children, or is that going to cascade?
 */
void BeatersElement::paint(juce::Graphics& g)
{
    // borders, labels, etc.
    StatusElement::paint(g);

    if (loopBeater.decayCounter > 0) {
        if (cycleBeater.decayCounter == 0)
          trace("not painting cycle");
        if (subcycleBeater.decayCounter == 0)
          trace("not painting subcycle");
    }
    
    bool subcycleOn = (subcycleBeater.decayCounter > 0);
    bool cycleOn = (cycleBeater.decayCounter > 0);
    bool loopOn = (loopBeater.decayCounter > 0);

    //if (loopOn) cycleOn = true;
    //if (cycleOn) subcycleOn = true;

    subcycleBeater.paintBeater(g, subcycleOn);
    cycleBeater.paintBeater(g, cycleOn);
    loopBeater.paintBeater(g, loopOn);
}

/**
 * Thread concern!
 *
 * Unclear how painting is supposed to work in timer threads.
 * Example shows a timer using MessageManagerLock which we do
 * but I don't know if that means we can go ahead and paint or
 * if we're supposed to call repaint() which I guess schedules
 * a paint request in the main event loop.
 */
void BeatersElement::update(MobiusState* state)
{
    MobiusTrackState* track = &(state->tracks[state->activeTrack]);
    MobiusLoopState* loop = &(track->loops[track->activeLoop]);

    // how imporatnt is it to avoid redundant calls to repaint()?
    bool anyChanged = false;

    if (loop->beatSubCycle) {
        bool beatChanged = subcycleBeater.start();
        if (beatChanged) anyChanged = true;
        loop->beatSubCycle = false;
        
    }
    else {
        bool beatChanged = subcycleBeater.tick();
        if (beatChanged) anyChanged = true;
    }
    
    if (loop->beatCycle) {
        bool beatChanged = cycleBeater.start();
        if (beatChanged) anyChanged = true;
        loop->beatCycle = false;
        
    }
    else {
        bool beatChanged = cycleBeater.tick();
        if (beatChanged) anyChanged = true;
    }
    
    if (loop->beatLoop) {
        bool beatChanged = loopBeater.start();
        if (beatChanged) anyChanged = true;
        loop->beatLoop = false;
        
    }
    else {
        bool beatChanged = loopBeater.tick();
        if (beatChanged) anyChanged = true;
    }
        
    if (anyChanged)
      repaint();
}

/**
 * Update one of the beaters.
 * Returns true if state changed that requires a repaint
 */
bool BeatersElement::update(Beater* b, bool* hit)
{
    bool changed = false;
    
    if (*hit) {
        changed = b->start();
        // we've "consumed" the hit flag turn it off for the next boundary
        *hit = false;
    }
    else {
        changed = b->tick();
    }

    return changed;
}

//////////////////////////////////////////////////////////////////////
//
// Beater
//
//////////////////////////////////////////////////////////////////////

/**
 * Called by Beaters to turn us on.
 * Returns true if the graphics state changed.
 */
bool Beater::start()
{
    bool changed = false;
    if (decayCounter > 0) {
        // we're already on, ignore or extend?
        // decayCounter = BeaterDecay;
    } 
    else {
        // we're currently off
        decayCounter = BeaterDecay;
        changed = true;
    }
    return changed;
}

/**
 * Called by Beaters every tick.
 * Return true if the light inside us dies.
 */
bool Beater::tick()
{
    bool changed = false;
    
    if (decayCounter > 0) {
        decayCounter = decayCounter - 1;
        if (decayCounter == 0) {
            changed = true;
        }
    }
    return changed;
}

bool Beater::reset()
{
    bool changed = false;
    if (decayCounter > 0) {
        decayCounter = 0;
        changed = true;
    }
    return changed;
}

/**
 * Called by Beaters to let our little light shine.
 * I'm assuming that Juce won't call paint on subcomponents
 * if the parent overrides paint.
 */
void Beater::paintBeater(juce::Graphics& g, bool on)
{
    // who should own the border, us or Beaters?
    if (on) {
        // we're on
        // todo: keep it bordered
        g.setColour(juce::Colour(MobiusPink));
        g.fillEllipse((float)getX(), (float)getY(), (float)getWidth(), (float)getHeight());
    }
    else {
        g.setColour(juce::Colour(MobiusBlue));
        g.drawEllipse((float)getX(), (float)getY(), (float)getWidth(), (float)getHeight(), 2.0f);
    }
}

// forward mouse events to our parent
void Beater::mouseDown(const juce::MouseEvent& e)
{
    getParentComponent()->mouseDown(e);
}

void Beater::mouseDrag(const juce::MouseEvent& e)
{
    getParentComponent()->mouseDrag(e);
}

void Beater::mouseUp(const juce::MouseEvent& e)
{
    getParentComponent()->mouseUp(e);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
