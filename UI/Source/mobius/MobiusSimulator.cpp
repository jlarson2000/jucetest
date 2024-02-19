/**
 * A simulator of the Mobius engine for UI testing.
 */

#include "../util/Trace.h"
#include "../util/Util.h"

#include "../model/MobiusConfig.h"
#include "../model/MobiusState.h"
#include "../model/ModeDefinition.h"
#include "../model/FunctionDefinition.h"
#include "../model/Parameter.h"
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

void MobiusSimulator::setListener(MobiusListener* l)
{
    listener = l;
}

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
    else if (action->target == TargetParameter) {
        Parameter* p = action->targetPointer.parameter;
        if (p == nullptr) {
            trace("Unresolved parameter: %s\n", action->targetName);
        }
        else if (p == OutputLevelParameter) {
            int tracknum = action->scopeTrack;
            if (tracknum == 0) {
                tracknum = state.activeTrack;
            }
            else {
                tracknum--;
            }
            MobiusTrackState* track = &(state.tracks[tracknum]);
            track->outputLevel = action->getValueInt();
        }
        else {
            trace("Unimplemented parameter: %s\n", p->getName());
        }
            
    }
    else {
        trace("Unexpected action target %s\n", action->target->getName());
    }

    // until we support queuing make sure this is clean
    action->actionId = 0;
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
    loop->cycles = 0;
    loop->cycle = 0;
    loop->subcycle = 0;
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
        loop->subcycle = 0;
        
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
            play(loop, frames);
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Play
//
// Adapted from Loop::play, playLocal, notifyBeatListeners
// Conceptually we are filling the output buffer with content from
// from the play layer.  This is normally the most recent layer unless
// you have used Undo.
//
// We've simplified this by assuming that input and output latency are
// zero so the record frame (mFrame) and play frame (mPlayFrame) are the same.
// There is only one layer and it defines the loop size.
//
// If we cross a subcycle, cycle, or loop boundary within this buffer,
// we set the beat flags.
//
//////////////////////////////////////////////////////////////////////

void MobiusSimulator::play(MobiusLoopState* loop, int bufferFrames)
{
    // if we're in pause mode, do not advance
	if (loop->paused) {
		// mOutput->captureTail();
	}
    else {
        // number of frames in this loop/layer
        long loopTransitionFrame = loop->frames;
        // number of frames remaining in this loop
        long remaining = loopTransitionFrame - loop->frame;

        // do we have more remaining than the size of this buffer?
        if (remaining >= bufferFrames) {
            // bliss, no boundary conditions, play away
            // put this layer's frames into the output stream
            notifyBeatListeners(loop, bufferFrames);
            // mPlayFrame += frames;
            loop->frame += bufferFrames;
        }
        else if (remaining >= 0) {
            // remaining loop frames is contained within this buffer
            // play whatever is left in this layer
            long remainder = bufferFrames - remaining;

            // subtlty of why we're here with >=0 but only play/notify if >0
            if (remaining > 0) {
                // play remaining
                notifyBeatListeners(loop, remaining);
                loop->frame += remaining;
            }

            // tests for various modes and sets the play frame, we can skip
            // now we "loop"
            loop->frame = 0;

			notifyBeatListeners(loop, remainder);
			loop->frame += remainder;
        }
        else if (remaining < 0) {
            // play frame beyond the end of the loop, error
            // supposed to Reset
        }
    }
}

/**
 * Detect when segment of the audio buffer includes one of the beats.
 * Usually frames will be the full buffer frames until we reach a loop boundary
 * and do remainder processing above.
 */
void MobiusSimulator::notifyBeatListeners(MobiusLoopState* loop, long bufferFrames)
{
	// Don't do this in insert mode since we're never really
	// returning to the loop start?

    long loopFrames = loop->frames;
    bool notify = false;
    
    if (loopFrames == 0) {
		// how can this happen?
		trace("Loop: notifyBeatListeners: zero length loop\n");
	}
	else {
		int cycles = loop->cycles;
		long firstLoopFrame = 0;

        // this is the user's percieved play frame
        //long playFrame = mPlayFrame - mOutput->latency;
        long playFrame = loop->frame;

        // will never be true without latency
		if (playFrame < 0)
          playFrame += loop->frames;

        // is firstLoopFrame within this window?
        // I don't understand the math, firstLoopFrame is never
        // set and will always be zero, bug?
        long delta = playFrame - firstLoopFrame;

        if (delta < bufferFrames) {
            loop->beatLoop = true;
            // cycle and subcycle reset
            loop->cycle = 0;
            loop->subcycle = 0;
            notify = true;
        }

		long lastBufferFrame = playFrame + bufferFrames - 1;

		long cycleFrames = loopFrames;
		if (cycles > 1) {
			cycleFrames = loopFrames / cycles;
			int delta = lastBufferFrame % cycleFrames;
			// delta is now the number of frames beyond a cycle point
			// if we get a negative number subtracting the frame count,
			// then this block of frames surrounded a cycle boundary
			if (delta - bufferFrames <= 0) {
                loop->beatCycle = true;
                loop->cycle = loop->frame / cycleFrames;
                loop->subcycle = 0;
                notify = true;
            }
		}

		// similar calculation as for cycles, is there a faster way?
		// !! this is the same roundoff problem that 
		// getQuantizedFrame has
		// int ticks = mPreset->getSubcycles();
        int ticks = 4;
        
		// sanity check to avoid divide by zero
		if (ticks == 0) ticks = 1;
		long tickFrames = cycleFrames / ticks;
		delta = lastBufferFrame % tickFrames;
		if (delta - bufferFrames <= 0) {
            loop->beatSubCycle = true;
            // todo: maintain loop->subcycle
            notify = true;
        }

        // if we set any of the flags use send the UI
        // a signal to break out of it's wait loop
	}

    if (notify && listener != nullptr)
      listener->MobiusTimeBoundary();
    
}

//////////////////////////////////////////////////////////////////////
//
// Maintenance
//
//////////////////////////////////////////////////////////////////////

void MobiusSimulator::performMaintenance()
{
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

