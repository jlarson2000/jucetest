/**
 * A simulator of the Mobius engine for UI testing.
 */

#include "../util/Trace.h"
#include "../util/Util.h"

#include "../model/MobiusConfig.h"
#include "../model/MobiusState.h"
#include "../model/ModeDefinition.h"
#include "../model/FunctionDefinition.h"
#include "../model/UIAction.h"
#include "../model/XmlREnderer.h"

#include "MobiusSimulator.h"

MobiusSimulator::MobiusSimulator()
{
    // this is given to us later
    configuration = nullptr;
}

MobiusSimulator::~MobiusSimulator()
{
    delete configuration;
}

//////////////////////////////////////////////////////////////////////
//
// Configuration
//
//////////////////////////////////////////////////////////////////////

void MobiusSimulator::configure(MobiusConfig* config)
{
    // clone it so we can make internal modifications
    XmlRenderer xr;
    configuration = xr.clone(config);

    state.init();

    // supposed to pull this from config
    state.trackCount = 8;
    state.activeTrack = 0;

    for (int t = 0 ; t < state.trackCount ; t++) {
        MobiusTrackState* track = &(state.tracks[t]);
        track->loopCount = 4;
        track->activeLoop = 0;
    }

    // don't need to support config update while active yet
    globalReset();
}

//////////////////////////////////////////////////////////////////////
//
// State
//
//////////////////////////////////////////////////////////////////////

MobiusState* MobiusSimulator::getState()
{
    return &state;
}

//////////////////////////////////////////////////////////////////////
//
// Actions
//
//////////////////////////////////////////////////////////////////////

/**
 * Perform an action.
 * In the old interface, ownership of the Action is taken
 * so we can resolve and intern it.
 * Think more about this.  UI wants to hold copies of partially
 * resolved Actions
 *
 * Sign, I decided not ot have static FunctionDefinition pointers
 * to avoid conflicts with the ones the engine uses.  Mostly won't need
 * these except here where we have to compare by name instead.  If this
 * starts happening often will have to add them.
 */
void MobiusSimulator::doAction(UIAction* action)
{
    if (action->target == TargetFunction) {
        FunctionDefinition* f = action->targetPointer.function;
        if (f == nullptr) {
            trace("Unresolved function: %s\n", action->targetName);
        }
        else if (StringEqual(f->getName(), "GlobalReset")) {
            globalReset();
        }
        else if (StringEqual(f->getName(), "Reset")) {
            doReset(action);
        }
        else if (StringEqual(f->getName(), "Record")) {
            doRecord(action);
        }
        else {
            trace("Unimplemented function: %s\n", f->getName());
        }
    }
    else {
        trace("Unexpected action target %s\n", action->target->getName());
    }
}

//////////////////////////////////////////////////////////////////////
//
// Tests
//
// Just a hook to force the engine to do something that isn't
// defined by any Actions
//
// Used early on and can delete eventually
//
//////////////////////////////////////////////////////////////////////

void MobiusSimulator::test()
{
    trace("Running a test...\n");
}

//////////////////////////////////////////////////////////////////////
//
// Reset
//
//////////////////////////////////////////////////////////////////////

/**
 * Put all tracks and loops into Reset
 * Don't need to be influenced by the action
 */
void MobiusSimulator::globalReset()
{
    for (int t = 0 ; t < state.trackCount ; t++) {
        MobiusTrackState* track = &(state.tracks[t]);
        for (int l = 0 ; l < track->loopCount ; l++) {
            MobiusLoopState* loop = &(track->loops[l]);
            reset(loop);
        }
        track->activeLoop = 0;
    }
    state.activeTrack = 0;
}

/**
 * This should obey scope
 */ 
void MobiusSimulator::doReset(UIAction* action)
{
    MobiusTrackState* track = &(state.tracks[state.activeTrack]);
    MobiusLoopState* loop = &(track->loops[track->activeLoop]);
    reset(loop);
}

void MobiusSimulator::reset(MobiusLoopState* loop)
{
    loop->mode = ResetMode;
    loop->frames = 0;
    loop->frame = 0;
}

//////////////////////////////////////////////////////////////////////
//
// Record
//
//////////////////////////////////////////////////////////////////////

/**
 * Does scope matter for Record?
 * Always record into the active loop.
 */
void MobiusSimulator::doRecord(UIAction* action)
{
    MobiusTrackState* track = &(state.tracks[state.activeTrack]);
    MobiusLoopState* loop = &(track->loops[track->activeLoop]);
    ModeDefinition* mode = loop->mode;

    if (mode == RecordMode) {
        // end the recording
        loop->mode = PlayMode;
        loop->frame = 0;

        // for beater testing pretend we have 2 cycles
        loop->cycles = 2;
        loop->cycle = 0;

        // can start the UI beaters
        loop->beatLoop = true;
        loop->beatCycle = true;
        loop->beatSubCycle = true;
    }
    else {
        reset(loop);
        loop->mode = RecordMode;
    }
}

//////////////////////////////////////////////////////////////////////
//
// Interrupt
//
//////////////////////////////////////////////////////////////////////


/**
 * Simulate the processing of an audio interrupt
 *
 * hmm, the UI doesn't care about ModeDefinition poitner constants
 * but the engine does so may as well define them?
 */
void MobiusSimulator::simulateInterrupt(float* input, float* output, int frames)
{
    for (int t = 0 ; t < state.trackCount ; t++) {
        MobiusTrackState* track = &(state.tracks[t]);
        MobiusLoopState* loop = &(track->loops[track->activeLoop]);
        ModeDefinition* mode = loop->mode;

        if (mode == RecordMode) {
            // track is recording
            // technically we don't update loop frames until the recording ends
            // but we don't have anywhere else to store it
            loop->frames += frames;
            loop->frame += frames;
        }
        else if (loop->frames > 0) {
            // advance the play position
            int newFrame = loop->frame + frames;

            // should be getting subcycles from Preset, but assume 4 for now
            int subcycles = 4;
            int cycleFrames = loop->frames / loop->cycles;
            int subcycleFrames = cycleFrames / subcycles;
            
            int startSubcycle = (int)((float)(loop->frame) / (float)subcycleFrames);
            int endSubcycle = (int)(newFrame / subcycleFrames);
            if (endSubcycle > startSubcycle)
              loop->beatSubCycle = true;

            int startCycle = (int)((float)(loop->frame) / (float)cycleFrames);
            int endCycle = (int)(newFrame / cycleFrames);
            if (endCycle > startCycle)
              loop->beatCycle = true;

            if (newFrame > loop->frames) {
                // we loop!
                loop->frame = newFrame - loop->frames;
                loop->beatCycle = true;
                loop->beatLoop = true;
            }
            else {
                loop->frame = newFrame;
            }

            // weird rounding?
            if (loop->beatLoop) {
                loop->beatCycle = true;
                loop->beatSubCycle = true;
            }
            else if (loop->beatCycle) {
                loop->beatSubCycle = true;
            }
            
        }
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

