/*
 * Object capturing the runtime state of the Mobius engine.
 */

#include "MobiusState.h"

void MobiusLayerState::init()
{
    checkpoint = false;
}

void MobiusEventState::init()
{
    type = nullptr;
    function = nullptr;
    frame = 0;
    argument = 0;
}

void MobiusLoopState::init()
{
    number = 0;
    mode = nullptr;
    recording = false;
    paused = false;
    frame = 0;
    cycle = 0;
    cycles = 0;
    frames = 0;
    nextLoop = 0;
    returnLoop = 0;
    overdub = false;
    mute = false;
    beatLoop = false;
    beatCycle = false;
    beatSubCycle = false;
    windowOffset = 0;
    historyFrames = 0;

    active = false;
    pending = false;
    reverse = false;
    speed = false;
    pitch = false;
    mute = false;

    eventCount = 0;
    for (int i = 0 ; i < MaxEvents ; i++)
      events[i].init();
    
    layerCount = 0;
    lostLayers = 0;
    for (int i = 0 ; i < MaxLayers ; i++)
      layers[i].init();
    
    redoCount = 0;
    lostRedo = 0;
    for (int i = 0 ; i < MaxRedoLayers ; i++)
      redoLayers[i].init();
};

/**
 * State for one track.
 */
void  MobiusTrackState::init()
{
	number = 0;
	preset = 0;
	loopCount = 0;  // old initialized this to 1
	inputMonitorLevel = 0;
	outputMonitorLevel = 0;
	inputLevel = 0;
	outputLevel = 0;
	feedback = 0;
	altFeedback = 0;
	pan = 0;
    speedToggle = 0;
    speedOctave = 0;
	speedStep = 0;
	speedBend = 0;
    pitchOctave = 0;
    pitchStep = 0;
    pitchBend = 0;
    timeStretch = 0;
	reverse = false;
	focusLock = false;
    solo = false;
    globalMute = false;
    globalPause = false;
	group = 0;

    // SyncSource enumeration, pick a value
    syncSource = SYNC_DEFAULT;
    
    // SyncUnit enumeration, pick a value
    syncUnit = SYNC_UNIT_BEAT;
    
    tempo = 0.0f;
	beat = 0;
	bar = 0;
	outSyncMaster = false;
	trackSyncMaster = false;

    activeLoop = 0;
    for (int i = 0 ; i < MaxLoops ; i++)
      loops[i].init();
};

void MobiusState::init()
{
    globalRecording = false;
    activeTrack = 0;
    trackCount = 0;
    for (int i = 0 ; i < MaxTracks ; i++)
      tracks[i].init();
};

//////////////////////////////////////////////////////////////////////
//
// Simulation
//
//////////////////////////////////////////////////////////////////////

/**
 * Mock up a MobiusState with interesting data for UI testing.
 */
void MobiusState::simulate(MobiusState* state)
{
    state->init();
    state->trackCount = 8;
    state->activeTrack = 1;

    for (int t = 0 ; t < state->trackCount ; t++) {
        MobiusTrackState& track = state->tracks[t];
        track.number = t;
        track.preset = 0;
        track.loopCount = 4;
        track.inputMonitorLevel = 127; // what's the range here?
        track.outputMonitorLevel = 127;
        track.inputLevel = 127;
        track.outputLevel = 127;
        track.feedback = 127;
        track.altFeedback = 127;
        track.pan = 64;
        track.speedToggle = 0;
        track.speedOctave = 0;
        track.speedStep = 0;
        track.speedBend = 0;
        track.pitchOctave = 0;
        track.pitchStep = 0;
        track.pitchBend = 0;
        track.timeStretch = 0;
        track.reverse = false;
        track.focusLock = false;
        track.solo = false;
        track.globalMute = false;
        track.globalPause = false;
        track.group = 0;

        track.syncSource = SYNC_TRACK;
        track.syncUnit = SYNC_UNIT_BAR;
        track.tempo = 120.0f;
        track.beat = 2;
        track.bar = 3;
        track.outSyncMaster = false;
        track.trackSyncMaster = true;

        track.activeLoop = 0;

    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

    