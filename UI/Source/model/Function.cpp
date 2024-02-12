/*
 * Model for function definitions.
 *  
 */

#include <vector>

#include "../util/Trace.h"
#include "../util/Util.h"

#include "SystemConstant.h"
#include "Function.h"

//////////////////////////////////////////////////////////////////////
//
// Global Registry
//
//////////////////////////////////////////////////////////////////////

/**
 * A registry of all Functions, created as they are constructed.
 * This is primarily for binding where we need to associate things
 * dynamically with any parameter identified by name.  Engine
 * code rarely needs these.
 *
 * The vector will be built out by the Function constructor.
 * Normally all Function objects will be static objects.
 */
std::vector<Function*> Function::Functions;

void Function::dumpFunctions()
{
    for (int i = 0 ; i < Functions.size() ; i++) {
        Function* f = Functions[i];
        Trace(1, "Function %s\n", f->getName());
    }
}

/**
 * Find a Function by name
 * This doesn't happen often so we can do a liner search.
 */
Function* Function::getFunction(const char* name)
{
	Function* found = nullptr;
	
	for (int i = 0 ; i < Functions.size() ; i++) {
		Function* f = Functions[i];
		if (StringEqualNoCase(f->getName(), name)) {
            found = f;
            break;
        }
	}
	return found;
}

//////////////////////////////////////////////////////////////////////
//
// Base Class
//
//////////////////////////////////////////////////////////////////////


Function::Function(const char* name) :
    SystemConstant(name, nullptr)
{
    // add to the global registry
    ordinal = Functions.size();
    Functions.push_back(this);

    // todo, will want to flag deprecated functions
    // or better yet, just leave them out
}

Function::~Function()
{
}

//////////////////////////////////////////////////////////////////////
//
// Function Definition Objects
//
// Unlike Parameter objects, we didn't keep these in a single file,
// they were strewn about the code in files with other things related to
// the implementation of that function.
//
// Since there is almost no implementation in these we don't need to
// subclass them and can just make static objects direcdtly from the base class.
//
//////////////////////////////////////////////////////////////////////

// Do we even need subclasses here, could do it all with member initializer lists?
// Since the UI doesn't need direct access to the pointers, we could ski
// the second step of assigning the static object?

Function AutoRecordObj {"AutoRecord"};
Function* AutoRecord = &AutoRecordObj;

Function BackwardObj {"Backward"};
Function* Backward = &BackwardObj;

Function BounceObj {"Bounce"};
Function* Bounce = &BounceObj;

Function BreakpointObj {"Breakpoint"};
Function* Breakpoint = &BreakpointObj;

Function CheckpointObj {"Checkpoint"};
Function* Checkpoint = &CheckpointObj;

Function ClearObj {"Clear"};
Function* Clear = &ClearObj;

Function ConfirmObj {"Confirm"};
Function* Confirm = &ConfirmObj;

Function CoverageObj {"Coverage"};
Function* Coverage = &CoverageObj;

Function DebugObj {"Debug"};
Function* Debug = &DebugObj;

Function DebugStatusObj {"DebugStatus"};
Function* DebugStatus = &DebugStatusObj;

Function DivideObj {"Divide"};
Function* Divide = &DivideObj;

Function Divide3Obj {"Divide3"};
Function* Divide3 = &Divide3Obj;

Function Divide4Obj {"Divide4"};
Function* Divide4 = &Divide4Obj;

Function DriftObj {"Drift"};
Function* Drift = &DriftObj;

Function DriftCorrectObj {"DriftCorrect"};
Function* DriftCorrect = &DriftCorrectObj;

Function FocusLockObj {"FocusLock"};
Function* FocusLock = &FocusLockObj;

Function ForwardObj {"Forward"};
Function* Forward = &ForwardObj;

Function GlobalMuteObj {"GlobalMute"};
Function* GlobalMute = &GlobalMuteObj;

Function GlobalPauseObj {"GlobalPause"};
Function* GlobalPause = &GlobalPauseObj;

Function GlobalResetObj {"GlobalReset"};
Function* GlobalReset = &GlobalResetObj;

Function HalfspeedObj {"Halfspeed"};
Function* Halfspeed = &HalfspeedObj;

Function IgnoreObj {"Ignore"};
Function* Ignore = &IgnoreObj;

Function InitCoverageObj {"InitCoverage"};
Function* InitCoverage = &InitCoverageObj;

Function InsertObj {"Insert"};
Function* Insert = &InsertObj;

Function InstantMultiplyObj {"InstantMultiply"};
Function* InstantMultiply = &InstantMultiplyObj;

// these are similar to replicated functions but have been in use
// for a long time, think about this
Function InstantMultiply3Obj {"InstantMultiply3"};
Function* InstantMultiply3 = &InstantMultiply3Obj;
Function InstantMultiply4Obj {"InstantMultiply4"};
Function* InstantMultiply4 = &InstantMultiply4Obj;

Function LongUndoObj {"LongUndo"};
Function* LongUndo = &LongUndoObj;

// Formerly LoopN, Loop1, Loop2, etc.
Function SelectLoopObj {"SelectLoop"};
Function* SelectLoop = &SelectLoopObj;

Function MidiOutObj {"MidiOut"};
Function* MidiOut = &MidiOutObj;

Function MidiStartObj {"MidiStart"};
Function* MidiStart = &MidiStartObj;

Function MidiStopObj {"MidiStop"};
Function* MidiStop = &MidiStopObj;

// what was this?
//Function MyMoveObj {"MyMove"};
//Function* MyMove = &MyMoveObj;

Function MultiplyObj {"Multiply"};
Function* Multiply = &MultiplyObj;

Function MuteObj {"Mute"};
Function* Mute = &MuteObj;

Function MuteOffObj {"MuteOff"};
Function* MuteOff = &MuteOffObj;

Function MuteOnObj {"MuteOn"};
Function* MuteOn = &MuteOnObj;

Function MuteRealignObj {"MuteRealign"};
Function* MuteRealign = &MuteRealignObj;

Function MuteMidiStartObj {"MuteMidiStart"};
Function* MuteMidiStart = &MuteMidiStartObj;

Function NextLoopObj {"NextLoop"};
Function* NextLoop = &NextLoopObj;

Function NextTrackObj {"NextTrack"};
Function* NextTrack = &NextTrackObj;

Function OverdubObj {"Overdub"};
Function* Overdub = &OverdubObj;

Function OverdubOffObj {"OverdubOff"};
Function* OverdubOff = &OverdubOffObj;

Function OverdubOnObj {"OverdubOn"};
Function* OverdubOn = &OverdubOnObj;

Function PauseObj {"Pause"};
Function* Pause = &PauseObj;

Function PitchBendObj {"PitchBend"};
Function* PitchBend = &PitchBendObj;

Function PitchDownObj {"PitchDown"};
Function* PitchDown = &PitchDownObj;

Function PitchNextObj {"PitchNext"};
Function* PitchNext = &PitchNextObj;

Function PitchCancelObj {"PitchCancel"};
Function* PitchCancel = &PitchCancelObj;

Function PitchOctaveObj {"PitchOctave"};
Function* PitchOctave = &PitchOctaveObj;

Function PitchPrevObj {"PitchPrev"};
Function* PitchPrev = &PitchPrevObj;

Function PitchStepObj {"PitchStep"};
Function* PitchStep = &PitchStepObj;

Function PitchUpObj {"PitchUp"};
Function* PitchUp = &PitchUpObj;

Function PlayObj {"Play"};
Function* Play = &PlayObj;

Function PrevLoopObj {"PrevLoop"};
Function* PrevLoop = &PrevLoopObj;

Function PrevTrackObj {"PrevTrack"};
Function* PrevTrack = &PrevTrackObj;

Function RealignObj {"Realign"};
Function* Realign = &RealignObj;

Function RecordObj {"Record"};
Function* Record = &RecordObj;

Function RedoObj {"Redo"};
Function* Redo = &RedoObj;

Function RehearseObj {"Rehearse"};
Function* Rehearse = &RehearseObj;

Function ReloadScriptsObj {"ReloadScripts"};
Function* ReloadScripts = &ReloadScriptsObj;

Function ReplaceObj {"Replace"};
Function* Replace = &ReplaceObj;

Function ResetObj {"Reset"};
Function* Reset = &ResetObj;

Function RestartObj {"Restart"};
Function* Restart = &RestartObj;

Function RestartOnceObj {"RestartOnce"};
Function* RestartOnce = &RestartOnceObj;

Function ResumeScriptObj {"ResumeScript"};
Function* ResumeScript = &ResumeScriptObj;

Function ReverseObj {"Reverse"};
Function* Reverse = &ReverseObj;

// this was formerly SampleN, Sample1, etc.
Function PlaySampleObj {"PlaySample"};
Function* PlaySample = &PlaySampleObj;

Function SaveCaptureObj {"SaveCapture"};
Function* SaveCapture = &SaveCaptureObj;

Function SaveLoopObj {"SaveLoop"};
Function* SaveLoop = &SaveLoopObj;

Function ShuffleObj {"Shuffle"};
Function* Shuffle = &ShuffleObj;

Function ShortUndoObj {"ShortUndo"};
Function* ShortUndo = &ShortUndoObj;

Function SlipObj {"Slip"};
Function* Slip = &SlipObj;

Function SlipForwardObj {"SlipForward"};
Function* SlipForward = &SlipForwardObj;

Function SlipBackwardObj {"SlipBackward"};
Function* SlipBackward = &SlipBackwardObj;

Function SoloObj {"Solo"};
Function* Solo = &SoloObj;

Function SpeedDownObj {"SpeedDown"};
Function* SpeedDown = &SpeedDownObj;

Function SpeedNextObj {"SpeedNext"};
Function* SpeedNext = &SpeedNextObj;

Function SpeedCancelObj {"SpeedCancel"};
Function* SpeedCancel = &SpeedCancelObj;

Function SpeedPrevObj {"SpeedPrev"};
Function* SpeedPrev = &SpeedPrevObj;

Function SpeedShiftObj {"SpeedShift"};
Function* SpeedShift = &SpeedShiftObj;

Function SpeedOctaveObj {"SpeedOctave"};
Function* SpeedOctave = &SpeedOctaveObj;

Function SpeedStepObj {"SpeedStep"};
Function* SpeedStep = &SpeedStepObj;

Function SpeedBendObj {"SpeedBend"};
Function* SpeedBend = &SpeedBendObj;

Function SpeedUpObj {"SpeedUp"};
Function* SpeedUp = &SpeedUpObj;

Function SpeedToggleObj {"SpeedToggle"};
Function* SpeedToggle = &SpeedToggleObj;

Function StartCaptureObj {"StartCapture"};
Function* StartCapture = &StartCaptureObj;

Function StartPointObj {"StartPoint"};
Function* StartPoint = &StartPointObj;

Function StopCaptureObj {"StopCapture"};
Function* StopCapture = &StopCaptureObj;

Function StutterObj {"Stutter"};
Function* Stutter = &StutterObj;

Function SubstituteObj {"Substitute"};
Function* Substitute = &SubstituteObj;

//Function SurfaceObj {"Surface"};
//Function* Surface = &SurfaceObj;

// don't really like needing SUS variants for these
// try to just have the base Function with canSustain set
// and make it nice in the binding UI
Function SUSInsertObj {"SUSInsert"};
Function* SUSInsert = &SUSInsertObj;

Function SUSMultiplyObj {"SUSMultiply"};
Function* SUSMultiply = &SUSMultiplyObj;

Function SUSMuteObj {"SUSMute"};
Function* SUSMute = &SUSMuteObj;

Function SUSMuteRestartObj {"SUSMuteRestart"};
Function* SUSMuteRestart = &SUSMuteRestartObj;

Function SUSNextLoopObj {"SUSNextLoop"};
Function* SUSNextLoop = &SUSNextLoopObj;

Function SUSOverdubObj {"SUSOverdub"};
Function* SUSOverdub = &SUSOverdubObj;

Function SUSPrevLoopObj {"SUSPrevLoop"};
Function* SUSPrevLoop = &SUSPrevLoopObj;

Function SUSRecordObj {"SUSRecord"};
Function* SUSRecord = &SUSRecordObj;

Function SUSRehearseObj {"SUSRehearse"};
Function* SUSRehearse = &SUSRehearseObj;

Function SUSReplaceObj {"SUSReplace"};
Function* SUSReplace = &SUSReplaceObj;

Function SUSReverseObj {"SUSReverse"};
Function* SUSReverse = &SUSReverseObj;

Function SUSSpeedToggleObj {"SUSSpeedToggle"};
Function* SUSSpeedToggle = &SUSSpeedToggleObj;

Function SUSStutterObj {"SUSStutter"};
Function* SUSStutter = &SUSStutterObj;

Function SUSSubstituteObj {"SUSSubstitute"};
Function* SUSSubstitute = &SUSSubstituteObj;

Function SUSUnroundedInsertObj {"SUSUnroundedInsert"};
Function* SUSUnroundedInsert = &SUSUnroundedInsertObj;

Function SUSUnroundedMultiplyObj {"SUSUnroundedMultiply"};
Function* SUSUnroundedMultiply = &SUSUnroundedMultiplyObj;

Function SyncMasterObj {"SyncMaster"};
Function* SyncMaster = &SyncMasterObj;

Function SyncMasterTrackObj {"SyncMasterTrack"};
Function* SyncMasterTrack = &SyncMasterTrackObj;

Function SyncMasterMidiObj {"SyncMasterMidi"};
Function* SyncMasterMidi = &SyncMasterMidiObj;

Function SyncStartPointObj {"SyncStartPoint"};
Function* SyncStartPoint = &SyncStartPointObj;

Function TimeStretchObj {"TimeStretch"};
Function* TimeStretch = &TimeStretchObj;

// Formerly TrackN, Track1, etc.
Function SelectTrackObj {"SelectTrack"};
Function* SelectTrack = &SelectTrackObj;

Function TrackCopyObj {"TrackCopy"};
Function* TrackCopy = &TrackCopyObj;

Function TrackCopyTimingObj {"TrackCopyTiming"};
Function* TrackCopyTiming = &TrackCopyTimingObj;

Function TrackGroupObj {"TrackGroup"};
Function* TrackGroup = &TrackGroupObj;

Function TrackResetObj {"TrackReset"};
Function* TrackReset = &TrackResetObj;

Function TrimEndObj {"TrimEnd"};
Function* TrimEnd = &TrimEndObj;

Function TrimStartObj {"TrimStart"};
Function* TrimStart = &TrimStartObj;

Function UIRedrawObj {"UIRedraw"};
Function* UIRedraw = &UIRedrawObj;

Function UndoObj {"Undo"};
Function* Undo = &UndoObj;

Function UndoOnlyObj {"UndoOnly"};
Function* UndoOnly = &UndoOnlyObj;

Function WindowBackwardObj {"WindowBackward"};
Function* WindowBackward = &WindowBackwardObj;

Function WindowForwardObj {"WindowForward"};
Function* WindowForward = &WindowForwardObj;

Function WindowStartBackwardObj {"WindowStartBackward"};
Function* WindowStartBackward = &WindowStartBackwardObj;

Function WindowStartForwardObj {"WindowStartForward"};
Function* WindowStartForward = &WindowStartForwardObj;

Function WindowEndBackwardObj {"WindowEndBackward"};
Function* WindowEndBackward = &WindowEndBackwardObj;

Function WindowEndForwardObj {"WindowEndForward"};
Function* WindowEndForward = &WindowEndForwardObj;

Function WindowMoveObj {"WindowMove"};
Function* WindowMove = &WindowMoveObj;

Function WindowResizeObj {"WindowResize"};
Function* WindowResize = &WindowResizeObj;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
