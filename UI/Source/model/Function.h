/* 
 * Model for function definitions.
 *
 * Functions are commands that can be sent to the engine.
 * They differ from Parameters in that they do not have values
 * and cannot be configured.
 *
 * TODO: Check VST3 and see if plugins have a similar concept that
 * could be used these days.
 *
 * Unlike Parameter, this has been substantially simplified from the old
 * Mobius model which was almost entirely related to runtime operation.
 * There will be a parallel set of InternalFunction
 * definitions that hold those.  This keeps the public interface clean
 * and free of internal class references.
 */

#pragma once

#include <vector>

#include "SystemConstant.h"

//////////////////////////////////////////////////////////////////////
//
// Function Class
//
//////////////////////////////////////////////////////////////////////

class Function : public SystemConstant {

  public:
    
	Function(const char* name);
    virtual ~Function();

    int ordinal;				// internal number for indexing

    // bring things over from InternalFunction as necessary for bindings

    //////////////////////////////////////////////////////////////////////
    // Global Function Registry
    //////////////////////////////////////////////////////////////////////

    static std::vector<Function*> Functions;
    static void dumpFunctions();
	static Function* getFunction(const char* name);

  private:

};

//////////////////////////////////////////////////////////////////////
//
// Replicated Function
//
// Extenion used by functions that support a numeric multiplier.
// Some functions have both a set of relative and absolute functions
// so we multiply only when the replicated flag is on.
//
// This is not used yet, but may still be relevant.
// It was used by a handful of functions that can target things that
// have a configurable size.  Exammples were:
//
//    Select Track
//    Select Loop
//    Play Sample
//    Run Script
//
// For track selection, since there are a variable number of tracks
// there is no one function that says "select track 2".  We could (and did)
// hard code a default set, but doesn't work for samples and scripts which
// are completely random.  These are handled now with a single function
// that takes an argument.  Eventually might work out a way to bind them
// without the user needing to specify an argument.  But since this is really
// just syntactic sugar in the UI it doesn't need to be in the engine.
// 
//////////////////////////////////////////////////////////////////////

#if 0
class ReplicatedFunction : public Function {
  public:
	ReplicatedFunction();
  protected:
	bool replicated;
	char fullName[32];
};

/**
 * Constant for RunScriptFunction.
 */
#define MAX_SCRIPT_NAME 1024

/**
 * This is the only specific function class that we define globally
 * because Function needs it to create Function wrappers for loaded
 * scripts.
 */
class RunScriptFunction : public Function {
  public:
	RunScriptFunction(class Script* s);
	void invoke(class Action* action, class Mobius* m);
	bool isMatch(const char* xname);
  private:
	// we have to maintain copies of these since the strings the
	// Script return can be reclained after an autoload
	char mScriptName[MAX_SCRIPT_NAME];
};
#endif

/****************************************************************************
 *                                                                          *
 *                             FUNCTION CONSTANTS                           *
 *                                                                          *
 ****************************************************************************/

extern Function* AutoRecord;
extern Function* Backward;
extern Function* Bounce;
extern Function* Breakpoint;
extern Function* Checkpoint;
extern Function* Clear;
extern Function* Confirm;
extern Function* Coverage;
extern Function* Debug;
extern Function* DebugStatus;
extern Function* Divide;
extern Function* Divide3;
extern Function* Divide4;
extern Function* Drift;
extern Function* DriftCorrect;
extern Function* FocusLock;
extern Function* Forward;
extern Function* GlobalMute;
extern Function* GlobalPause;
extern Function* GlobalReset;
extern Function* Halfspeed;
extern Function* Ignore;
extern Function* InitCoverage;
extern Function* Insert;
extern Function* InstantMultiply;
// these are similar to replicated functions but have been in use
// for a long time, think about this
extern Function* InstantMultiply3;
extern Function* InstantMultiply4;
extern Function* LongUndo;
// Formerly LoopN, Loop1, Loop2, etc.
extern Function* SelectLoop;
extern Function* MidiOut;
extern Function* MidiStart;
extern Function* MidiStop;
// what was this?
extern Function* MyMove;
extern Function* Multiply;
extern Function* Mute;
extern Function* MuteOff;
extern Function* MuteOn;
extern Function* MuteRealign;
extern Function* MuteMidiStart;
extern Function* NextLoop;
extern Function* NextTrack;
extern Function* Overdub;
extern Function* OverdubOff;
extern Function* OverdubOn;
extern Function* Pause;
extern Function* PitchBend;
extern Function* PitchDown;
extern Function* PitchNext;
extern Function* PitchCancel;
extern Function* PitchOctave;
extern Function* PitchPrev;
extern Function* PitchStep;
extern Function* PitchUp;
extern Function* Play;
extern Function* PrevLoop;
extern Function* PrevTrack;
extern Function* Realign;
extern Function* Record;
extern Function* Redo;
extern Function* Rehearse;
extern Function* ReloadScripts;
extern Function* Replace;
extern Function* Reset;
extern Function* Restart;
extern Function* RestartOnce;
extern Function* ResumeScript;
extern Function* Reverse;
// this was formerly SampleN, Sample1, etc.
extern Function* PlaySample;
extern Function* SaveCapture;
extern Function* SaveLoop;
extern Function* Shuffle;
extern Function* ShortUndo;
extern Function* Slip;
extern Function* SlipForward;
extern Function* SlipBackward;
extern Function* Solo;
extern Function* SpeedDown;
extern Function* SpeedNext;
extern Function* SpeedCancel;
extern Function* SpeedPrev;
extern Function* SpeedShift;
extern Function* SpeedOctave;
extern Function* SpeedStep;
extern Function* SpeedBend;
extern Function* SpeedUp;
extern Function* SpeedToggle;
extern Function* StartCapture;
extern Function* StartPoint;
extern Function* StopCapture;
extern Function* Stutter;
extern Function* Substitute;
extern Function* Surface;
// don't really like needing SUS variants for these
// try to just have the base Function with canSustain set
// and make it nice in the binding UI
extern Function* SUSInsert;
extern Function* SUSMultiply;
extern Function* SUSMute;
extern Function* SUSMuteRestart;
extern Function* SUSNextLoop;
extern Function* SUSOverdub;
extern Function* SUSPrevLoop;
extern Function* SUSRecord;
extern Function* SUSRehearse;
extern Function* SUSReplace;
extern Function* SUSReverse;
extern Function* SUSSpeedToggle;
extern Function* SUSStutter;
extern Function* SUSSubstitute;
extern Function* SUSUnroundedInsert;
extern Function* SUSUnroundedMultiply;
extern Function* SyncMaster;
extern Function* SyncMasterTrack;
extern Function* SyncMasterMidi;
extern Function* SyncStartPoint;
extern Function* TimeStretch;
// Formerly TrackN, Track1, etc.
extern Function* SelectTrack;
extern Function* TrackCopy;
extern Function* TrackCopyTiming;
extern Function* TrackGroup;
extern Function* TrackReset;
extern Function* TrimEnd;
extern Function* TrimStart;
extern Function* UIRedraw;
extern Function* Undo;
extern Function* UndoOnly;
extern Function* WindowBackward;
extern Function* WindowForward;
extern Function* WindowStartBackward;
extern Function* WindowStartForward;
extern Function* WindowEndBackward;
extern Function* WindowEndForward;
extern Function* WindowMove;
extern Function* WindowResize;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
