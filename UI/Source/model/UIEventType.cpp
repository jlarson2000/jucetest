/*
 * System constants that define the types of events
 * that can be scheduled internally on the loop timeline
 * and returned in the MobiusState.
 *
 * Since these are spread all over the old code I'm not exactly
 * sure what the internal/display names are so guess for now
 * and only require the display name.  Not even sure if we really
 * need the difference since these are not looked up by name.
 */

#include <vector>

#include "SystemConstant.h"
#include "UIEventType.h"

std::vector<UIEventType*> UIEventType::Instances;

UIEventType::UIEventType(const char* name, const char* symbol) :
    SystemConstant(name, nullptr)
{
    ordinal = Instances.size();
    Instances.push_back(this);
    
    // if the symbol is not set, this is hidden
    timelineSymbol = symbol;
    isStart = false;
    isEnd = false;
    isWeird = false;
}

UIEventType::UIEventType(const char* name, const char* symbol, bool s, bool e, bool w) :
    UIEventType(name, symbol)
{
    isStart = s;
    isEnd = e;
    isWeird = w;
}

// usualy ugly SystemConstant pointer dance for auto-free
// to avoid link conflicts with the same objects in old code
// use the Type suffix, but should really decide if we need these at all
// don't know if the internal names match since they're hard to pin down

// shouldn't try to display these on the timeline
UIEventType InvokeEventObj {"Invoke", "?"};
UIEventType* InvokeEventType = &InvokeEventObj;

UIEventType ValidateEventObj {"Validate", "V"};
UIEventType* ValidateEventType = &ValidateEventObj;


UIEventType RecordEventObj {"Record", "R", true, false, false};
UIEventType* RecordEventType = &RecordEventObj;

// same symbol as Record, but isEnd will color it red
UIEventType RecordStopEventObj {"RecordStop", "R", false, true, false};
UIEventType* RecordStopEventType = &RecordStopEventObj;

UIEventType PlayEventObj {"Play", "P"};
UIEventType* PlayEventType = &PlayEventObj;

UIEventType OverdubEventObj {"Overdub", "O"};
UIEventType* OverdubEventType = &OverdubEventObj;

UIEventType MultiplyEventObj {"Multiply", "M", true, false, false};
UIEventType* MultiplyEventType = &MultiplyEventObj;

UIEventType MultiplyEndEventObj {"MultiplyEnd", "M", false, true, false};
UIEventType* MultiplyEndEventType = &MultiplyEndEventObj;

UIEventType InstantMultiplyEventObj {"InstantMultiply", "IM"};
UIEventType* InstantMultiplyEventType = &InstantMultiplyEventObj;

UIEventType InstantDivideEventObj {"InstantDivide", "ID"};
UIEventType* InstantDivideEventType = &InstantDivideEventObj;

UIEventType InsertEventObj {"Insert", "I", true, false, false};
UIEventType* InsertEventType = &InsertEventObj;

UIEventType InsertEndEventObj {"InsertEnd", "I", false, true, false};
UIEventType* InsertEndEventType = &InsertEndEventObj;

UIEventType StutterEventObj {"Stutter", "St"};
UIEventType* StutterEventType = &StutterEventObj;

UIEventType ReplaceEventObj {"Replace", "Rp"};
UIEventType* ReplaceEventType = &ReplaceEventObj;

UIEventType SubstituteEventObj {"Substitute", "S"};
UIEventType* SubstituteEventType = &SubstituteEventObj;

// I think the next three are internal only
UIEventType LoopEventObj {"Loop", "?"};
UIEventType* LoopEventType = &LoopEventObj;

UIEventType CycleEventObj {"Cycle", "?"};
UIEventType* CycleEventType = &CycleEventObj;

UIEventType SubCycleEventObj {"Subcycle", "?"};
UIEventType* SubCycleEventType = &SubCycleEventObj;

UIEventType ReverseEventObj {"Reverse", "Rv"};
UIEventType* ReverseEventType = &ReverseEventObj;

// I think internal due to latency compensation
UIEventType ReversePlayEventObj {"ReversePlay", "?"};
UIEventType* ReversePlayEventType = &ReversePlayEventObj;

UIEventType SpeedEventObj {"Speed", "Sp"};
UIEventType* SpeedEventType = &SpeedEventObj;

UIEventType RateEventObj {"Rate", "Ra"};
UIEventType* RateEventType = &RateEventObj;

UIEventType PitchEventObj {"Pitch", "Pi"};
UIEventType* PitchEventType = &PitchEventObj;

UIEventType BounceEventObj {"Bounce", "B"};
UIEventType* BounceEventType = &BounceEventObj;

UIEventType MuteEventObj {"Mute", "Mu"};
UIEventType* MuteEventType = &MuteEventObj;

// should be filtered
UIEventType JumpPlayEventObj {"Jump", "J"};
UIEventType* JumpPlayEventType = &JumpPlayEventObj;

UIEventType UndoEventObj {"Undo", "U"};
UIEventType* UndoEventType = &UndoEventObj;

UIEventType RedoEventObj {"Redo", "Re"};
UIEventType* RedoEventType = &RedoEventObj;

// how does this differ from RunScriptEventObj?
UIEventType ScriptEventObj {"Script", "Sc", false, false, true};
UIEventType* ScriptEventType = &ScriptEventObj;

UIEventType StartPointEventObj {"StartPoint", "SP"};
UIEventType* StartPointEventType = &StartPointEventObj;

UIEventType RealignEventObj {"Realign", "Rl"};
UIEventType* RealignEventType = &RealignEventObj;

// probably only in scripts, but might be nice to see
UIEventType MidiStartEventObj {"MIDIStart", "Ms"};
UIEventType* MidiStartEventType = &MidiStartEventObj;

// these are common and really need an icon
UIEventType SwitchEventObj {"Switch", "LS"};
UIEventType* SwitchEventType = &SwitchEventObj;

UIEventType ReturnEventObj {"Return", "Rt"};
UIEventType* ReturnEventType = &ReturnEventObj;

// weird, I guess paired with ReturnEvent?
UIEventType SUSReturnEventObj {"SUSReturn", "Rt", false, false, true};
UIEventType* SUSReturnEventType = &SUSReturnEventObj;

// pretty sure these are instant
UIEventType TrackEventObj {"Track", "Tk"};
UIEventType* TrackEventType = &TrackEventObj;

// would be nice to capture the Script name in the event summary
// for the extended display
// wait, how does this differ from just ScriptEvent?
UIEventType RunScriptEventObj {"Script", "Sc", false, false, true};
UIEventType* RunScriptEventType = &RunScriptEventObj;

UIEventType SampleTriggerEventObj {"Sample", "Sm"};
UIEventType* SampleTriggerEventType = &SampleTriggerEventObj;

// not sure if these can happen
UIEventType SyncEventObj {"Sync", "Sy"};
UIEventType* SyncEventType = &SyncEventObj;

UIEventType SlipEventObj {"Slip", "Sl"};
UIEventType* SlipEventType = &SlipEventObj;

UIEventType MoveEventObj {"Move", "Mv"};
UIEventType* MoveEventType = &MoveEventObj;

UIEventType ShuffleEventObj {"Shuffle", "Sh"};
UIEventType* ShuffleEventType = &ShuffleEventObj;

// I think just somethign I use for debugging
UIEventType SyncCheckEventObj {"SyncCheck", "?"};
UIEventType* SyncCheckEventType = &SyncCheckEventObj;

UIEventType MidiOutEventObj {"MIDIOut", "Mo"};
UIEventType* MidiOutEventType = &MidiOutEventObj;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
