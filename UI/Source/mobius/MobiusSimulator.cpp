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
#include "../model/Setup.h"
#include "../model/Preset.h"
#include "../model/UIAction.h"
#include "../model/XmlRenderer.h"
#include "../model/UIEventType.h"

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

/**
 * This always makes an internal copy of the passed object.
 * It is the responsibility of the caller to either free it
 * or keep using it.
 */
void MobiusSimulator::configure(MobiusConfig* config)
{
    // clone it so we can make internal modifications
    // since we can be called after config editing delete the existing one
    delete configuration;
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
 * Sigh, I decided not ot have static FunctionDefinition pointers
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
        else if (StringEqual(f->getName(), "Record")) {
            doRecord(action);
        }
        else if (StringEqual(f->getName(), "NextLoop")) {
            doSwitch(action, 1);
        }
        else if (StringEqual(f->getName(), "PrevLoop")) {
            doSwitch(action, -1);
        }
        else if (StringEqual(f->getName(), "SelectLoop")) {
            int lnum = action->getValueInt();
            // kludge, 1 means "increment"
            doSwitch(action, lnum + 2);
        }
        else if (StringEqual(f->getName(), "NextTrack")) {
            // no track selection options yet, just go there
            int next = state.activeTrack + 1;
            int max = configuration->getTracks();
            if (next >= max)
              next = 0; // wrap or just stay there?
            state.activeTrack = next;
        }
        else if (StringEqual(f->getName(), "PrevTrack")) {
            int next = state.activeTrack - 1;
            if (next < 0) {
                // wrap or stay at zero?
                next = configuration->getTracks() - 1;
            }
            state.activeTrack = next;
        }
        else if (StringEqual(f->getName(), "SelectTrack")) {
            int next = action->getValueInt();
            if (next >= 0 && next < configuration->getTracks())
              state.activeTrack  = next;
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

MobiusTrackState* MobiusSimulator::getTargetTrack(UIAction* action)
{
    int tracknum = action->scopeTrack;
    if (tracknum == 0) {
        // zero scope means active
        tracknum = state.activeTrack;
    }
    else {
        // otherwise it is a track number 1 based
        tracknum--;
    }
    return &(state.tracks[tracknum]);
}

void MobiusSimulator::doSwitch(UIAction* action, int next)
{
    MobiusTrackState* track = getTargetTrack(action);
    // todo: simulate quantization with an event

    if (next < 0) {
        // prev, I think for these don't wrap
        int current = track->activeLoop;
        if (current > 0) {
            track->activeLoop = current - 1;
        }
    }
    else if (next == 1) {
        // next
        int current = track->activeLoop;
        next = current + 1;
        if (next < configuration->getMaxLoops()) {
            track->activeLoop = next;
        }
    }
    else {
        // it is SelectLoop with number starting from 2
        next -=2;
        if (next >= 0 && next < (configuration->getMaxLoops() - 1))
          track->activeLoop = next;
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
    loop->init();
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

        simulateEvents();
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

/**
 * Put an interesting collection of events in the
 * first loop of the first track.  These won't be consumed
 * and will stay there forever but enough for testing
 * event display
 *
 * For quantized events, we're hard coding the initial loop to
 * be 2 cycles long with 4 subcycles.  The actual number of frames
 * will depend on how long you waited when pressing Record.
 *
 * Event fields are
 *   type/function
 *   arguments, frame, pending
 */
void MobiusSimulator::simulateEvents()
{
    MobiusTrackState* track = &(state.tracks[state.activeTrack]);
    MobiusLoopState* loop = &(track->loops[track->activeLoop]);

    // Overdub start/end at quant points 1 and 2
    // test whether we can verify that that they belong
    // together and show them in start/end colors

    simulateEvent(loop, OverdubEventType, 1);
    simulateEvent(loop, OverdubEventType, 2);

    // somethign with an isEnd
    simulateEvent(loop, MultiplyEndEventType, 3);

    // stacked events
    simulateEvent(loop, InsertEventType, 4);
    simulateEvent(loop, SpeedEventType, 4);
    
    // close to the end
    MobiusEventState* ev = simulateEvent(loop, ScriptEventType, 0);
    if (ev != nullptr) {
        ev->frame = loop->frames - 100;
    
        // pending
        ev = simulateEvent(loop, SwitchEventType, 0);
        ev->pending = true;
        ev->argument = 2;
    }
}

MobiusEventState* MobiusSimulator::simulateEvent(MobiusLoopState* loop, UIEventType* type, int q)
{
    MobiusEventState* ev = nullptr;
    if (loop->eventCount < MaxEvents) {
        ev = &(loop->events[loop->eventCount]);
        loop->eventCount++;

        long frames = loop->frames;
        long cycleFrames = frames / 2;
        long subFrames = cycleFrames / 4;

        ev->type = type;
        ev->frame = subFrames * q;
    }
    
    return ev;
}

//////////////////////////////////////////////////////////////////////
//
// Maintenance
//
//////////////////////////////////////////////////////////////////////

void MobiusSimulator::performMaintenance()
{
}

//////////////////////////////////////////////////////////////////////
//
// Parameters
//
//////////////////////////////////////////////////////////////////////

/**
 * Locate the Setup that would be in current use.
 */
Setup* MobiusSimulator::getActiveSetup()
{
    // unclear how we represented which Setup to use if we had more than
    // one, just pick the first
    Setup* setup = configuration->getSetups();
    if (setup == nullptr)
      trace("No active setup in global configuration\n");
    return setup;
}

SetupTrack* MobiusSimulator::getSetupTrack(int tracknum)
{
    SetupTrack* track = nullptr;

    Setup* setup = getActiveSetup();
    if (setup != nullptr) {
        track = setup->getTrack(tracknum);
        if (track == nullptr) {
            trace("No track in the Setup with number %d\n", tracknum);
        }
    }
    return track;
}

Preset* MobiusSimulator::getTrackPreset(SetupTrack* track)
{
    Preset* preset = nullptr;

    // kludge: make sure ordinals are assigned
    int ordinal = 0;
    for (Preset* p = configuration->getPresets() ; p != nullptr ; p = p->getNext()) {
        p->ordinal = ordinal;
        ordinal++;
    }
    
    const char* pname = track->getPreset();
    if (pname == nullptr) {
        // not uncommon, auto-select the first one
        // is there a configuration for the default preset?
        preset = configuration->getPresets();
    }
    else {
        // find the named Preset from the global config
        for (Preset* p = configuration->getPresets() ; p != nullptr ; p = p->getNext()) {
            if (StringEqual(pname, p->getName())) {
                preset = p;
                break;
            }
        }

        if (preset == nullptr)
          trace("Preset not found %s\n", pname);
    }
    
    return preset;
}

Preset* MobiusSimulator::getTrackPreset(int tracknum)
{
    Preset* preset = nullptr;

    SetupTrack* track = getSetupTrack(tracknum);
    if (track != nullptr)
      preset = getTrackPreset(track);

    return preset;
}

/**
 * Return the runtime value of a parameter as a normalized integer.
 * This can only access Parameters that have been declared as exportable.
 * Not checking for that yet.
 * Need a return value to indiciate "supported"?
 */
int MobiusSimulator::getParameter(Parameter* p, int tracknum)
{
    int value = 0;

    if (p == TrackPresetParameter) {
        // this is the only TYPE_STRING supported, convert to ordinal
        SetupTrack* track = getSetupTrack(tracknum);
        if (track != nullptr) {
            Preset* preset = getTrackPreset(track);
            if (preset != nullptr)
              value = preset->ordinal;
        }
    }
    else if (p->type == TYPE_STRING) {
        trace("Unable to export parameter %s\n", p->getName());
    }
    else if (p->scope == PARAM_SCOPE_GLOBAL) {
        // few interesting things
        value = p->getOrdinalValue(configuration);
    }
    else if (p->scope == PARAM_SCOPE_PRESET) {
        // this means a Preset parameter in the given track
        Preset* preset = getTrackPreset(tracknum);
        if (preset != nullptr) {
            value = p->getOrdinalValue(preset);
        }
    }
    else if (p->scope == PARAM_SCOPE_SETUP) {
        Setup* setup = getActiveSetup();
        if (setup != nullptr) {
            value = p->getOrdinalValue(setup);
        }
    }
    else if (p->scope == PARAM_SCOPE_TRACK) {
        SetupTrack* track = getSetupTrack(tracknum);
        if (track != nullptr) {
            value = p->getOrdinalValue(track);
        }
    }

    return value;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

